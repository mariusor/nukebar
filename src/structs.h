#ifndef NUKEBAR_STRUCTS_H
#define NUKEBAR_STRUCTS_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdbool.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

typedef union ivec2 {
    struct {
        int32_t x;
        int32_t y;
    };
    struct {
        int32_t w;
        int32_t h;
    };
    struct {
        int32_t u;
        int32_t v;
    };
    int32_t e[2];
} iv2;

struct nukebar_output {
    iv2 pos;
    iv2 size;
    struct wl_list next;
};

struct nukebar_seat {
    struct wl_list next;
};

struct nukebar {
    struct wl_egl_window *window;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_output *output;
    struct wl_surface *surface;
    struct wl_seat *seat;
    //struct wl_shm *shm;
    //struct wl_list outputs;
    //struct wl_list seats;

    struct xdg_toplevel *xdg_toplevel;
    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;

    struct zwlr_layer_surface_v1 *layer_surface;
	struct zxdg_output_manager_v1 *xdg_output_manager;
	struct zxdg_output_v1 *xdg_output;
    struct zwlr_layer_shell_v1 *layer_shell;

    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    struct timespec last_frame;

    bool stop;
};

#endif // NUKEBAR_STRUCTS_H
