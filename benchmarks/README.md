# Benchmark 

## 2021-06: Alpha 3

```
Run on (16 X 2300 MHz CPU s)
CPU Caches:
  L1 Data 32 KiB (x8)
  L1 Instruction 32 KiB (x8)
  L2 Unified 256 KiB (x8)
  L3 Unified 16384 KiB (x1)
Load Average: 2.94, 3.60, 3.46
------------------------------------------------------------------
Benchmark                        Time             CPU   Iterations
------------------------------------------------------------------
DBCreateOpen                 25924 ns        25920 ns        25768
DBOpenClose                  26393 ns        26390 ns        26994
DBWriteInt                   93714 ns        93595 ns         7645
DBMultipleIntWrites        9250266 ns      9241329 ns           76
DBRandomIntReads           1947999 ns      1951884 ns          361
DBWriteStruct                98879 ns        96940 ns         7622
DBMultipleStructWrites    10496381 ns     10501361 ns           72
DBRandomStructReads        2134887 ns      2138132 ns          340
BatchWriteSingleFile        235523 ns       226135 ns         2895
BatchWriteMultipleFiles     962288 ns       941011 ns          711
BatchReadMultipleFiles      777912 ns       777956 ns          886
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
