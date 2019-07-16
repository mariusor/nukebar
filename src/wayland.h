#ifndef NUKEBAR_WAYLAND_H
#define NUKEBAR_WAYLAND_H

#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <linux/input-event-codes.h>

#include "xdg-shell-client-protocol.h"
#include "render.h"

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t serial,  wl_fixed_t x,  wl_fixed_t y)
{
    _trace("pointer_motion[%p], pointer[%p] serial=%d [%d,%d]", data, pointer, serial, x, y);
}

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x,  wl_fixed_t y)
{
    _trace("pointer_enter[%p], pointer[%p] serial=%d [%d,%d] surface[%p]", data, pointer, serial, x, y, surface);
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial,  struct wl_surface *surface)
{
    _trace("pointer_leave[%p], pointer[%p] serial=%d surface[%p]", data, pointer, serial, surface);
}

static void pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t x,  uint32_t y,  wl_fixed_t pos)
{
    _trace("pointer_axis[%p], pointer[%p] pos=%d [%d,%d]", data, pointer, pos, x, y);
}

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *top, int32_t x, int32_t y, struct wl_array *list)
{
    _trace("xdg_toplevel_configure[%p], xdg_toplevel[%p] [%d,%d] arr[%p]", data, top, x, y, list);
    struct nukebar *bar = data;
    if (x != bar->width || y != bar->height) {
        wl_egl_window_resize(bar->window, x, y, x - bar->width, y - bar->height);

        bar->width = x;
        bar->height = y;
    }
}

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    _trace("xdg_surface[%p], serial=%d data[%p]", xdg_surface, serial, data);

    if (data == NULL) { return; }
    struct nukebar* bar = data;
    xdg_surface_ack_configure(xdg_surface, serial);
    wl_surface_commit(bar->surface);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
    _trace("xdg_toplevel_close[%p] top_level[%p]", data, xdg_toplevel);

    if (data == NULL) { return; }
    struct nukebar *bar = data;
    bar->stop = true;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
};

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state)
{
    _trace("pointer_button[%p], pointer[%p] serial=%d, time=%d, but=%d, state=%d", data, pointer, serial, time, button, state);
    pointer = (void*)pointer;
    time = (uint32_t)time;
    struct nukebar *bar = data;

    if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
        xdg_toplevel_move(bar->xdg_toplevel, bar->seat, serial);
    }
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
    _trace("seat_capabilities[%p], seat[%p], cap=%d", data, seat, capabilities);
    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        struct wl_pointer *pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(pointer, &pointer_listener, data);
    }
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
};

static void output_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
    _trace("output_mode[%p] wl_output[%p] flags=%d, size:%dx%d refresh=%d", data, wl_output, flags, width, height, refresh);
}

static void output_done(void *data, struct wl_output *wl_output) {
    _trace("output_done[%p] wl_output[%p]", data, wl_output);
}

static void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
    _trace("output_scale[%p] wl_output[%p] factor=%d", data, wl_output, factor);
}


static void output_geometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y, int32_t width_mm, int32_t height_mm, int32_t subpixel, const char *make, const char *model, int32_t transform)
{
    _trace("output_geometry[%p] wl_output[%p] pos: %dx%d size(mm):%dx%d subpixel=%d, %s-%s-%s %d", data, wl_output, x, y, width_mm, height_mm, subpixel, make, model, transform);
}

struct wl_output_listener output_listener = {
    .geometry = output_geometry,
    .mode = output_mode,
    .done = output_done,
    .scale = output_scale,
};
static void xdg_output_handle_logical_position(void *data, struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y)
{
    _trace("xdg_output_local_position[%p] xdg_output[%p] pos: %dx%d", data, xdg_output, x, y);
    struct nukebar *bar = data;
    bar->x = x;
    bar->y = y;
}

static void xdg_output_handle_logical_size(void *data, struct zxdg_output_v1 *xdg_output, int32_t width, int32_t height)
{
    _trace("xdg_output_local_size[%p] xdg_output[%p] size: %dx%d", data, xdg_output, width, height);
    struct nukebar *bar = data;
    bar->height = height;
    bar->width = width;
}

static void xdg_output_handle_done(void *data, struct zxdg_output_v1 *xdg_output)
{
    _trace("xdg_output_done[%p] xdg_output[%p]", data, xdg_output);
    struct nukebar *bar = data;
    bar->surface = wl_compositor_create_surface(bar->compositor);

    //determine_bar_visibility(bar, false);
}

static void xdg_output_handle_name(void *data, struct zxdg_output_v1 *xdg_output, const char *name)
{
    _trace("xdg_output_name[%p] xdg_output[%p] name=%s", data, xdg_output, name);
}

static void xdg_output_handle_description(void *data, struct zxdg_output_v1 *xdg_output, const char *description)
{
    _trace("xdg_output_description[%p] xdg_output[%p] description=%s", data, xdg_output, description);
}

struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = xdg_output_handle_logical_position,
    .logical_size = xdg_output_handle_logical_size,
    .done = xdg_output_handle_done,
    .name = xdg_output_handle_name,
    .description = xdg_output_handle_description,
};

static void add_xdg_output(struct nukebar *bar) {
    if (bar->xdg_output != NULL) {
        return;
    }
    bar->xdg_output = zxdg_output_manager_v1_get_xdg_output(bar->xdg_output_manager, bar->output);
    zxdg_output_v1_add_listener(bar->xdg_output, &xdg_output_listener, bar->output);
}

static void destroy_layer_surface(struct nukebar *bar) {
    if (!bar->layer_surface) {
        return;
    }
    zwlr_layer_surface_v1_destroy(bar->layer_surface);
    wl_surface_attach(bar->surface, NULL, 0, 0); // detach buffer
    bar->layer_surface = NULL;
    bar->width = 0;
    //output->frame_scheduled = false;
}

static void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface) {
    struct nukebar *bar = data;
    destroy_layer_surface(bar);
    _trace("layer_surface_closed[%p] %p", bar, surface);
}

static void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface, uint32_t serial, uint32_t width, uint32_t height)
{
    struct nukebar *bar = data;
    bar->width = width;
    bar->height = height;
    zwlr_layer_surface_v1_ack_configure(surface, serial);
    //set_output_dirty(bar);
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static void add_layer_surface(struct nukebar *bar)
{
    bar->layer_surface = zwlr_layer_shell_v1_get_layer_surface(bar->layer_shell, bar->surface, bar->output, ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM, "panel");
    uint32_t pos = 0;
    zwlr_layer_surface_v1_add_listener(bar->layer_surface, &layer_surface_listener, bar->output);
    zwlr_layer_surface_v1_set_anchor(bar->layer_surface, pos);
}

static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    struct nukebar* bar  = data;
    _trace("%45s[%03d:%d]: data[%p], registry[%p]", interface, name, version, data, bar->registry);
    if (bar->registry != registry) {
        _warn("different registry pointers: %p vs %p", bar->registry, registry);
    }
    version = (uint32_t)version;
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        bar->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        bar->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        bar->layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
    } else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
        bar->xdg_output_manager = wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
    }else if (strcmp(interface, wl_seat_interface.name) == 0) {
        bar->seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(bar->seat, &seat_listener, bar);
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        bar->output = wl_registry_bind(registry, name, &wl_output_interface, 3);
        wl_output_add_listener(bar->output, &output_listener, bar);
        if (bar->xdg_output_manager != NULL) {
            add_xdg_output(bar);
        }
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    _trace("data pointer %p", data);
    struct nukebar *bar  = data;
    registry = (void*)registry;
    if (bar->registry != registry) {
        _warn("registry pointers are different, %p vs %p", bar->registry, registry);
    }
    name = (uint32_t)name;
    // Who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};


static void frame_handle_done(void *data, struct wl_callback *callback, uint32_t time) {
    //_trace("callback%p, time=%d", callback, time);
    time = time;
    wl_callback_destroy(callback);
    struct nukebar *bar = data;

    if (!render(bar, time)) {
        _error("failed to render frame, stopping.");
        bar->stop = true;
    }
}

static const struct wl_callback_listener frame_listener = {
    .done = frame_handle_done,
};

/*
static void set_output_dirty(struct nukebar *bar)
{
    if (bar->surface) {
        render(bar, 0);
    }
}
*/

bool wayland_init(struct nukebar *bar)
{
    bar->width = 1;
    bar->height = 1;

    bar->display = wl_display_connect(NULL);
    if (bar->display == NULL) {
        _error("failed to create display");
        return false;
    }

    bar->registry = wl_display_get_registry(bar->display);
    wl_registry_add_listener(bar->registry, &registry_listener, bar);

    wl_display_dispatch(bar->display);

    bar->surface = wl_compositor_create_surface(bar->compositor);
    bar->xdg_surface = xdg_wm_base_get_xdg_surface(bar->xdg_wm_base, bar->surface);
    bar->xdg_toplevel = xdg_surface_get_toplevel(bar->xdg_surface);

    xdg_surface_add_listener(bar->xdg_surface, &xdg_surface_listener, bar);
    xdg_toplevel_add_listener(bar->xdg_toplevel, &xdg_toplevel_listener, bar);

    if (bar->compositor == NULL || bar->xdg_wm_base == NULL) {
        _error("no wl_compositor or xdg_wm_base support");
        return false;
    }

    bar->egl_display = eglGetDisplay((EGLNativeDisplayType)bar->display);
    if (bar->egl_display == EGL_NO_DISPLAY) {
        _error("failed to create EGL display");
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(bar->egl_display, &major, &minor)) {
        _error("failed to initialize EGL");
        return false;
    }

    EGLint count;
    eglGetConfigs(bar->egl_display, NULL, 0, &count);

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE,
    };
    EGLint n = 0;
    EGLConfig *configs = calloc(count, sizeof(EGLConfig));
    eglChooseConfig(bar->egl_display, config_attribs, configs, count, &n);
    if (n == 0) {
        _error("failed to choose an EGL config");
        free(configs);
        return false;
    }
    EGLConfig egl_config = configs[0];

    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE,
    };
    bar->egl_context = eglCreateContext(bar->egl_display, egl_config, EGL_NO_CONTEXT, context_attribs);

    bar->window = wl_egl_window_create(bar->surface, bar->width, bar->height);
    bar->egl_surface = eglCreateWindowSurface(bar->egl_display, egl_config, (EGLNativeWindowType)bar->window, NULL);

    wl_surface_commit(bar->surface);
    wl_display_roundtrip(bar->display);

    add_layer_surface(bar);

    free(configs);

    return true;
}

void wayland_destroy(struct nukebar *bar)
{
    if (NULL != bar->egl_display) {
        if (NULL != bar->egl_context) {
            eglDestroyContext(bar->egl_display, bar->egl_context);
        }
        if (NULL != bar->egl_surface) {
            eglDestroySurface(bar->egl_display, bar->egl_surface);
        }
        eglTerminate(bar->egl_display);

        wl_egl_window_destroy(bar->window);
    }
    if (NULL != bar->xdg_toplevel) {
        xdg_toplevel_destroy(bar->xdg_toplevel);
    }
    if (NULL != bar->xdg_surface) {
        xdg_surface_destroy(bar->xdg_surface);
    }
    if (NULL != bar->surface) {
        wl_surface_destroy(bar->surface);
    }
    if (NULL != bar->compositor) {
        wl_compositor_destroy(bar->compositor);
    }
    if (NULL != bar->registry) {
        wl_registry_destroy(bar->registry);
    }
    if (NULL != bar->display) {
        wl_display_disconnect(bar->display);
    }
    _info("destroyed everything");
}

#endif // NUKEBAR_WAYLAND_H
