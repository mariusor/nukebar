#ifndef NUKEBAR_HELLO_H
#define NUKEBAR_HELLO_H

#define _XOPEN_SOURCE 700
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <linux/input-event-codes.h>

#include "cat.h"
#include "shm.h"
#include "xdg-shell-client-protocol.h"

static const int width = 128;
static const int height = 128;


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
}

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    _trace("xdg_surface[%p], serial=%d data[%p]", xdg_surface, serial, data);

    if (data == NULL) { return; }
    struct nukebar* bar = (struct nukebar*)data;
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

static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    struct nukebar* bar  = (struct nukebar*)data;
    _trace("%45s[%03d:%d]: data[%p], registry[%p]", interface, name, version, data, bar->registry);
    if (bar->registry != registry) {
        _warn("different registry pointers: %p vs %p", bar->registry, registry);
    }
    version = (uint32_t)version;
    if (strcmp(interface, wl_shm_interface.name) == 0) {
        bar->shm = wl_registry_bind(bar->registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        bar->seat = wl_registry_bind(bar->registry, name, &wl_seat_interface, 1);
        wl_seat_add_listener(bar->seat, &seat_listener, bar);
    } else if (strcmp(interface, wl_compositor_interface.name) == 0) {
        bar->compositor = wl_registry_bind(bar->registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        bar->xdg_wm_base = wl_registry_bind(bar->registry, name, &xdg_wm_base_interface, 1);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    _trace("data pointer %p", data);
    struct nukebar *bar  = (void*)data;
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

static struct wl_buffer *create_buffer(struct wl_shm* shm)
{
    int stride = width * 4;
    int size = stride * height;

    int fd = create_shm_file(size);
    if (fd < 0) {
        _error("creating a buffer file for %d B failed: %m\n", size);
        return NULL;
    }

    void *shm_data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shm_data == MAP_FAILED) {
        _error("mmap failed: %s\n", strerror(errno));
        close(fd);
        return NULL;
    }

    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, size);
    struct wl_buffer *buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    // MagickImage is from cat.h
    memcpy(shm_data, MagickImage, size);
    return buffer;
}

int hello(struct nukebar *bar)
{
    bar->display = wl_display_connect(NULL);
    if (bar->display == NULL) {
        _error("failed to create display\n");
        return EXIT_FAILURE;
    }

    bar->registry = wl_display_get_registry(bar->display);
    wl_registry_add_listener(bar->registry, &registry_listener, bar);
    wl_display_roundtrip(bar->display);

    if (bar->shm == NULL || bar->compositor == NULL || bar->xdg_wm_base == NULL) {
        _error("no wl_shm, wl_compositor or xdg_wm_base support\n");
        return EXIT_FAILURE;
    }

    struct wl_buffer *buffer = create_buffer(bar->shm);
    if (buffer == NULL) {
        return EXIT_FAILURE;
    }

    bar->surface = wl_compositor_create_surface(bar->compositor);
    struct xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(bar->xdg_wm_base, bar->surface);
    bar->xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);

    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, bar);
    xdg_toplevel_add_listener(bar->xdg_toplevel, &xdg_toplevel_listener, bar);

    wl_surface_commit(bar->surface);
    wl_display_roundtrip(bar->display);

    wl_surface_attach(bar->surface, buffer, 0, 0);
    wl_surface_commit(bar->surface);

    while (wl_display_dispatch(bar->display) != -1 && !bar->stop) {
        // This space intentionally left blank
    }

    xdg_toplevel_destroy(bar->xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(bar->surface);
    wl_buffer_destroy(buffer);
    _info("exiting");
    return EXIT_SUCCESS;
}
#endif // NUKEBAR_HELLO_H
