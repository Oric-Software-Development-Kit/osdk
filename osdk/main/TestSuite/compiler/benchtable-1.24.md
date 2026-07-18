# OSDK 1.24 benchmark table (ISS oricCompilerBenchmark) — correctness-gated

Regenerated 2026-07-18 against the full 1.24 compiler+peephole set as of this
branch: 8-bit char ops (incl. the -O3 dead-char-widen elision, `ec00e1da`),
`*2^n`/shift-by-8 codegen, shift-unroll x<<2/x<<3, in-place inc/dec, byte
zero-compare, and the mature peephole (dead-load, copy-propagation,
comment-transparency, redundant-index-load, branch relaxation + orphan-fold
dissolve, epilogue-fold). Sizes = .tap bytes (build only). Cycles = Oricutron
internal cycle counter via GDB step-over of the timed call, IRQ masked (`sei`),
fresh emulator per sample, measured on the arg-fixed `oricutron-cbench2` build.

**Every metric is correctness-gated.** Before any size/cycle number is trusted,
each sample's *computed output* (routed through the printer) is required to be
byte-identical across all five configs (1.23-O2 as anchor where it builds). All
22 samples verified — see the correctness matrix. `ERR`/`-` on 1.23 = the two
samples 1.23 cannot build (0b literals / _Static_assert); their 4 new configs
still agree with each other.

## Correctness (output identical across all buildable configs; 1.23 anchor)

| sample | verified |
|---|:--:|
| 00-type-sizes | yes |
| 01-dummy | yes |
| 02-hello-world | yes |
| 03-bytecpy | yes |
| 04-memcopy | yes |
| 05-0xcafe | yes |
| 06-sieve | yes |
| 07-aes256 | yes |
| 08-mandelbrot | yes |
| 09-frogmove | yes |
| 10-pi | yes |
| 11-shuffle | yes |
| 12-bubble-sort | yes |
| 13-selection-sort | yes |
| 14-insertion-sort | yes |
| 15-merge-sort | yes |
| 16-quick-sort | yes |
| 17-counting-sort | yes |
| 18-radix-sort | yes |
| 19-shell-sort | yes |
| 20-heap-sort | yes |
| 21-eight-queens | yes |

## Code size (tap bytes)

| sample | 123-O2 | new-O2 | new-O2pp | new-O3 | new-O3pp | O3 vs 1.23 | correct |
|---|--:|--:|--:|--:|--:|--:|:--:|
| 00-type-sizes | ERR | 2526 | 2403 | 2181 | 2083 | - | yes |
| 01-dummy | 1993 | 1715 | 1631 | 1461 | 1395 | -26.7% | yes |
| 02-hello-world | 2028 | 1750 | 1665 | 1496 | 1429 | -26.2% | yes |
| 03-bytecpy | 2003 | 1725 | 1639 | 1467 | 1401 | -26.8% | yes |
| 04-memcopy | 2141 | 1863 | 1759 | 1555 | 1479 | -27.4% | yes |
| 05-0xcafe | ERR | 2305 | 2150 | 1959 | 1862 | - | yes |
| 06-sieve | 11496 | 11218 | 11056 | 10685 | 10584 | -7.1% | yes |
| 07-aes256 | 15961 | 13961 | 11901 | 12823 | 11286 | -19.7% | yes |
| 08-mandelbrot | 2602 | 2336 | 2211 | 1954 | 1879 | -24.9% | yes |
| 09-frogmove | 5201 | 4098 | 3888 | 3592 | 3460 | -30.9% | yes |
| 10-pi | 4406 | 4128 | 3985 | 3556 | 3457 | -19.3% | yes |
| 11-shuffle | 4014 | 3736 | 3580 | 3168 | 3033 | -21.1% | yes |
| 12-bubble-sort | 3812 | 3534 | 3352 | 2951 | 2830 | -22.6% | yes |
| 13-selection-sort | 3874 | 3596 | 3408 | 3006 | 2876 | -22.4% | yes |
| 14-insertion-sort | 3778 | 3500 | 3344 | 2924 | 2811 | -22.6% | yes |
| 15-merge-sort | 6021 | 5743 | 5439 | 4716 | 4498 | -21.7% | yes |
| 16-quick-sort | 4268 | 3990 | 3777 | 3307 | 3144 | -22.5% | yes |
| 17-counting-sort | 5488 | 5210 | 4961 | 4341 | 4181 | -20.9% | yes |
| 18-radix-sort | 6410 | 6132 | 5819 | 4995 | 4825 | -22.1% | yes |
| 19-shell-sort | 3923 | 3645 | 3473 | 3016 | 2893 | -23.1% | yes |
| 20-heap-sort | 4495 | 4217 | 3933 | 3458 | 3276 | -23.1% | yes |
| 21-eight-queens | 4270 | 3992 | 3751 | 3433 | 3301 | -19.6% | yes |
| **total** | 98184 | 94920 | 89125 | 82044 | 77983 | | |

## Cycles (verified-correct only)

| sample | 123-O2 | new-O2 | new-O2pp | new-O3 | new-O3pp | O3 vs 1.23 | correct |
|---|--:|--:|--:|--:|--:|--:|:--:|
| 00-type-sizes | - | 11104 | 10888 | 10596 | 10492 | - | yes |
| 01-dummy | 16 | 16 | 16 | 16 | 16 | +0.0% | yes |
| 02-hello-world | 386 | 368 | 368 | 368 | 368 | -4.7% | yes |
| 03-bytecpy | 30 | 30 | 27 | 24 | 24 | -20.0% | yes |
| 04-memcopy | 1507530 | 1507530 | 1302727 | 843986 | 762063 | -44.0% | yes |
| 05-0xcafe | - | 2038 | 1950 | 1902 | 1877 | - | yes |
| 06-sieve | 15749565 | 15753683 | 15433419 | 14254275 | 14253329 | -9.5% | yes |
| 07-aes256 | 84508723 | 50924202 | 38750984 | 48880631 | 38697670 | -42.2% | yes |
| 08-mandelbrot | 166325468 | 161030686 | 159845085 | 154859230 | 154556001 | -6.9% | yes |
| 09-frogmove | 31761313 | 12906161 | 12070803 | 9385481 | 8879887 | -70.4% | yes |
| 10-pi | 38590319 | 37841553 | 38302830 | 36600635 | 36593052 | -5.2% | yes |
| 11-shuffle | 2761815 | 2788508 | 2740918 | 2565636 | 2548789 | -7.1% | yes |
| 12-bubble-sort | 14976369 | 14977682 | 13546317 | 10450286 | 10251398 | -30.2% | yes |
| 13-selection-sort | 9852263 | 9832553 | 8682003 | 6405932 | 5843277 | -35.0% | yes |
| 14-insertion-sort | 6174806 | 6194516 | 5955948 | 4715601 | 4633016 | -23.6% | yes |
| 15-merge-sort | 3766736 | 3766689 | 3602039 | 2996259 | 2923533 | -20.5% | yes |
| 16-quick-sort | 3096817 | 3116501 | 2997863 | 2630613 | 2615477 | -15.1% | yes |
| 17-counting-sort | 1875385 | 1875384 | 1821045 | 1613928 | 1617809 | -13.9% | yes |
| 18-radix-sort | 7662741 | 7661976 | 7548791 | 7150069 | 7128216 | -6.7% | yes |
| 19-shell-sort | 3442589 | 3441266 | 3266957 | 2779400 | 2714084 | -19.3% | yes |
| 20-heap-sort | 5079380 | 5077414 | 4838677 | 4392742 | 4315692 | -13.5% | yes |
| 21-eight-queens | 72699162 | 72613402 | 69076157 | 62677440 | 61897471 | -13.8% | yes |

## Notes

- **The earlier aes256 "-O2 faster than -O3" anomaly is resolved.** At the last
  regen (2026-07-17) -O3 kept the folded char-widen (`CZBW`), leaving it smaller
  but *slower* than -O2 (58.1M vs 53.0M). The -O3 dead-char-widen elision
  (`ec00e1da`, byte-only load via `CWB`) fixed that: aes256 -O3 is now both
  smaller **and** faster than -O2 (48.9M vs 50.9M cyc; 12823 vs 13961 B), as it
  should be. With the peephole the two are essentially tied (-O3pp 38.70M,
  -O2pp 38.75M).
- **Headlines vs 1.23-O2 (new-O3):** frogmove −70.4%, memcopy −44.0%, aes256
  −42.2%, selection-sort −35.0%, bubble-sort −30.2% cycles; code size −20…31%
  typical (frogmove −30.9%), aes256 −19.7%. Totals: size 98184 → 77983 B
  (−20.8%) at -O3pp.
- **The peephole (`pp`) never changes output**, and its win now scales with how
  macro-heavy the code is. On tight scalar code it's small (~1-3%), but on
  char-heavy code the comment-transparency + copy-propagation passes clean up
  redundancy across macro seams for large gains: aes256 -O3 → -O3pp −20.8%
  cycles / −12% size, memcopy −9.7% cycles, selection-sort −8.8%. (At -O2 the
  peephole win is larger still — aes256 -O2 → -O2pp −23.9% cycles — because -O2
  leaves it more to clean.)
- `00-type-sizes` and `05-0xcafe` use `0b` literals / `_Static_assert` that 1.23
  cannot build (`ERR`/`-`); their four new configs agree with each other, so
  they remain correctness-verified.
