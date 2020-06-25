@echo off
rem SECTORS
del bb_sect.h
cutbmp 35 bbar.bmp _tmp 136 0 151 15
file2c _tmp bb_sect.h _BB_SECT_MKDOOR

rem MISC
del bb_misc.h
cutbmp 35 bbar.bmp _tmp 0 17 15 32
file2c _tmp bb_misc.h _BB_MISC_KILL
cutbmp 35 bbar.bmp _tmp 17 17 32 32
file2c _tmp bb_misc.h _BB_MISC_COPY
cutbmp 35 bbar.bmp _tmp 34 17 49 32
file2c _tmp bb_misc.h _BB_MISC_PASTE
cutbmp 35 bbar.bmp _tmp 68 17 83 32
file2c _tmp bb_misc.h _BB_MISC_GRID
cutbmp 35 bbar.bmp _tmp 85 17 100 32
file2c _tmp bb_misc.h _BB_MISC_SNAP
cutbmp 35 bbar.bmp _tmp 102 17 117 32
file2c _tmp bb_misc.h _BB_MISC_SAVE
cutbmp 35 bbar.bmp _tmp 119 17 134 32
file2c _tmp bb_misc.h _BB_MISC_LOAD
cutbmp 35 bbar.bmp _tmp 136 17 151 32
file2c _tmp bb_misc.h _BB_MISC_MAPDOWN
cutbmp 35 bbar.bmp _tmp 153 17 168 32
file2c _tmp bb_misc.h _BB_MISC_MAPUP
cutbmp 35 bbar.bmp _tmp 170 17 185 32
file2c _tmp bb_misc.h _BB_MISC_MAPRIGHT
cutbmp 35 bbar.bmp _tmp 187 17 202 32
file2c _tmp bb_misc.h _BB_MISC_MAPLEFT
cutbmp 35 bbar.bmp _tmp 204 17 219 32
file2c _tmp bb_misc.h _BB_MISC_ZOOMIN
cutbmp 35 bbar.bmp _tmp 221 17 236 32
file2c _tmp bb_misc.h _BB_MISC_ZOOMOUT
cutbmp 35 bbar.bmp _tmp 238 17 253 32
file2c _tmp bb_misc.h _BB_MISC_CUT
cutbmp 35 bbar.bmp _tmp 255 17 270 32
file2c _tmp bb_misc.h _BB_MISC_ROTATE
cutbmp 35 bbar.bmp _tmp 272 17 287 32
file2c _tmp bb_misc.h _BB_MISC_SCALE
cutbmp 35 bbar.bmp _tmp 289 17 304 32
file2c _tmp bb_misc.h _BB_MISC_CHECK
cutbmp 35 bbar.bmp _tmp 306 17 321 32
file2c _tmp bb_misc.h _BB_MISC_RUN

rem LINE
del bb_line.h
cutbmp 52 bbar.bmp _tmp 0 34 15 49
file2c _tmp bb_line.h _BB_LINE_EDITSD1
cutbmp 52 bbar.bmp _tmp 17 34 32 49
file2c _tmp bb_line.h _BB_LINE_EDITSD2
cutbmp 52 bbar.bmp _tmp 34 34 49 49
file2c _tmp bb_line.h _BB_LINE_FLIPSIDE
cutbmp 52 bbar.bmp _tmp 51 34 66 49
file2c _tmp bb_line.h _BB_LINE_FLIPLINE
cutbmp 52 bbar.bmp _tmp 68 34 83 49
file2c _tmp bb_misc.h _BB_LINE_MKSTAIR

:done
echo 
