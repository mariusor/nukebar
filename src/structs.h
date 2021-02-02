#ifndef NUKEBAR_STRUCTS_H
#define NUKEBAR_STRUCTS_H

#include <stdbool.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
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

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#endif

typedef enum wayland_pixel_layout {
    PIXEL_LAYOUT_XRGB_8888,
    PIXEL_LAYOUT_RGBX_8888,
} wayland_pl;

struct wayland_img {
    void *pixels;
    int w, h, pitch;
    wayland_pl pl;
    //enum nk_font_atlas_format format;
};

struct nukebar {
    struct xdg_wm_base *xdg_wm_base;
    struct wl_backend *backend;
    struct wl_shm *wl_shm;
    struct wl_seat* seat;
    struct wl_callback *frame_callback;
    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct wl_buffer *front_buffer;
    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_output *output;

    //struct wl_list outputs;
    //struct wl_list seats;

    struct zwlr_layer_shell_v1 *layer_shell;
    struct zwlr_layer_surface_v1 *layer_surface;

    struct zxdg_output_manager_v1 *xdg_output_manager;
    struct zxdg_output_v1 *xdg_output;

    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
    struct timespec last_frame;

    bool stop;

    /*nuklear vars*/
    struct nk_context ctx;
    struct nk_rect scissors;
    //struct nk_font_atlas atlas;
    struct wayland_img font_tex;
    uint32_t *data;
    int mouse_pointer_x;
    int mouse_pointer_y;
    uint8_t tex_scratch[512 * 512];
};

#endif // NUKEBAR_STRUCTS_H
