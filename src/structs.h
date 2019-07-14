#ifndef NUKEBAR_STRUCTS_H
#define NUKEBAR_STRUCTS_H

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdbool.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

struct nukebar {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    //struct wl_list outputs;
    //struct wl_list seats;

    struct wl_surface *surface;
    //struct zwlr_layer_surface_v1 *layer_surface;
    //struct zwlr_layer_shell_v1 *layer_shell;
    struct xdg_toplevel *xdg_toplevel;
    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;

    struct wl_seat *seat;

    EGLDisplay egl_display;
    EGLContext egl_context;
    EGLSurface egl_surface;

    bool stop;
};
#endif // NUKEBAR_STRUCTS_H
