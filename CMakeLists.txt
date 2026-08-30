cmake_minimum_required(VERSION 4.4)
project(Sophie LANGUAGES C OBJC)
add_library(cocoa_graphics 
               src/graphics.c 
               src/platform_cocoa.m
               src/font_freetype.c)

target_include_directories(cocoa_graphics
                           PRIVATE
                           src/
                           thirdparty/)

find_package(PkgConfig 
             REQUIRED 
             STATIC)
pkg_check_modules(FREETYPE 
                  REQUIRED 
                  freetype2)

find_library(COCOA 
             Cocoa 
             REQUIRED)

target_include_directories(cocoa_graphics 
                           PRIVATE 
                           ${FREETYPE_INCLUDE_DIRS})
target_compile_options(cocoa_graphics 
                       PRIVATE 
                       ${FREETYPE_CFLAGS_OTHER})

target_link_directories(cocoa_graphics 
                        PRIVATE 
                        ${FREETYPE_LIBRARY_DIRS})
target_link_libraries(cocoa_graphics 
                      PRIVATE 
                      ${FREETYPE_LIBRARIES}
                      ${COCOA})

set(CMAKE_BUILD_TYPE Release)
