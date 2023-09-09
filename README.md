# frandom
Implementation of [Downey's method](https://allendowney.com/research/rand/) to generate [0 1) floating point values.
Attempt to accelerate this method with modified [Alias method](https://en.wikipedia.org/wiki/Alias_method), which is specialized for integer and `2^n` distribution.

# Performance

There are three implementations

1. frandom_table: using precomputed alias method tables
2. frandom_downey: straightforward but using a built-in bit operation
3. frandom_downey_opt: slightly add optims

Measure on next environment,

|||
|:---|:---|
|CPU|Core i5-10210U|
|OS|Debian 12.1|

## g++ 12.2.0
| | timing (millioseconds) |
|:---|:---|
|frandom_table|536|
|frandom_downey|371|
|frandom_downey_opt|268|

## clang++ 14.0.6
| | timing (millioseconds) |
|:---|:---|
|frandom_table|521|
|frandom_downey|362|
|frandom_downey_opt|292|

# Conclusion
Built-in functions are superior to cheap tricks. A straightforward implementation will be sufficient for general use cases.

