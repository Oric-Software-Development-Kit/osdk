# OSDK 1.24 — complete fix list (feature/compiler-improvements)

Working document for the official patch notes (`doc_historic.htm`, 1.24 section).
Every entry: **Found** (how it surfaced), **Problem**, **Fix**.
Items marked `[documented]` are already in the current 1.24 section of
doc_historic.htm; unmarked items still need entries.

---

## 1. Compiler (compiler.exe, → V1.41)

### -O3: call result stored through a call-clobbered pointer / at the wrong width
- **Found:** the long-standing "aes256 fails at -O3" report. A multi-day hunt first (wrongly) suspected the macros then the peephole optimizer; both were exonerated. Runtime tracing pinned it to `buf[i] = rj_sbox(buf[i])` in aes_subBytes, where the C runtime zero page (sp/fp) got a wrong high byte, cascading into wild stores.
- **Problem:** the -O3 ASGN-into-call fold (make a call store its result straight into the assignment's lvalue) fired in two unsound cases. For `dst[i] = f(...)` it stored the result through the destination-address temporary — but the call clobbers the scratch temporaries, so the address was stale by the time the folded store ran (the compiler even emitted SAVE/RESTORE around the call, but the folded store executed before the RESTORE). And it inherited the CALL's width, so an int-returning call folded into a char assignment stored a word, clobbering the neighbouring byte (`somechar = somefunc()` to a global was also affected — it overwrote the next global).
- **Fix (gen.c):** the ASGN-into-call fold is now suppressed when the right child is a CALL and either the call's result width differs from the assignment's, or the destination address lives in a temporary (clobbered across the call). Both cases fall back to the correct sequence: call result to a temp, RESTORE, then a properly-widthed ASGN. Matching-width, stable-target folds (e.g. `int_global = int_func()`) are unchanged, so no perf loss there.
- **Result:** aes256 -O3 runs correctly for the first time (crash at 4.1M cycles -> full run at ~72M, and -O3+peephole is now smaller AND faster than -O3 alone, as intended). New t_aes known-answer test in the suite (encrypt/decrypt round-trip). This is the third bug in the borrowed-temporary family (see the two -O3 entries below).

### Benchmark result (ISS oricCompilerBenchmark, 2026-07-17, vs OSDK 1.23 -O2)
Full 22-sample table regenerated after the -O3 fix (results/benchtable-{size,cycles}-20260717.csv).
- **Code size:** new-O3 averages -19% vs 1.23-O2 (up to -30%); the peephole shaves a little more. aes256 -O3+peephole is the smallest build (13913 B) - the config that used to CRASH.
- **Speed:** new-O3 totals -15% cycles vs 1.23-O2 across the compute samples. Standouts: frogmove -66.7%, selection-sort -35%, bubble-sort -30%, aes256 -12.5% (-14.3% with peephole). The peephole adds a further ~1-2% on the heavy samples (aes -2.0%).
Cycles are exact: read from Oricutron's internal cycle counter (correct and stable for many years) via a GDB step-over of the timed call; IRQ masked (sei) for determinism. (The --cport @CYCLES convenience is a new, benchmark-specific addition not yet in the official OSDK, so it isn't relied on here.)

### Float expressions always failed — "expression too complex" `[documented]`
- **Found:** any floating point expression failed to compile (regression shipped in OSDK 1.23).
- **Problem:** the 1.40 temporary-register change marked all 32 float temporaries permanently busy.
- **Fix:** restored the per-function temporary name reset; float temporaries are frame-allocated on demand. (`b301e483`)

### 0b binary literals `[documented]`
- **Found:** oricCompilerBenchmark "0xcafe" sample failed to compile.
- **Fix:** lex.c accepts 0b/0B integer literals. (`b301e483`)

### Constant folding used 32-bit host arithmetic `[documented]`
- **Found:** cc65 bug2461 — `0xFF50 + 0x100` folded to `0x10050` at compile time but wraps to `0x0050` at run time, so ==/<= against runtime values failed; also corrupted compare-rev13's failure counter (hundreds of folded unsigned compares branched wrong).
- **Problem:** simp.c folded with host (32-bit) limits; lex.c classified literals with host limits; gen.c could receive constant-constant residues it had no macros for (crash class: missing FAMILY_CC* variants).
- **Fix:** all folds wrap to the 16-bit target width; literals wrap/classify with target limits; emitters normalize leftover constant operands (swap commutative, materialize through scratch). (`7dbf6572`)

### Signed >> was a logical shift `[documented]`
- **Found:** t_arith execution test: `-16 >> 2` gave 16380 instead of -4.
- **Problem:** RSHI and RSHU both emitted the logical RSHW family. Implementation-defined per C89, but every mainstream compiler sign-extends and Oric code relies on it.
- **Fix:** new ASRW macro family (cmp #$80 / ror core); RSHI emits it, RSHU unchanged. (`b0cb1c7a`)

### Constant-first == / != stopped the build
- **Found:** cc65 bug1408 and compare-rev tests: `0 == -x` (x unsigned) survives the folder, emitter produced macro names (NEW_CD…) the library deliberately does not provide.
- **Fix:** == and != are symmetric, so the emitter swaps operands to the constant-second variants; `0 == x` now also reaches the optimized EQ0W/NE0W forms. (`44b69536`)

### -O3: dereference lost when passed as differently-typed argument
- **Found:** execution suite t_ptr: `f(*(arr + i))` with f taking unsigned passed the ADDRESS instead of the value.
- **Problem:** (1) conversion no-op elision compared operand/result by NAME only — after the -O3 INDIR fold, "(tmp0),0" vs "tmp0" look identical and eliding dropped the dereference; (2) the temporary borrowed by a folded INDIR was released too early, so another node could clobber the address before use.
- **Fix:** elision also requires equal addressing modes; borrowed slot stays reserved until the parent allocated its result. (`4f860d3a`)

### -O3: borrowed temporary freed while still shared
- **Found:** t_struct at -O3: in `p->a == a && p->b == b && p->c == c` the pointer in tmp0 was overwritten by the next subexpression's temporaries.
- **Problem:** a folded INDIR only owns its child's temporary when the fold consumed the LAST reference; when still shared, releasing it freed a busy slot that was not ours.
- **Fix:** ownership tracked via the reference count; with the link65 fix below, t_struct passes 12/12 at -O3 for the first time (and -O3 beats -O2 on both size and speed there). (`b20215ef`)

---

## 2. Macro library (MACROS.H)

### Six -O3 macro bodies emitted invalid assembly `[documented]`
- **Found:** any -O3 build with `a = a - b` on stack-resident variables broke (same class as the 2019 forum report).
- **Problem:** INDIRW_DY, SUBW_YYY, COMW_YY, ANDW_YYY, XORW_YYY, ORW_YYY stored with a literal index or a direct label instead of ",y".
- **Fix:** corrected bodies; 8 missing ASGNS struct-copy variants (CD/DD/AD/YD/CY/DY/YY/YZ) added — -O3 `*ps = *pt` previously died on an undefined macro. (`1a671d72`)

### `#HIGH(cte1)+1` — wrong page for $FF-low constants
- **Found:** static audit of all ~700 macros (hunting the aes256 -O3 corruption class).
- **Problem:** `#define HIGH >` and XA's `<`/`>` byte operators bind the WHOLE trailing expression, so `#HIGH(cte1)+1` assembles as HIGH(cte1+1): the high byte is one too large whenever the constant's low byte is $FF — a wrong-page pointer. Affected MOVW_CY and 6 shift macros (LSHW/RSHW/ASRW _CYD/_CYY).
- **Fix:** removed the `+1` on all 7 lines, matching the correct siblings. (`0c941568`)

### ASGNW_YD always miscompiled the low byte
- **Found:** same audit.
- **Problem:** `lda ptr1,y` loads the source low byte, then `lda #0` immediately discards it AND leaves Y at the wrong offset for the store.
- **Fix:** `lda #0` → `ldy #0`. (`0c941568`)

### FRASM-era `*+N` self-modifying code → symbolic labels (robustness/readability)
- **Found:** review triggered by the question "can the peephole optimizer break macro-internal branches?".
- **Problem:** 52 `sta/stx *+N` operand patches (22 ASGNS block-copy macros, CALLF_DD/DY) gave no clue what instruction/byte they patch, and their safety against the optimizer depended on the frozen-region layout rather than a structural guarantee. (Zero `*+N` branches existed; named-label branches and .( .) scopes were already safe — labels are optimizer barriers.)
- **Fix:** patched load labeled `src`, patched store `dst`, inline jsr `jsrto`; `*+N` → `src+1/src+2`, `dst+1/dst+2`, `jsrto+1/jsrto+2`. Proven byte-identical (probe over all 22 ASGNS variants + both CALLF, old vs new → same binary). (`0c941568`)

### ASRW family + naming glossary `[documented]`
- New ASRW arithmetic-shift family; glossary in MACROS.H/gen.c documenting operation families and addressing-mode suffix letters. (`b0cb1c7a`)

---

## 3. MacroSplitter (0.3) — expansion + peephole optimizer

### Built-in macro expansion is now the default
- OSDKMACROEXPAND=1 default: compiler output expanded by macrosplitter -M instead of a second cpp.exe pass; =0 restores the old path. Byte-identical taps verified. (`2ee28af7`)

### New 6502 peephole optimizer (-O, used at OSDKMACRO=-O)
- Removes redundant load/store patterns after macro expansion (self-store, load-after-store, dead store, tail call jsr+rts→jmp, dead rts after jmp, cross-register transfers, same-immediate reload).

### Peephole hardened against 4 unsafety classes
- **Found:** systematic safety review before enabling it more widely.
- **Problems/Fixes:** (1) regions containing `*+N`/`*-N` operands are frozen — no elimination or size change may move a self-mod target; (2) loads from the I/O page $300-$3FF are never eliminated (VIA reads have side effects); (3) load-after-store elimination requires N/Z to already reflect the register, preserving flag semantics for a following branch; (4) comment lines tokenize whole and act as barriers. Byte-identical output on real code (guards cost nothing). (`62e3ccc8`)

### Peephole normalized #>($fffff0ff) to the wrong value
- **Found:** every signed-bitfield test in the cc65 sweep failed with the optimizer on.
- **Problem:** the immediate normalizer parsed with strtol, which clamps above LONG_MAX; the compiler emits 16-bit constants sign-extended to 32 hex digits, so the clear-mask #>($fffff0ff) (= $f0) became #255 and field stores stopped clearing bits.
- **Fix:** parse with strtoul, truncate to 16 bits the way XA evaluates; t_bitfield guard test added. (`7b85903b`)

### Optimizer exonerated for the aes256 -O3 crash (investigation result, no code change)
- All 54 eliminations on aes256 proven individually safe (CZBW/CSBW src==dst self-stores); reassembly clean. The crash is a LATENT layout-sensitive bug elsewhere that any 2-byte shift trips — still open, tracked separately. Worth noting in dev docs, not user patch notes.

---

## 4. Link65 (→ 1.5)

### Statement scanner corrupted multi-statement lines
- **Found:** -O3 struct code failed at assembly with "Label 'mul16i' not defined".
- **Problem:** parseline's strtok pokes NULs into the shared line buffer and its opcode scan can run past the current ':'-separated statement; a NUL landing in a later statement hid the remaining ':' separators, so trailing statements (the `jsr mul16i` in the expanded MULI macro) were never scanned and the library dependency was missed.
- **Fix:** each statement parsed in an isolated buffer copy; preprocessor lines kept whole so #include paths with drive-letter ':' keep working. (`7e890bdc`)

---

## 5. C library (lib/*.s, headers)

### Dynamic allocator repaired: malloc / free / realloc (+ validation test)
- **Found:** full audit + new t_malloc.c validation test (the API was rarely used — most OSDK code allocates statically — and turned out broken in several independent ways).
- **malloc.s:** the "no next block" case loaded the length limit from _heapend but fell through into the temp=next code, overwriting it with 0 → len = 0-start (huge) → **the heap-end bound was never enforced** (this path runs on the very first allocation). Added the missing skip branch.
- **free.s:** (1) the _freemc machine-code entry null-checked the C stack (sp) instead of the pointer passed in A/X, so a direct call (e.g. from realloc) could silently skip a valid free; now tests A/X. (2) the 16-bit descriptor decrement used `dec lo : bpl hi`, which corrupts the high byte whenever the low result has bit 7 set (≥129 live blocks); replaced with test-before-decrement.
- **realloc.s:** rewritten — broken in every path: signed size comparison (bmi) mis-routed shrinks to the grow path; the shrink path read/wrote descriptor offset 0/1 (the NEXT pointer) instead of the length at 2/3, corrupting the free list; the grow path's stack-based memcpy hack never actually resized or re-accounted the block. New code: shrink in place, grow via malloc + inline copy + free, consistent size-including-overhead convention.
- **Validation:** t_malloc.c — heap bounds, no-overlap, accounting (nheapbytes/nheapdesc), block reuse, realloc grow/shrink/NULL/zero with data preservation, heap-end OOM. 39/39 at -O1/-O2/-O3. (`03ebd774`)

### strchr(s, '\0') returned NULL
- **Problem:** end-of-string was tested BEFORE comparing, but C89 says the terminator is part of the string.
- **Fix:** compare first; SMC operand offset adjusted and verified at binary level. (`8fc57a6e`)

### strtok was not C89
- **Problem:** no leading-delimiter skip (consecutive delimiters produced empty tokens); wrong sequencing at the last token.
- **Fix:** C89 rewrite — skip leading delimiters, terminate in place, continuation cleared at exhaustion so further calls return NULL. (`c29fcea1`)

### sprintf missing NUL terminator; new atoi and fprintf `[documented]`
- sprintf never wrote the terminating NUL (buffer reuse showed stale tails); new C89 atoi; new fprintf + minimal FILE/stdin/stdout/stderr. (`d8f87385`)

### Runtime int→float conversion used the wrong ROM entry `[documented]`
- "cif" pointed at $DF24 — the tail of ROM SGN, which converts only the signed 8-bit value in A — so every runtime (float) cast of an int variable was wrong ((float)10 == 0.0). Unnoticed because positive literal casts are constant-folded. Now a proper routine using the unsigned 16-bit ROM entry $DF40 with sign handling. (`8c74528d`)

### Headers: EXIT_SUCCESS/EXIT_FAILURE, const-correct string.h `[documented]`
- Missing EXIT_* was the single most common build failure across the imported cc65 suite (55 of 313 tests). (`d85d0f9b`)

### String functions: strstr, strcspn, strspn, strrchr, memccpy (+ t_string test)
- **Found:** lib audit; validated with the new t_string.c (page-crossing buffers, C89 edge cases). Baseline evidence: memccpy made ANY program using it fail to link; the others returned wrong results in specific conditions.
- **strstr.s:** on a mismatch after a partial match it reset the needle but never rewound the haystack, so overlapped matches were missed (strstr("aaab","aab") returned NULL instead of s+1). Now records the match start (offset+page) and resumes from match start + 1. Also strstr(s,"") returned garbage (uninitialized result registers); now returns s per C89.
- **strcspn.s / strspn.s:** the s1 page-crossing path jumped back into the INNER loop instead of the outer one, silently skipping the first character of every 256-byte page — wrong results for any string longer than 256 bytes (a skipped stop-character made the span run long). Now continues with the outer loop.
- **strrchr.s:** (1) the returned pointer was computed with the scan-advanced high byte of the string pointer, so any string longer than 256 bytes returned an address too high by the number of pages crossed; the original high byte is now saved and used. (2) C89: strrchr(s,'\0') now returns a pointer to the terminator instead of NULL (same class as the strchr fix).
- **memccpy.s:** rewritten standalone. The old code patched a byte inside strncpy (which, after the strncpy rewrite, landed on an OPCODE, not an operand) and jumped to a label (cpycommon) that no longer exists — any program referencing memccpy failed at link/assembly time. Prototype added to string.h (it was missing entirely).
- **Validation:** t_string.c — 24 checks covering all five functions including >256-byte page-crossing buffers and C89 edge cases, at -O1/-O2/-O3.
- **Size discipline:** strcspn/strspn are byte-identical (branch retarget only); strrchr +4 bytes for the two fixes; strstr +18 (the rewind state); memccpy is 62 bytes standalone with a two-counter loop (the old 9-byte version only "worked" by not linking at all).

### file_unpack_raw: missing clc in the C wrapper
- **Found:** lib audit; confirmed by carry-state trace.
- **Problem:** the C-API wrapper in front of the (correct, carefully flag-commented) LZ77 core adds the unpacked size to the destination pointer with the carry in whatever state the compiled caller left it — the sibling _file_unpack does clc before the identical addition. If C=1 at entry, the end pointer is one too high and one extra byte is emitted past the intended end.
- **Fix:** add the clc, matching the sibling.

### SEDORIC support rewritten: sedoric() fixed, sed_savefile/sed_loadfile added, new zero-page shelter module
- **Found:** lib audit + long-standing field reports (forum "SEDORIC in C": sedoric() caused "syntax error on exit" and garbage LIST output).
- **Problem (old sedoric()):** the command string was copied UNBOUNDED into the $35 line buffer, which overlaps the C runtime zero page from $50 on — any command longer than 26 characters corrupted ap/fp/sp; and TXTPTR ($E9/$EA) was clobbered without restore (the "syntax error on exit").
- **New lib/zeropage.s:** _zp_swap exchanges the C runtime zero-page block (ap..zp_compiler_save_end, bounds taken from the zp_crt labels so OSDK_ZP_START relocation is handled) with an internal buffer. Self-inverse: one 20-byte routine + 44-byte buffer both saves and restores; IRQ-safe (sei during the swap). Only the compiler block is swapped, so ROM/BASIC/SEDORIC zero-page state (CHRGET, TXTPTR, float accus) stays live — which DOS calls require. Reusable for any ROM/DOS interop.
- **New sedoric.s:** fresh implementation. sedoric() now shelters the C block, bounds the command to the 79-char buffer and restores TXTPTR. New sed_savefile(name,buf,len) / sed_loadfile(name,buf,&len) save/load SEDORIC data files via the documented entry points, named after "SEDORIC (3.0) à nu" (XNF $D454, XLOADA $E0E5, the common SAVE tail $DE0B, RAMROM $04F2, ERRGOTO, VSALO/DESALO/FISALO/EXSALO/LGSALO); both return the DOS error number (cleared before the call), 0 on success.
- **Credits:** entry points and names from "SEDORIC (3.0) à nu" (A. Chéramy, C. Sittler); load/save technique validated against ISS's GPL lib-sedoric (reference only, not copied — github.com/iss000/oricOpenLibrary, forum thread t=2232).
- **Size:** whole module ~366 bytes (vs ~490 for the GPL reference), pulled only when referenced.
- **Validation:** runtime-validated end to end in Oricutron booting a generated SEDORIC disk: !DIR executes, sed_savefile's SAVED.DAT physically lands in the disk image, sed_loadfile brings the HIRES picture to $A000 and it displays, and the C program keeps running after every call.
- **New sample/c/sedoric:** demo + the full floppy pipeline in osdk_build.bat (make → PictConv -o1 picture TAP → tap2dsk → old2mfm). Documents OSDKTAPNAME (the internal tape name becomes the SEDORIC file name: SEDDEMO → SEDDEMO.COM) and the disktype=microdisc emulator requirement.
- **Follow-up fix (inclusive end addresses):** SEDORIC end addresses are inclusive (SAVE"F",A#A000,E#BF3F saves 8000 bytes; catalog length = FISALO−DESALO). The first version used C-style exclusive bounds: sed_savefile saved one byte too many and sed_loadfile reported the length one byte short (7999 for the 8000-byte picture — spotted by Mike in the demo status line). FISALO now gets begin+len−1 and *len returns LGSALO+1; boot-verified (status line reports 8000, SAVED.DAT is a true 64 bytes).

### Backlog noted: tap2dsk -m flag to emit MFM directly (backward compatible; old tap2dsk+old2mfm projects unaffected). Long-wished by Mike.

### CRT slimming: -396 bytes for a minimal program
- The 256-byte software stack is no longer emitted in the tap (moved to .bss above the image; osdk_end semantics preserved); enter/leave moved from always-linked header.s to lib/frame.s, linked only when referenced. cif/cfi moved from header.s to lib/float.s, on demand (-33 bytes for every non-float program). (`6ade0460`, `32f86eff`)

---

## 6. Build system (make.bat, preprocessor)

- **mcpp 2.7.2 is the default C preprocessor** (C99-conforming, file:line diagnostics, hard errors on missing includes); legacy cpp.exe stays bundled, OSDKCPP overrides; license file shipped. Byte-identical output verified vs cpp.exe on the suite + ~175 cc65 A/B pairs. (`a5eb0c19`, `497e3a3d`, `2ee28af7`)
- **Project -I now searched before the bundled include** so a project can override a bundled header. (`29433d4e`)
- **INCLUDE/C_INCLUDE_PATH cleared around the preprocessing pass** — mcpp honors them, so a VS developer prompt silently pulled MSVC system headers. (`6ade0460`)

## 7. Test infrastructure (not user-facing; summarize as one patch-note entry) `[documented]`

- Automated execution test suite (9 tests × -O1/-O2/-O3, results via emulated printer, CSV diffs). (`eca90bd1`+)
- cc65/SDCC-heritage regression importer: ~313 upstream tests run UNMODIFIED; missing standard headers provided by a shim documenting OSDK's real type sizes. (`31cefb3f`)
- ISS oricCompilerBenchmark runner with exact cycle counts (--cport counter; selectable OSDK root for A/B against released toolchains). (`5ab5a85a`, `29433d4e`)
- Sandboxed emulator, lockfile serialization, --turbo (4× faster sweeps) / --headless, hidden-desktop fallback. (misc)
- t_malloc allocator validation test (39 checks). (`03ebd774`)

---

## Still open (NOT for 1.24 notes yet)
- aes256 -O3 latent layout-sensitive bug (any 2-byte shift trips it; peephole exonerated).
- Lib audit remainder to verify/fix: memccpy (SMC wrong byte + undefined label), strstr (no rewind after partial match), strcspn/strspn (page-cross skips a char), strrchr (page-cross wrong address), sedoric (unbounded ZP copy).
- LOW/HIGH → native `<`/`>` modernization in MACROS.H.
