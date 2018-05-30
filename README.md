# sctable
Parse stats from old StarCraft end of game screenshots.

Saving this here just for my own amusemed.
This code was written to be compiled in Windows 98 with DJGPP somewhere in the years 1999 and 2000 and I have not touched it since. To my surprise still compiles in linux with g++ perfectly and actually works.

## help
```sh
sctab options
options:
 a - set add mode to add
 r - set add mode to replace
 s - set add mode to substract
 n - don't add anything
 g - edd extra average points when creating html-file
 b filename - use statistics table file
 f filename - use a single file for adding
 m filename - creat html-file from the statistics table
 t filename - create txt-file from the statistics table
 h - print this help
 v - print version
Add mode specifies the way the statistics read
from file(s) should be added to the main table.
If option n is specified, then nothing will be add,
replaced or substracted.
In default every files which ends with .pcx will be
processed and if valid the information from them will
be added to statistics.
StarCraft statistics table editor v0.78
```

## compile
```sh
g++ -o sctable sctable.cc misc.cc
```
