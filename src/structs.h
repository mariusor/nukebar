#ifndef NUKEBAR_STRUCTS_H
#define NUKEBAR_STRUCTS_H

#include <stdbool.h>
#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"

struct nukebar {
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_shm *shm;
    struct wl_list outputs;
    struct wl_list seats;

    struct wl_surface *surface;
    struct zwlr_layer_surface_v1 *layer_surface;
    struct zwlr_layer_shell_v1 *layer_shell;

    bool running;
};
#endif // NUKEBAR_STRUCTS_H
