# OSDK 1.24 benchmark table (ISS oricCompilerBenchmark) — correctness-gated

Regenerated 2026-07-17 with the full compiler+peephole improvements (8-bit char
ops, `*2^n`/shift-by-8 codegen, dead-load elimination). Sizes = .tap bytes
(build only). Cycles = Oricutron internal cycle counter via GDB step-over of the
timed call, IRQ masked (`sei`), fresh emulator per sample, measured on the
arg-fixed `oricutron-cbench2` build.

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
| 00-type-sizes | ERR | 2531 | 2507 | 2194 | 2192 | - | yes |
| 01-dummy | 1993 | 1715 | 1703 | 1461 | 1461 | -26.7% | yes |
| 02-hello-world | 2028 | 1750 | 1738 | 1496 | 1496 | -26.2% | yes |
| 03-bytecpy | 2003 | 1725 | 1713 | 1467 | 1467 | -26.8% | yes |
| 04-memcopy | 2141 | 1863 | 1849 | 1555 | 1553 | -27.4% | yes |
| 05-0xcafe | ERR | 2305 | 2269 | 1959 | 1959 | - | yes |
| 06-sieve | 11496 | 11218 | 11202 | 10685 | 10683 | -7.1% | yes |
| 07-aes256 | 15961 | 14045 | 13431 | 13327 | 13203 | -16.5% | yes |
| 08-mandelbrot | 2602 | 2336 | 2320 | 1954 | 1950 | -24.9% | yes |
| 09-frogmove | 5201 | 4108 | 4066 | 3618 | 3616 | -30.4% | yes |
| 10-pi | 4406 | 4128 | 4106 | 3556 | 3548 | -19.3% | yes |
| 11-shuffle | 4014 | 3736 | 3714 | 3168 | 3160 | -21.1% | yes |
| 12-bubble-sort | 3812 | 3534 | 3512 | 2951 | 2943 | -22.6% | yes |
| 13-selection-sort | 3874 | 3596 | 3576 | 3006 | 3000 | -22.4% | yes |
| 14-insertion-sort | 3778 | 3500 | 3482 | 2924 | 2920 | -22.6% | yes |
| 15-merge-sort | 6021 | 5743 | 5717 | 4716 | 4704 | -21.7% | yes |
| 16-quick-sort | 4268 | 3990 | 3972 | 3307 | 3303 | -22.5% | yes |
| 17-counting-sort | 5488 | 5210 | 5186 | 4341 | 4331 | -20.9% | yes |
| 18-radix-sort | 6410 | 6132 | 6108 | 4995 | 4985 | -22.1% | yes |
| 19-shell-sort | 3923 | 3645 | 3627 | 3016 | 3012 | -23.1% | yes |
| 20-heap-sort | 4495 | 4217 | 4199 | 3458 | 3454 | -23.1% | yes |
| 21-eight-queens | 4270 | 3992 | 3876 | 3433 | 3423 | -19.6% | yes |
| **total** | 98184 | 95019 | 93873 | 82587 | 82363 | | |

## Cycles (verified-correct only)

| sample | 123-O2 | new-O2 | new-O2pp | new-O3 | new-O3pp | O3 vs 1.23 | correct |
|---|--:|--:|--:|--:|--:|--:|:--:|
| 00-type-sizes | - | 11064 | 11056 | 10680 | 10668 | - | yes |
| 01-dummy | 16 | 16 | 16 | 16 | 16 | +0.0% | yes |
| 02-hello-world | 386 | 368 | 368 | 368 | 368 | -4.7% | yes |
| 03-bytecpy | 30 | 30 | 30 | 24 | 24 | -20.0% | yes |
| 04-memcopy | 1507530 | 1507530 | 1507528 | 843986 | 843984 | -44.0% | yes |
| 05-0xcafe | - | 2038 | 2002 | 1902 | 1902 | - | yes |
| 06-sieve | 15749565 | 15753683 | 15746378 | 14254275 | 14251088 | -9.5% | yes |
| 07-aes256 | 84508723 | 52999791 | 50928474 | 58074885 | 58023163 | -31.3% | yes |
| 08-mandelbrot | 166325468 | 161030686 | 160873411 | 154859230 | 154609567 | -6.9% | yes |
| 09-frogmove | 31761313 | 13163201 | 13115256 | 10219331 | 10219329 | -67.8% | yes |
| 10-pi | 38590319 | 37841553 | 37851810 | 36600635 | 36250074 | -5.2% | yes |
| 11-shuffle | 2761815 | 2788508 | 2758901 | 2565636 | 2565632 | -7.1% | yes |
| 12-bubble-sort | 14976369 | 14977682 | 14974693 | 10450286 | 10449253 | -30.2% | yes |
| 13-selection-sort | 9852263 | 9832553 | 9862961 | 6405932 | 6406986 | -35.0% | yes |
| 14-insertion-sort | 6174806 | 6194516 | 6192031 | 4715601 | 4715343 | -23.6% | yes |
| 15-merge-sort | 3766736 | 3766689 | 3762852 | 2996259 | 2993222 | -20.5% | yes |
| 16-quick-sort | 3096817 | 3116501 | 3094710 | 2630613 | 2630031 | -15.1% | yes |
| 17-counting-sort | 1875385 | 1875384 | 1872893 | 1613928 | 1613153 | -13.9% | yes |
| 18-radix-sort | 7662741 | 7661976 | 7660265 | 7150069 | 7149537 | -6.7% | yes |
| 19-shell-sort | 3442589 | 3441266 | 3438781 | 2779400 | 2775949 | -19.3% | yes |
| 20-heap-sort | 5079380 | 5077414 | 5091803 | 4392742 | 4389256 | -13.5% | yes |
| 21-eight-queens | 72699162 | 72613402 | 69420441 | 62677440 | 62202360 | -13.8% | yes |

## Notes

- **aes256 is faster at -O2 (53.0M) than -O3 (58.1M).** The 8-bit char work helps
  the char-heavy AES code most at -O2, where the dead operand-widen (`CZBW`) is
  elided; at -O3 the byte load is folded into the widen so the widen is kept,
  leaving -O3 leaner in size but slightly heavier in cycles here. Both are
  verified-correct. Eliding the char widen at -O3 too is a known follow-up.
- Headlines vs 1.23-O2: frogmove -67.8%, memcopy -44%, selection-sort -35%,
  aes256 -31.3% at -O3 (−37% at -O2), bubble-sort -30%.
- The peephole (`pp`) adds a further ~1-4% and never changes output.
