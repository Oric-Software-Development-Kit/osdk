
; ==========================================================================
;
;  XA Label Reference
;  ------------------
;  This sample demonstrates every type of label supported by XA.
;  It assembles and runs on an Oric, but its main purpose is to serve as a reference you can read alongside the documentation.
;
;  Label types covered:
;    1. Global labels             (visible everywhere)
;    2. Scoped Global labels      (+prefix, exported in relocatable mode)
;    3. Block-scoped labels       (.( / .) blocks)
;    4. Block escape (&prefix)    (define a label one level up)
;    5. Redefinable labels        (-prefix, like a variable)
;    6. Cheap local labels        (@prefix, scoped to routine)
;    7. Unnamed labels             (: define, :+/:- reference)
;
; ==========================================================================


; Screen geometry
#define SCREEN_BASE  $BB80
#define LINE_WIDTH   40


; --------------------------------------------------------------------------
;
;  1. Standar/global LABELS
;
;  A plain label is visible from anywhere in the file, both before and after its definition (XA is a two-pass assembler).  
;  Use them for routines and data that other code needs to reference.
;
;  Labels outside a .( / .) block are already global.  
;  The '+' prefix is only useful INSIDE a block: it forces the label to be visible at the global level despite being in a scope.
;  This lets you hide a routine's internals in a block while still exposing some parts of it to the outside world.
;
;  The _ in front of main is only useful when linking with the C runtintime
;
; --------------------------------------------------------------------------

_main  
    ; The 4 labels all point to the same routine at the exact same address
    jsr global_label
    jsr _global_label_for_c
    jsr global_label_in_scope
    jsr global_label_in_scope_for_c

    jsr demo_blocks
    jsr demo_block_escape
    jsr demo_redefinable
    jsr demo_cheap_local
    jsr demo_unnamed
    rts


global_label                    ; Label that is global from XA point of view, but not usable from a C module
_global_label_for_c             ; Adding a _ in front of a label makes the label usable from a C module (where it can be used without the "_")
    .(                          ; Opens a scope
+global_label_in_scope          ; visible globally despite the block
+global_label_in_scope_for_c    ; visible globally despite the block
loop                            ; this one stays block-local
    nop
    bne loop                    ; Branch to the local label (that one can exist many times in the code since it's local)
    rts
    .)                          ; Closes a scope



; --------------------------------------------------------------------------
;
;  3. BLOCK-SCOPED LABELS  (.( / .) )
;
;  A .( / .) pair creates a scope block.  
;  Labels defined inside the block are local: they are invisible from outside.  
;  This lets you reuse common names like "loop" without worrying about collisions.
;
;  The typical pattern is: entry point label outside the block, internal labels (loop, skip, done...) hidden inside.
;
; --------------------------------------------------------------------------

demo_blocks
    ; Both blocks below define "loop" — no collision because each .( .) is its own scope.
    .(
    lda #65                 ; 'A'
    ldx #0
loop
    sta SCREEN_BASE+LINE_WIDTH*2,x
    inx
    cpx #LINE_WIDTH
    bne loop
    .)

    .(
    lda #66                 ; 'B'
    ldx #0
loop                        ; same name, different scope — no problem
    sta SCREEN_BASE+LINE_WIDTH*3,x
    inx
    cpx #LINE_WIDTH
    bne loop
    .)
    rts



; --------------------------------------------------------------------------
;
;  4. BLOCK ESCAPE  (&prefix)
;
;  When you have nested blocks, a label in the inner block is invisible
;  to the outer block.  Prefixing it with '&' makes it visible one
;  level up, so the outer block can reference it.
;
;  Use '&&' to escape two levels, '&&&' for three, etc.
;
; --------------------------------------------------------------------------

demo_block_escape
    .(                          ; outer block
    .(                          ; inner block
&print_char                     ; & escapes one level: visible in outer block
    lda #66                     ; 'B'
    sta SCREEN_BASE+LINE_WIDTH*4
    rts
    .)
    ; Without &, calling print_char here would fail — it would be trapped inside the inner block.  
    ; The & makes it reachable.
    jsr print_char
    .)
    rts


; --------------------------------------------------------------------------
;
;  5. REDEFINABLE LABELS  (-prefix)
;
;  A '-' prefix allows a label to be redefined later using an assignment operator.  
;  Normal labels can only be defined once; redefinable labels act like variables whose value can change.
;
;  This is handy for building tables where each row is computed from the previous one, or for compile-time counters.
;
; --------------------------------------------------------------------------

demo_redefinable
    ; Start with cursor at line 6
-cursor = SCREEN_BASE + LINE_WIDTH*6

    lda #67                 ; 'C'
    sta cursor              ; writes to line 6

    ; Advance cursor to line 7
-cursor += LINE_WIDTH

    lda #68                 ; 'D'
    sta cursor              ; writes to line 7

    ; Advance cursor to line 8
-cursor += LINE_WIDTH

    lda #69                 ; 'E'
    sta cursor              ; writes to line 8

    rts



; --------------------------------------------------------------------------
;
;  6. CHEAP LOCAL LABELS  (@prefix)
;
;  Cheap local labels are a lightweight alternative to .( / .) blocks.
;  They are prefixed with '@' and are scoped to the nearest preceding standard label definition.  
;  Each time a new standard label is defined, the cheap local scope resets, so the same @name can be reused in the next routine without collision.
;
;  Advantages over blocks:
;   - No .( / .) boilerplate
;   - One label per routine is enough to create a scope
;   - Ideal for short loop/skip patterns inside routines
;
;  Rules:
;   - @labels are only visible between their enclosing standard label and the next standard label.
;   - You can mix @labels with .( / .) blocks if needed.
;
; --------------------------------------------------------------------------

demo_cheap_local
    jsr fill_line_10
    jsr fill_line_11
    rts


fill_line_10
    ; @loop here belongs to fill_line_10's scope
    lda #70                 ; 'F'
    ldx #0
@loop
    sta SCREEN_BASE+LINE_WIDTH*10,x
    inx
    cpx #LINE_WIDTH
    bne @loop
    rts


fill_line_11
    ; @loop here belongs to fill_line_11's scope -- no collision!
    lda #71                 ; 'G'
    ldx #0
@loop
    sta SCREEN_BASE+LINE_WIDTH*11,x
    inx
    cpx #LINE_WIDTH
    bne @loop
    rts



; --------------------------------------------------------------------------
;
;  7. UNNAMED LABELS  (: define, :+/:- reference)
;
;  Unnamed labels have no name at all.  You define one with a bare ':'
;  at the start of a line, and reference the nearest one with :+ (next
;  forward) or :- (previous backward).  Use :++ to skip one forward,
;  :+++ to skip two, etc.
;
;  They are even more lightweight than @cheap locals — useful for tiny
;  branches where even naming the label feels like overkill.
;
;  Requires the -a assembler flag (set via OSDKXAPARAMS in osdk_config.bat).
;
; --------------------------------------------------------------------------

demo_unnamed
    ; Fill line 13 with 'H', then line 14 with 'I'
    lda #72                 ; 'H'
    ldx #0
:                               ; first unnamed label (the loop)
    sta SCREEN_BASE+LINE_WIDTH*13,x
    inx
    cpx #LINE_WIDTH
    bne :-                      ; branch back to the nearest ':'

    lda #73                 ; 'I'
    ldx #0
:                               ; second unnamed label (another loop)
    sta SCREEN_BASE+LINE_WIDTH*14,x
    inx
    cpx #LINE_WIDTH
    bne :-                      ; branch back — refers to THIS ':', not the first one
    rts



; --------------------------------------------------------------------------
;
;  SUMMARY - When to use what
;
;  Label type      Syntax      Best for
;  ──────────────  ──────────  ─────────────────────────────────────
;  Standard        name        Routines, data, anything shared
;  Global          +name       Entry points for linker (with -R)
;  Block-scoped    .( / .)     Isolating a section of code
;  Block-escape    &name       Exporting from inside a block
;  Redefinable     -name       Compile-time variables / cursors
;  Cheap local     @name       Quick loop/skip labels in routines
;  Unnamed         : :+ :-    Tiny branches not worth naming
;
; --------------------------------------------------------------------------
