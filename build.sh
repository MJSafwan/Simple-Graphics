#!/usr/bin/env bash

PROOT="$(pwd)"
cd src

# cc -o $PROOT/ncurses demo/main.c font_ncurses.c graphics.c platform_ncurses.c -I$PROOT/thirdparty -lncurses -g -O3 -I.

cc -o $PROOT/ncurses_freetype demo/main.c font_freetype.c graphics.c platform_ncurses.c -lncurses -I. /Users/hoods/opt/freetype/lib/libfreetype.a -framework Cocoa -I/Users/hoods/opt/freetype/include/freetype2 -I$PROOT/thirdparty -lbz2 -L /opt/homebrew/opt/libpng/lib -lpng -L /opt/homebrew/opt/harfbuzz/lib/ -lharfbuzz -lz -g -O3

# cc -o $PROOT/cocoa demo/main.c font_freetype.c graphics.c platform_cocoa.m -I. /Users/hoods/opt/freetype/lib/libfreetype.a -framework Cocoa -I/Users/hoods/opt/freetype/include/freetype2 -I$PROOT/thirdparty -lbz2 -L /opt/homebrew/opt/libpng/lib -lpng -L /opt/homebrew/opt/harfbuzz/lib/ -lharfbuzz -lz -g -O3
