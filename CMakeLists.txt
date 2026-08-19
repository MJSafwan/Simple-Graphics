cmake_minimum_required(VERSION 4.4)
project(Sophie LANGUAGES C OBJC)
add_executable(cocoa_demo 
               src/demo/main.c 
               src/graphics.c 
               src/platform_cocoa.m
               src/font_freetype.c)

target_include_directories(cocoa_demo
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

target_include_directories(cocoa_demo 
                           PRIVATE 
                           ${FREETYPE_INCLUDE_DIRS})
target_compile_options(cocoa_demo 
                       PRIVATE 
                       ${FREETYPE_CFLAGS_OTHER}
                       -O3)
target_link_directories(cocoa_demo 
                        PRIVATE 
                        ${FREETYPE_LIBRARY_DIRS})
target_link_libraries(cocoa_demo 
                      PRIVATE 
                      ${FREETYPE_LIBRARIES}
                      ${COCOA})






