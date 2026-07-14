# OSDK compiler execution test suite

Self-checking C programs that are compiled with the full OSDK pipeline,
executed in Oricutron, and report their results through the emulated
printer. Unlike a compile-only check, this catches *wrong code*, not just
build breaks.

## Running

    .\run_tests.ps1                          # all tests at -O1 -O2 -O3
    .\run_tests.ps1 -Levels 3 -Filter t_ptr  # one test, one level
    .\run_tests.ps1 -Label "after-my-fix"    # tag the result CSV

Each run writes `results\<timestamp>_<label>.csv` with, per test and per
optimization level: build/run status, pass and fail counts, elapsed 100Hz
ticks (speed) and TAP size in bytes (code size).

To compare two runs (e.g. before/after a compiler change):

    .\compare_results.ps1 results\old.csv results\new.csv -Out report.md

## How it works

- Tests live in `tests\t_*.c` and use `testkit\testkit.h`:
  `tk_begin("name")`, `tk_check(cond, "check")`, `tk_check_eq(v, want,
  "check")`, `tk_end()`. Results go to the printer via the Atmos ROM
  PrintChar routine ($F5C1); Oricutron captures them in `printer_out.txt`.
- Timing uses the $0272/$0273 100Hz countdown decremented by the ROM IRQ.
- The runner builds each test in `scaffold\` with make.bat, boots the TAP
  in a private emulator copy in `sandbox\` (so it never interferes with a
  developer's Oricutron session), polls the printer file for `@END`, then
  kills the emulator. Timeout means hang/crash.
- `sandbox\`, `scaffold\` and `results\` are generated and git-ignored.

## Output protocol (printer)

    @TEST <name>
    FAIL <check-name> [got=XXXX want=XXXX]     (only failing checks)
    @RESULT pass=<hex> fail=<hex> ticks=<hex>
    @END

## Current tests

| file | covers |
|---|---|
| t_arith.c | int/unsigned arithmetic, div/mod signs, shifts, promotions, comparisons |
| t_binlit.c | 0b binary literals (added in Compiler 1.41) |
| t_float.c | float arithmetic/compares/conversions (regression-fixed in 1.41) |
| t_frame.c | frame-resident -O3 shapes (SUBW_YYY etc., fixed in MACROS.H 2026-07) |
| t_ptr.c | pointers, multi-dim arrays, the 2019 forum -O3 copy loop |
| t_struct.c | struct copies in all ASGNS addressing shapes, struct params/returns |

## Known pre-existing failures (documented, not regressions)

- `t_arith/sar`: signed `>>` emits a logical shift (RSHI and RSHU both map
  to RSHW in gen.c) - implementation-defined per C89 but surprising.
- `t_float/itof-neg`: (float) of a negative int converts incorrectly.
- `t_ptr/ptr-arith` at -O3: `*(arr + const + var)` reads a wrong address.
- `t_struct` at -O3: build fails because link65's dependency scan misses
  the `jsr mul16i` generated for variable struct indexing.

## Ideas for growth

- Import cc65's `test/val` suite unmodified: put the tests on a SEDORIC
  disk with an INIST of `PRSET:...` (tap2dsk `-i` option) so their printf
  output is redirected to the printer (OSDK printf goes through the $0238
  vector, which PR SET hooks). Boot with `-k microdisc -d disk.dsk`.
- lcc's own `tst/` suite (same front end lineage), Dhrystone for a
  standard speed figure.
