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

@interface app_delegate: NSObject <NSApplicationDelegate>
@end

struct {
    /* Platform specific */
    NSWindow *window;
    window_view *view;
    NSImage *image;
    NSBitmapImageRep *reps;

    fb_t framebuffer;
    int current_buffer;
    FT_Library freetype;
    int width;
    int height;
    int window_should_close;
    char *title;

    struct {
        void (*resize_callback)(int, int);
    } callbacks;

    struct {
        char type[255];
        char keycode[255];
    } events;
} core = {0};

void set_resize_callback(void (*resize_callback)(int, int)) {
    core.callbacks.resize_callback = resize_callback;
}

FT_Library get_freetype() {
    return core.freetype;
}

void make_window(void) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    app_delegate *del = [[app_delegate alloc] init];
    assert(del);
    [NSApp setDelegate: del];

    core.window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, core.width, core.height)
                                         styleMask: NSWindowStyleMaskTitled|
                                                    NSWindowStyleMaskClosable|
                                                    NSWindowStyleMaskResizable|
                                                    NSWindowStyleMaskMiniaturizable
                                         backing: NSBackingStoreBuffered
                                         defer: NO]; 

    [core.window setTitle: @(core.title)];
    core.window.opaque = true;
    core.window.delegate = [[window_controller alloc] init];
    core.window.preservesContentDuringLiveResize = false;


    core.framebuffer = make_framebuffer(3000, 3000);
    core.framebuffer.width = core.width;
    core.framebuffer.height = core.height;

    core.view = [[window_view alloc] initWithFrame: NSMakeRect(0, 0, core.width, core.height)];
    [core.window setContentView:core.view];
}

@implementation app_delegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp stop: nil];
}

@end

@implementation window_controller

- (void)windowWillClose: (NSNotification *)notification {
    core.window_should_close = 1;
}
- (void) windowDidEndLiveResize:(NSNotification *) notification {
    core.framebuffer.width = core.width;
    core.framebuffer.height = core.height;
}

- (NSSize)windowWillResize: (NSWindow *)win toSize: (NSSize) size {
    core.width = size.width;
    core.height = size.height;

    core.framebuffer.width = core.width;
    core.framebuffer.height = core.height;

    for (int y = 0; y < core.framebuffer.height; ++y) {
        for (int x = 0; x < core.framebuffer.width; ++x) {
            core.framebuffer.dirty[x + y*core.framebuffer.width] = 1;
            core.framebuffer.data[x + y*core.framebuffer.width] = (rgba){0, 0, 0, 255};
        }
    }

    [core.view release];
    core.view = [[window_view alloc] initWithFrame: NSMakeRect(0, 0, core.width, core.height)];
    [core.window setContentView:core.view];

    [core.image release];
    core.image = [[NSImage alloc]initWithSize: size];
    core.reps = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes: NULL
                      pixelsWide: core.width
                      pixelsHigh: core.height
                   bitsPerSample: sizeof(char) * 8
                 samplesPerPixel: sizeof(rgba)
                        hasAlpha: YES
                        isPlanar: NO
                  colorSpaceName: NSDeviceRGBColorSpace
                     bytesPerRow: 0
                    bitsPerPixel: 0
    ];
    [core.image addRepresentation: core.reps];

    if (core.callbacks.resize_callback) {
        core.callbacks.resize_callback(size.width, size.height);
    }
    update_framebuffer();
    return size;
}

@end

@implementation window_view 

- (void) updateFramebuffer {
    [self setNeedsDisplay: YES];
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect: dirtyRect];
    [core.image drawInRect: dirtyRect];
}

- (BOOL) performKeyEquivalent:(NSEvent *) event {
    return YES;
}

@end

int is_key_down(int keycode) {
    return core.events.type[NSEventTypeKeyDown] && core.events.keycode[keycode]; 
}

int is_some_key_down() {
    return core.events.type[NSEventTypeKeyDown];
}

int is_some_key_up() {
    return core.events.type[NSEventTypeKeyUp];
}

int is_key_up(int keycode) {
    return core.events.type[NSEventTypeKeyUp] && core.events.keycode[keycode]; 
}

int is_being_resized() {
    return core.window.inLiveResize;
}

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
                     bytesPerRow: 0
                    bitsPerPixel: 0
    ];

    assert(reps);
    core.reps = reps;
    [core.image addRepresentation: reps];
    make_window();
    [core.window makeKeyAndOrderFront:nil];

    [NSApp activateIgnoringOtherApps:YES];
    if (![[NSRunningApplication currentApplication] isFinishedLaunching])
        [NSApp run];
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

    for (int y = 0; y < fb->height; ++y) {
        for (int x = 0; x < fb->width; ++x) {
            int i = x + fb->width * y;
            if (!fb->dirty[i])
                continue;
            fb->dirty[i] = 0;
            vals[0] = fb->data[i].r;
            vals[1] = fb->data[i].g;
            vals[2] = fb->data[i].b;
            vals[3] = fb->data[i].a;
            [core.reps setPixel: vals atX:x y:y];
        }
    }
    [core.view updateFramebuffer];
}

extern float dt;

void poll_events() {
    memset(&core.events, 0, sizeof(core.events));
    printf("dt = %f\n", dt);
    @autoreleasepool {
        for (;;) {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
                break;

            core.events.type[event.type] = 1;
            if (event.type == NSEventTypeKeyDown ||
                event.type == NSEventTypeKeyUp) {
                if (event.ARepeat)
                    continue;
                core.events.keycode[event.keyCode] = 1;
            }
            [NSApp sendEvent:event];
        }
    } 
}

void close_window() {
    FT_Done_FreeType(core.freetype);
    framebuffer_free(&core.framebuffer);
    [core.window close];

    [core.window release];
    [core.view release];
    [core.image release];
    [core.reps release];
}
