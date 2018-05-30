# sctable
Parse stats from old StarCraft end of game screenshots.

Saving this here just for my own amusemed.

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
