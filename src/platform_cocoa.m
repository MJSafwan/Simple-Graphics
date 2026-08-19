#import <Cocoa/Cocoa.h>
#include <stdlib.h>
#include <string.h>
#include "internals.h"

#define PLATFORM_COCOA

@interface window_view : NSView
- (void) updateFramebuffer;
@end

@interface window_controller : NSWindowController <NSWindowDelegate>
@end

@interface app_delegate: NSObject <NSApplicationDelegate>
@end

struct window_handle {
    NSWindow *window;
    window_view *view;
    NSImage *image;
    NSBitmapImageRep *reps;
};

struct screen {
    NSScreen *handle;
};

struct timer {
    struct timespec frame_start, frame_end;
};

extern core_t core;

/* Internal Platform Implementation */

@implementation app_delegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    [NSApp stop: nil];
}

@end

@implementation window_controller

- (void)windowWillClose: (NSNotification *)notification {
    if ((core.window.handle->window.styleMask & NSWindowStyleMaskFullScreen) != 0)
        [core.window.handle->window toggleFullScreen: nil];
    core.window.window_should_close = 1;
}

- (void) windowDidEndLiveResize:(NSNotification *) notification {
    fb_t *fb = get_framebuffer();
    fb->width = core.window.width;
    fb->height = core.window.height;
    if (core.callbacks.finish_resize_callback) {
        core.callbacks.finish_resize_callback(core.window.width, core.window.height);
    }
}

- (NSSize)windowWillResize: (NSWindow *)win toSize: (NSSize) size {
    core.window.width = size.width;
    core.window.height = size.height;

    fb_t *fb = get_framebuffer();
    fb->width = size.width;
    fb->height = size.height;

    for (int y = 0; y < size.height; ++y) {
        for (int x = 0; x < size.width; ++x) {
            fb->dirty[x + y*core.window.width] = 1;
            fb->data[x + y*core.window.width] = (rgba){0, 0, 0, 255};
        }
    }
    fb->dirty_box.minx = 0;
    fb->dirty_box.miny = 0;
    fb->dirty_box.maxx = core.window.width;
    fb->dirty_box.maxy = core.window.height;

    [core.window.handle->view release];
    core.window.handle->view = [[window_view alloc] initWithFrame: NSMakeRect(0, 0, core.window.width, core.window.height)];
    NSCAssert(core.window.handle->view, @"Couldn't allocate an NSView!\n");
    [core.window.handle->window setContentView:core.window.handle->view];

    [core.window.handle->image release];
    core.window.handle->image = [[NSImage alloc]initWithSize: size];
    NSCAssert(core.window.handle->image, @"Couldn't allocate an NSImage!\n");
    core.window.handle->reps = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes: NULL
                      pixelsWide: core.window.width
                      pixelsHigh: core.window.height
                   bitsPerSample: sizeof(char) * 8
                 samplesPerPixel: sizeof(rgba)
                        hasAlpha: YES
                        isPlanar: NO
                  colorSpaceName: NSDeviceRGBColorSpace
                     bytesPerRow: 0
                    bitsPerPixel: 0
    ];
    NSCAssert(core.window.handle->reps, @"Couldn't allocate an NSBitmapImageRep!\n");
    [core.window.handle->image addRepresentation: core.window.handle->reps];

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
    [core.window.handle->image drawInRect: dirtyRect];
}

- (BOOL) performKeyEquivalent:(NSEvent *) event {
    return YES;
}

@end

static void make_window(void) {
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    app_delegate *del = [[app_delegate alloc] init];
    NSCAssert(del, @"Couldn't allocate an NSApplicationDelegate!\n");
    [NSApp setDelegate: del];

    core.window.handle->window = [[NSWindow alloc] initWithContentRect: NSMakeRect(0, 0, core.window.width, core.window.height)
                                         styleMask: NSWindowStyleMaskTitled|
                                                    NSWindowStyleMaskClosable|
                                                    NSWindowStyleMaskResizable|
                                                    NSWindowStyleMaskMiniaturizable
                                         backing: NSBackingStoreBuffered
                                         defer: NO]; 

    NSCAssert(core.window.handle->window, @"Couldn't allocate NSWindow!\n");
    [core.window.handle->window setTitle: @(core.window.title)];
    core.window.handle->window.opaque = true;
    core.window.handle->window.delegate = [[window_controller alloc] init];
    NSCAssert(core.window.handle->window.delegate, @"Couldn't allocate NSWindowDelegate!\n");
    core.window.handle->window.preservesContentDuringLiveResize = false;

    /* todo: change current_screen when screen changes and resize framebuffer */
    core.current_screen->handle = [NSScreen mainScreen];
    core.window.framebuffer = make_framebuffer(core.current_screen->handle.frame.size.width, core.current_screen->handle.frame.size.height);
    core.window.framebuffer.width = core.window.width;
    core.window.framebuffer.height = core.window.height;

    core.window.handle->view = [[window_view alloc] initWithFrame: NSMakeRect(0, 0, core.window.width, core.window.height)];
    NSCAssert(core.window.handle->view, @"Couldn't allocate NSView!");
    [core.window.handle->window setContentView:core.window.handle->view];
}

/* Exported Function */

void init_platform() {
    core.window.handle = malloc(sizeof(window_handle_t));
    NSCAssert(core.window.handle, @"Couldn't allocate window_handle_t!\n");
    core.current_screen = malloc(sizeof(screen_t));
    NSCAssert(core.current_screen, @"Couldn't allocate screen_t!\n");
    core.timer = malloc(sizeof(timer_t));
    NSCAssert(core.timer, @"Couldn't allocate timer_t!\n");

    core.window.handle->image = [[NSImage alloc]initWithSize: NSMakeSize(core.window.width, core.window.height)];
    NSCAssert(core.window.handle->image, @"Couldn't allocate NSImage!\n");

    NSBitmapImageRep *reps = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes: NULL
                      pixelsWide: core.window.width
                      pixelsHigh: core.window.height
                   bitsPerSample: sizeof(char) * 8
                 samplesPerPixel: sizeof(rgba)
                        hasAlpha: YES
                        isPlanar: NO
                  colorSpaceName: NSDeviceRGBColorSpace
                     bytesPerRow: 0
                    bitsPerPixel: 0
    ];
    NSCAssert(reps, @"Couldn't allocate NSBitmapImageRep!\n");

    core.window.handle->reps = reps;
    [core.window.handle->image addRepresentation: reps];
    make_window();
    [core.window.handle->window makeKeyAndOrderFront:nil];

    [NSApp activateIgnoringOtherApps:YES];
    if (![[NSRunningApplication currentApplication] isFinishedLaunching])
        [NSApp run];
}


void sync_frames_start() {
    clock_gettime(CLOCK_MONOTONIC, &core.timer->frame_start);
}

void sync_frames_end() {
    clock_gettime(CLOCK_MONOTONIC, &core.timer->frame_end);
    int64_t etn = (int64_t)core.timer->frame_end.tv_sec*1000000000 + (int64_t)core.timer->frame_end.tv_nsec;
    int64_t stn = (int64_t)core.timer->frame_start.tv_sec*1000000000 + (int64_t)core.timer->frame_start.tv_nsec;
    core.dt = (etn-stn)/(1000000000.0);
}

void update_framebuffer() {
    fb_t *fb = get_framebuffer();
    NSUInteger vals[4] = {0};
    for (int y = fb->dirty_box.miny; y <= fb->dirty_box.maxy; ++y) {
        for (int x = fb->dirty_box.minx; x <= fb->dirty_box.maxx; ++x) {
            int i = x + fb->width * y;
            if (!fb->dirty[i])
                continue;
            fb->dirty[i] = 0;
            vals[0] = fb->data[i].r;
            vals[1] = fb->data[i].g;
            vals[2] = fb->data[i].b;
            vals[3] = 255;
            [core.window.handle->reps setPixel: vals atX:x y:y];
        }
    }

    fb->dirty_box.minx = 0;
    fb->dirty_box.miny = 0;
    fb->dirty_box.maxx = 0;
    fb->dirty_box.maxy = 0;
    [core.window.handle->view updateFramebuffer];
}

void poll_events() {
    memset(&core.events, 0, sizeof(core.events));
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

            if (core.callbacks.keydown_callback && event.type == NSEventTypeKeyDown) {
                core.callbacks.keydown_callback();
            }

            if (core.callbacks.keyup_callback && event.type == NSEventTypeKeyUp) {
                core.callbacks.keyup_callback();
            }

            [NSApp sendEvent:event];
        }
    } 
}

void uninit_platform() {
    [core.window.handle->window close];
    [core.window.handle->window release];
    [core.window.handle->view release];
    [core.window.handle->image release];
    [core.window.handle->reps release];
    free(core.window.handle);
    free(core.timer);
    free(core.current_screen);
}

