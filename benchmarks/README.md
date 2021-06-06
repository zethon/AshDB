# Benchmark 

## 2021-06: Alpha 3

```
Run on (16 X 2300 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 256 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 2.17, 2.15, 2.40
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
DBCreateOpen                 26514 ns        26508 ns        26016
DBOpenClose                  26130 ns        26121 ns        26592
DBWriteInt                   97517 ns        97473 ns         7676
DBMultipleIntWrites        9921559 ns      9919770 ns           74
DBRandomIntReads           1918775 ns      1923551 ns          354
DBWriteStruct                99517 ns        98287 ns         7279
DBMultipleStructWrites     9789499 ns      9794758 ns           66
DBRandomStructReads        2061805 ns      2065728 ns          338
BatchWriteSingleFile        291230 ns       240452 ns         3053
BatchWriteMultipleFiles     948986 ns       940573 ns          562
BatchReadMultipleFiles      790605 ns       790495 ns          882
```

## 2021-06: Alpha 2
```
Run on (16 X 2300 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 256 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 1.38, 1.79, 1.91
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
DBCreateOpen                 26721 ns        26713 ns        26123
DBOpenClose                  26744 ns        26742 ns        26250
DBWriteInt                   91091 ns        91028 ns         7822
DBMultipleIntWrites        9226194 ns      9219063 ns           79
DBRandomIntReads           1860875 ns      1863817 ns          371
DBWriteStruct                99882 ns        98629 ns         7482
DBMultipleStructWrites     9522477 ns      9514260 ns           73
DBRandomStructReads        1988915 ns      1990536 ns          351
BatchWriteMultipleFiles    4966214 ns      4652037 ns          108
BatchReadMultipleFiles      778121 ns       778157 ns          918
```


## 2021-05: Alpha 1
```
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 256 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 2.11, 2.01, 1.79
--------------------------------------------------------------------
Benchmark                          Time             CPU   Iterations
--------------------------------------------------------------------
BM_DBCreateOpen                25794 ns        25792 ns        26568
BM_DBOpenClose                 25181 ns        25181 ns        27629
BM_DBWriteInt                  91354 ns        91269 ns         7487
BM_DBMultipleIntWrites       9255495 ns      9255213 ns           75
BM_DBRandomIntReads          1845182 ns      1846939 ns          378
BM_DBWriteStruct               97525 ns        95837 ns         7297
BM_DBMultipleStructWrites   10170736 ns     10177726 ns           73
BM_DBRandomStructReads       1975045 ns      1977746 ns          354
```
