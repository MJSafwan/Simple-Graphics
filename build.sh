#!/usr/bin/env bash

PROOT="$(pwd)"
cd src

cc -o $PROOT/main main.c font.c graphics.m graphics.c /Users/hoods/opt/freetype/lib/libfreetype.a -framework Cocoa -I/Users/hoods/opt/freetype/include/freetype2 -I$PROOT/thirdparty -lbz2 -L /opt/homebrew/opt/libpng/lib -lpng -L /opt/homebrew/opt/harfbuzz/lib/ -lharfbuzz -lz -g -O3
