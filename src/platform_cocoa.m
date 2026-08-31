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

static void make_window(void);

@implementation app_delegate
- (void)applicationDidFinishLaunching:(NSNotification *)notification {
    graphics_logf(LOG_DEBUG, "make_window(): Creating window...\n");
    make_window();
    graphics_logf(LOG_DEBUG, "make_window(): Done\n");

    // [NSApp activateIgnoringOtherApps: YES];
    [core.window.handle->window makeKeyAndOrderFront:nil];
    [core.window.handle->window makeFirstResponder:core.window.handle->view];
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

- (void)windowDidLoad {
    [super windowDidLoad];

    graphics_logf(LOG_DEBUG, "Window loaded\n");
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
    core.window.handle->window = [[NSWindow alloc] 
        initWithContentRect: NSMakeRect(0, 0, core.window.width, core.window.height)
                  styleMask: NSWindowStyleMaskTitled|
                             NSWindowStyleMaskClosable|
                             NSWindowStyleMaskResizable|
                             NSWindowStyleMaskMiniaturizable
                    backing: NSBackingStoreBuffered
                      defer: NO]; 

    NSCAssert(core.window.handle->window, @"Couldn't allocate NSWindow!\n");
    [core.window.handle->window setTitle: @(core.window.title)];
    // core.window.handle->window.opaque = true;
    
    core.window.handle->window.delegate = [[window_controller alloc] init];
    NSCAssert(core.window.handle->window.delegate, @"Couldn't allocate NSWindowConstroler!\n");

    // core.window.handle->window.preservesContentDuringLiveResize = false;

    /* todo: change current_screen when screen changes and resize framebuffer */
    core.current_screen->handle = [NSScreen mainScreen];
    core.window.framebuffer = make_framebuffer(core.current_screen->handle.frame.size.width, core.current_screen->handle.frame.size.height);
    core.window.framebuffer.width = core.window.width;
    core.window.framebuffer.height = core.window.height;

    core.window.handle->view = [[window_view alloc] initWithFrame: NSMakeRect(0, 0, core.window.width, core.window.height)];
    NSCAssert(core.window.handle->view, @"Couldn't allocate NSView!");
    [core.window.handle->window setContentView:core.window.handle->view];

}
/* https://boredzo.org/blog/archives/2007-05-22/virtual-key-codes */

#define PLATFORM_KEY_A 0
#define PLATFORM_KEY_S 1
#define PLATFORM_KEY_D 2
#define PLATFORM_KEY_F 3
#define PLATFORM_KEY_H 4
#define PLATFORM_KEY_G 5
#define PLATFORM_KEY_Z 6
#define PLATFORM_KEY_X 7
#define PLATFORM_KEY_C 8
#define PLATFORM_KEY_V 9
#define PLATFORM_KEY_B 11
#define PLATFORM_KEY_Q 12
#define PLATFORM_KEY_W 13
#define PLATFORM_KEY_E 14
#define PLATFORM_KEY_R 15
#define PLATFORM_KEY_Y 16
#define PLATFORM_KEY_T 17
#define PLATFORM_KEY_1 18
#define PLATFORM_KEY_2 19
#define PLATFORM_KEY_3 20
#define PLATFORM_KEY_4 21
#define PLATFORM_KEY_6 22
#define PLATFORM_KEY_5 23
#define PLATFORM_KEY_EQUALS 24
#define PLATFORM_KEY_9 25
#define PLATFORM_KEY_7 26
#define PLATFORM_KEY_MINUS 27
#define PLATFORM_KEY_8 28
#define PLATFORM_KEY_0 29
#define PLATFORM_KEY_RIGHTBRACKET 30
#define PLATFORM_KEY_O 31
#define PLATFORM_KEY_U 32
#define PLATFORM_KEY_LEFTBRACKET 33
#define PLATFORM_KEY_I 34
#define PLATFORM_KEY_P 35
#define PLATFORM_KEY_RETURN 36
#define PLATFORM_KEY_L 37
#define PLATFORM_KEY_J 38
#define PLATFORM_KEY_APOSTROPHE 39
#define PLATFORM_KEY_K 40
#define PLATFORM_KEY_SEMICOLON 41
#define PLATFORM_KEY_FRONTSLASH 42
#define PLATFORM_KEY_COMMA 43
#define PLATFORM_KEY_BACKSLASH 44
#define PLATFORM_KEY_N 45
#define PLATFORM_KEY_M 46
#define PLATFORM_KEY_PERIOD 47
#define PLATFORM_KEY_TAB 48
#define PLATFORM_KEY_BACKAPOSTROPHE 50
#define PLATFORM_KEY_DELETE 51
#define PLATFORM_KEY_ESCAPE 53
#define PLATFORM_KEY_COMMAND 55
#define PLATFORM_KEY_SHIFT 56
#define PLATFORM_KEY_CAPSLOCK 57
#define PLATFORM_KEY_OPTION 58
#define PLATFORM_KEY_CONTROL 59
#define PLATFORM_KEY_UP 126
#define PLATFORM_KEY_DOWN 125
#define PLATFORM_KEY_LEFT 123
#define PLATFORM_KEY_RIGHT 124

static void register_keycodes(void) {
#define X(a, b) core.events.keycode_translation[b] = a;
    KEYCODEX
#undef X
}

/* Exported Function */

const char *platform_name = "Cocoa";

void init_platform() {

    core.window.handle = malloc(sizeof(window_handle_t));
    NSCAssert(core.window.handle, @"Couldn't allocate window_handle_t!\n");
    core.current_screen = malloc(sizeof(screen_t));
    NSCAssert(core.current_screen, @"Couldn't allocate screen_t!\n");
    core.timer = malloc(sizeof(timer_t));
    NSCAssert(core.timer, @"Couldn't allocate timer_t!\n");

    register_keycodes();

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

    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    app_delegate *del = [[app_delegate alloc] init];
    NSCAssert(del, @"Couldn't allocate an NSApplicationDelegate!\n");
    [NSApp setDelegate: del];
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
    for (int i = 0; i < GRAPHICS_KEY_LAST; ++i) {
        if (is_key_pressed(i)) {
            core.events.keycode[i] = GRAPHICS_KEYDOWN;
            if (core.callbacks.keydown_callback) {
                core.callbacks.keydown_callback();
            }
        }
        if (is_key_released(i)) {
            core.events.keycode[i] = GRAPHICS_KEYUP;
            if (core.callbacks.keyup_callback) {
                core.callbacks.keyup_callback();
            }
        }
    }

    @autoreleasepool {
        for (;;) {
            NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                                untilDate:[NSDate distantPast]
                                                   inMode:NSDefaultRunLoopMode
                                                  dequeue:YES];
            if (event == nil)
                break;
            


            switch (event.type) {
                case NSEventTypeKeyDown:
                    if (event.ARepeat)
                        break;
                    int keycode_generic = core.events.keycode_translation[event.keyCode];
                    int key_state = core.events.keycode[keycode_generic];
                    core.events.keycode[keycode_generic] = GRAPHICS_KEYPRESS | GRAPHICS_KEYDOWN;
                    if (core.callbacks.keypress_callback) {
                        core.callbacks.keypress_callback(keycode_generic);
                    }
                    if (core.callbacks.keydown_callback) {
                        core.callbacks.keydown_callback();
                    }
                    break;

                case NSEventTypeLeftMouseDown:
                    core.events.mouse_buttons[GRAPHICS_LM] = 1;
                    if (core.callbacks.lmouse_down_callback) {
                        core.callbacks.lmouse_down_callback();
                    }
                    break;

                case NSEventTypeLeftMouseUp:
                    core.events.mouse_buttons[GRAPHICS_RM] = 1;
                    if (core.callbacks.lmouse_up_callback) {
                        core.callbacks.lmouse_up_callback();
                    }
                    break;

                case NSEventTypeRightMouseDown:
                    core.events.mouse_buttons[GRAPHICS_LM] = 0;
                    if (core.callbacks.rmouse_down_callback) {
                        core.callbacks.rmouse_down_callback();
                    }
                    break;

                case NSEventTypeRightMouseUp:
                    core.events.mouse_buttons[GRAPHICS_RM] = 0;
                    if (core.callbacks.rmouse_up_callback) {
                        core.callbacks.rmouse_up_callback();
                    }
                    break;

                case NSEventTypeKeyUp:
                {
                    if (event.ARepeat)
                        break;

                    int keycode_generic = core.events.keycode_translation[event.keyCode];
                    core.events.keycode[keycode_generic] = GRAPHICS_KEYRELEASE | GRAPHICS_KEYUP;
                    if (core.callbacks.keyrelease_callback) {
                        core.callbacks.keyrelease_callback(keycode_generic);
                    }
                    if (core.callbacks.keyup_callback) {
                        core.callbacks.keyup_callback();
                    }
                } break;
                case NSEventTypeMouseEntered:
                    [core.window.handle->window makeKeyAndOrderFront:nil];
                    break;
                case NSEventTypeMouseMoved:
                {
                    NSPoint mouse_pos = [NSEvent mouseLocation];
                    mouse_pos = [core.window.handle->window convertPointFromScreen: mouse_pos];
                    mouse_pos.y = core.window.height - mouse_pos.y;
                    Vector2 mpos = (Vector2){mouse_pos.x, mouse_pos.y};
                    if (mpos.x >= 0 && mpos.y >= 0) {
                        if (mpos.x <= core.window.width && mpos.y <= core.window.height) {
                            core.events.window_mouse_pos = mpos;
                            if (core.callbacks.mouse_moved_callback) {
                                core.callbacks.mouse_moved_callback(mpos);
                            }
                        }
                    }
                } break;
                default:
                    break;
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

