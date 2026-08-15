#import <Cocoa/Cocoa.h>
#import <Foundation/Foundation.h>
#import <stdio.h>
#import <assert.h>
#include "ft2build.h"
#include FT_FREETYPE_H

#include "graphics.h"


@interface window_view : NSView
- (void) updateFramebuffer;
@end

@interface window_controller : NSWindowController <NSWindowDelegate>
@end

struct {
    /* Platform specific */
    NSWindow *window;
    window_view *view;
    NSImage *image;
    NSBitmapImageRep *reps;

    fb_t framebuffer;
    regions_t regions;
    int current_buffer;
    FT_Library freetype;
    int width;
    int height;
    int window_should_close;
    char *title;
} core = {0};

FT_Library get_freetype() {
    return core.freetype;
}

void make_window(void) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];


    core.window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, core.width, core.height)
                                         styleMask: NSWindowStyleMaskTitled|
                                                    NSWindowStyleMaskClosable
                                         backing: NSBackingStoreBuffered
                                         defer: NO]; 

    [core.window setTitle: @(core.title)];
    core.window.delegate = [[window_controller alloc] init];

    core.framebuffer = make_framebuffer(core.width, core.height);
    // todo: make dim of regions based on screen res
    core.regions.dim = (Vector2){core.width/10, core.height/10};
    core.regions.items = malloc(core.regions.dim.x*core.regions.dim.y);
    memset(core.regions.items, 0, core.regions.dim.x*core.regions.dim.y);
    core.framebuffer.regions = &core.regions;

    core.view = [[window_view alloc] initWithFrame:((NSView *)[core.window contentView]).bounds];
    [core.window setContentView:core.view];
}


@implementation window_controller


- (void)windowWillClose: (NSNotification *)notification {
    core.window_should_close = 1;
}

- (NSSize)windowWillResize: (NSWindow *)win toSize: (NSSize) size {
    return size;
}

- (void)windowDidResize: (NSNotification *)notification {
    return;    
}

@end

@implementation window_view 

- (void) updateFramebuffer {
    [self setNeedsDisplay: YES];
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect: dirtyRect];
    [core.image drawInRect: self.bounds];
}

@end

void init_window(int width, int height, char *title) {
    core.width = width;
    core.height = height;
    core.title = title;
    FT_Init_FreeType(&core.freetype);

    core.image = [[NSImage alloc]initWithSize: NSMakeSize(core.width, core.height)];
    assert(core.image);

    NSBitmapImageRep *reps = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes: NULL
                      pixelsWide: core.width
                      pixelsHigh: core.height
                   bitsPerSample: sizeof(char) * 8
                 samplesPerPixel: sizeof(rgba)
                        hasAlpha: YES
                        isPlanar: NO
                  colorSpaceName: NSDeviceRGBColorSpace
                     bytesPerRow: core.width * sizeof(rgba)
                    bitsPerPixel: sizeof(rgba)*8
    ];

    assert(reps);
    core.reps = reps;
    [core.image addRepresentation: reps];
    make_window();
    [core.window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

fb_t *get_framebuffer() {
    return &core.framebuffer;
}

int window_should_close() {
    return core.window_should_close;
}

void update_framebuffer() {
    fb_t *fb = get_framebuffer();
    NSUInteger vals[4] = {0};
    int rx = (fb->width/core.regions.dim.x);
    int ry = (fb->height/core.regions.dim.y);

    for (int p = 0; p < core.regions.dim.y; ++p) {
        for (int q = 0; q < core.regions.dim.x; ++q) {
            int ri = q + p * core.regions.dim.x;
            if (!core.regions.items[ri])
                continue;
            core.regions.items[ri] = 0;
            for (int y = 0; y < ry; ++y) {
                for (int x = 0; x < rx; ++x) {
                    int tx = q * rx + x;
                    int ty = p * ry + y;
                    int i = tx + fb->width * ty;
                    if (!fb->dirty[i])
                        continue;
                    fb->dirty[i] = 0;
                    vals[0] = fb->data[i].r;
                    vals[1] = fb->data[i].g;
                    vals[2] = fb->data[i].b;
                    vals[3] = fb->data[i].a;
                    [core.reps setPixel: vals atX:tx y:ty];
                }
            }
        }
    }
    [core.view updateFramebuffer];
}

void poll_events() {
    @autoreleasepool {
        for (;;) {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
                break;

            [NSApp sendEvent:event];
        }
    } 
}

int get_window_height() {
    return core.height;
}

int get_window_width() {
    return core.width;
}

void close_window() {
    FT_Done_FreeType(core.freetype);
    framebuffer_free(&core.framebuffer);
    [core.window close];
}
