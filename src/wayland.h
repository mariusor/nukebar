#ifndef NUKEBAR_WAYLAND_H
#define NUKEBAR_WAYLAND_H

#include <fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <linux/input-event-codes.h>

#include "xdg-shell-client-protocol.h"

#include "render.h"

#define DEFAULT_WIDTH_PX 1920
#define DEFAULT_HEIGHT_PX 30

static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t serial, wl_fixed_t x, wl_fixed_t y)
{
	_trace2("pointer_motion[%p], pointer[%p] serial=%d [%d,%d]", data, pointer, serial, x, y);
}

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y)
{
	_trace2("pointer_enter[%p], pointer[%p] serial=%d [%d,%d] surface[%p]", data, pointer, serial, x, y, surface);
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface)
{
	_trace2("pointer_leave[%p], pointer[%p] serial=%d surface[%p]", data, pointer, serial, surface);
}

static void pointer_handle_axis(void *data, struct wl_pointer *pointer, uint32_t x, uint32_t y, wl_fixed_t pos)
{
	_trace2("pointer_axis[%p], pointer[%p] pos=%d [%d,%d]", data, pointer, pos, x, y);
}

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *top, int32_t x, int32_t y, struct wl_array *list)
{
	_trace2("xdg_toplevel_configure[%p], xdg_toplevel[%p] [%d,%d] arr[%p]", data, top, x, y, list);
	//struct nukebar *bar = data;
	//if (x != bar->width || y != bar->height) {
	//	bar->width = x;
	//	bar->height = y;
	//}
}

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
	_trace2("xdg_surface_configure[%p], xdg_surface[%p] serial=%d", data, xdg_surface, serial);

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
	_trace2("xdg_toplevel_close[%p] top_level[%p]", data, xdg_toplevel);

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
	_trace2("pointer_button[%p], pointer[%p] serial=%d, time=%d, but=%d, state=%d", data, pointer, serial, time, button, state);
	/*
	struct nukebar *bar = data;

	if (button == BTN_LEFT && state == WL_POINTER_BUTTON_STATE_PRESSED) {
		xdg_toplevel_move(bar->xdg_toplevel, bar->seat, serial);
	}
	*/
}

static const struct wl_pointer_listener pointer_listener = {
	.enter = pointer_handle_enter,
	.leave = pointer_handle_leave,
	.motion = pointer_handle_motion,
	.button = pointer_handle_button,
	.axis = pointer_handle_axis,
};

static void seat_handle_capabilities(void *data, struct wl_seat *seat, uint32_t capabilities) {
	_trace2("seat_capabilities[%p], seat[%p], cap=%d", data, seat, capabilities);
	if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
		struct wl_pointer *pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(pointer, &pointer_listener, data);
	}
#if 0
	if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
		struct wl_keyboard *keyboard = wl_seat_get_keyboard (seat);
		wl_keyboard_add_listener (keyboard, &nk_wayland_keyboard_listener, win);
	}
#endif
}

static void seat_handle_name(void *data, struct wl_seat *seat, const char* name) {
	_trace2("seat_capabilities[%p], seat[%p], name=%s", data, seat, name);
}

static const struct wl_seat_listener seat_listener = {
	.capabilities = seat_handle_capabilities,
	.name = seat_handle_name,
};

static void output_mode(void *data, struct wl_output *wl_output, uint32_t flags, int32_t width, int32_t height, int32_t refresh)
{
	// @todo(marius): here we add the screens that we want to display the bar on to
	_trace2("output_mode[%p] wl_output[%p] flags=%d, size:%dx%d refresh=%d", data, wl_output, flags, width, height, refresh);
}

static void output_done(void *data, struct wl_output *wl_output) {
	_trace2("output_done[%p] wl_output[%p]", data, wl_output);
}

static void output_scale(void *data, struct wl_output *wl_output, int32_t factor) {
	_trace2("output_scale[%p] wl_output[%p] factor=%d", data, wl_output, factor);
#if 0
	struct nukebar *bar = data;
	bar->scale = scale;
	if (output->state->run_display && output->width > 0 && output->height > 0) {
		render_frame(output);
	}
#endif
}


static void output_geometry(void *data, struct wl_output *wl_output, int32_t x, int32_t y, int32_t width_mm, int32_t height_mm, int32_t subpixel, const char *make, const char *model, int32_t transform)
{
	_trace2("output_geometry[%p] wl_output[%p] pos: %dx%d size(mm):%dx%d subpixel=%d, %s-%s-%s %d", data, wl_output, x, y, width_mm, height_mm, subpixel, make, model, transform);
}

struct wl_output_listener output_listener = {
	.geometry = output_geometry,
	.mode = output_mode,
	.done = output_done,
	.scale = output_scale,
};

static void xdg_output_handle_logical_position(void *data, struct zxdg_output_v1 *xdg_output, int32_t x, int32_t y)
{
	_trace2("xdg_output_local_position[%p] xdg_output[%p] pos: %dx%d", data, xdg_output, x, y);
	struct nukebar *bar = data;
	bar->x = x;
	bar->y = y;
	_debug("bar pos: %dx%d", x, y);
}

static void xdg_output_handle_logical_size(void *data, struct zxdg_output_v1 *xdg_output, int32_t width, int32_t height)
{
	_trace2("xdg_output_local_size[%p] xdg_output[%p] size: %dx%d", data, xdg_output, width, height);
	struct nukebar *bar = data;
	// note(marius): we compute the width and height of the bar based on data from configuration
	//bar->height = width; //
	//bar->width = width;
	_debug("bar size: %dx%d", bar->width, bar->height);
}

static void destroy_layer_surface(struct nukebar *bar) {
	if (!bar->layer_surface) {
		return;
	}
	_trace2("destroy_layer_surface[%p] %p", bar);
	zwlr_layer_surface_v1_destroy(bar->layer_surface);
	//wl_surface_attach(bar->surface, NULL, 0, 0); // detach buffer
	bar->layer_surface = NULL;
	bar->width = 0;
	//output->frame_scheduled = false;
}

static void layer_surface_closed(void *data, struct zwlr_layer_surface_v1 *surface) {
	struct nukebar *bar = data;
	destroy_layer_surface(bar);
	_trace2("layer_surface_closed[%p] %p", bar, surface);
}

static void set_output_dirty(struct nukebar* bar)
{
	if (bar->surface) {
		render(bar, 0);
	}
}

static void layer_surface_configure(void *data, struct zwlr_layer_surface_v1 *surface, uint32_t serial, uint32_t width, uint32_t height)
{
	struct nukebar *bar = data;
	_trace2("layer_surface_configure[%p] layer_surface[%p] serial=%d w=%d h=%d", bar, surface, serial, width, height);
	bar->width = width;
	bar->height = height;
	zwlr_layer_surface_v1_ack_configure(surface, serial);
	set_output_dirty(bar);
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
	.configure = layer_surface_configure,
	.closed = layer_surface_closed,
};

static void add_layer_surface(struct nukebar *bar)
{
	_trace("setting surface: %dx%d", bar->width, bar->height);
	bool overlay = true;
	enum zwlr_layer_shell_v1_layer layer = overlay ? ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY : ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM;

	bar->layer_surface = zwlr_layer_shell_v1_get_layer_surface(bar->layer_shell, bar->surface, bar->output, layer, "panel");
	assert(bar->layer_surface);
	zwlr_layer_surface_v1_add_listener(bar->layer_surface, &layer_surface_listener, bar);
	if (overlay) {
		// Empty input region
		bar->input_region = wl_compositor_create_region(bar->compositor);
		assert(bar->input_region);

		wl_surface_set_input_region(bar->surface, bar->input_region);
	}

	zwlr_layer_surface_v1_set_anchor(bar->layer_surface, ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM);
	//zwlr_layer_surface_v1_set_size(bar->layer_surface, bar->width, bar->height);
	//zwlr_layer_surface_v1_set_margin(bar->layer_surface, 0, 0, 0, 0);
	if (overlay) {
		zwlr_layer_surface_v1_set_exclusive_zone(bar->layer_surface, -1);
	}
	//wl_surface_commit(bar->surface);
}

static void xdg_output_handle_done(void *data, struct zxdg_output_v1 *xdg_output)
{
	_trace2("xdg_output_done[%p] xdg_output[%p]", data, xdg_output);
	struct nukebar *bar = data;
	if (NULL == bar->surface) {
		bar->surface = wl_compositor_create_surface(bar->compositor);
		assert(bar->surface);
	}

	// Empty input region
	struct wl_region *input_region = wl_compositor_create_region(bar->compositor);
	assert(input_region);
	wl_surface_set_input_region(bar->surface, input_region);
	wl_region_destroy(input_region);

	add_layer_surface(bar);
}

static void xdg_output_handle_name(void *data, struct zxdg_output_v1 *xdg_output, const char *name)
{
	_trace2("xdg_output_name[%p] xdg_output[%p] name=%s", data, xdg_output, name);
}

static void xdg_output_handle_description(void *data, struct zxdg_output_v1 *xdg_output, const char *description)
{
	_trace2("xdg_output_description[%p] xdg_output[%p] description=%s", data, xdg_output, description);
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
	//	_trace2("skipping output");
		return;
	}
	_trace("adding output");
	bar->xdg_output = zxdg_output_manager_v1_get_xdg_output(bar->xdg_output_manager, bar->output);
	zxdg_output_v1_add_listener(bar->xdg_output, &xdg_output_listener, bar);
}

static void bar_xdg_wm_base_ping (void *data, struct xdg_wm_base *xdg_wm_base, uint32_t serial)
{
	_trace2("xdg_wm_base pong");
	xdg_wm_base_pong(xdg_wm_base, serial);
}

static void handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
	struct nukebar* bar  = data;
	_trace2("%45s[%03d:%d]: bar[%p]", interface, name, version, data);
	if (bar->registry != registry) {
		_warn("different registry pointers: %p vs %p", bar->registry, registry);
	}
	version = (uint32_t)version;
	if (strcmp(interface, wl_compositor_interface.name) == 0) {
		bar->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
	} else if (strcmp(interface, wl_seat_interface.name) == 0) {
		bar->seat = wl_registry_bind(registry, name, &wl_seat_interface, 3);
		wl_seat_add_listener(bar->seat, &seat_listener, bar);
	} else if (strcmp(interface, wl_output_interface.name) == 0) {
		// TODO(marius): this needs an array of outputs, because we should be able to draw the bar on
		//  different ones.
		bar->output = wl_registry_bind(registry, name, &wl_output_interface, 1);
		wl_output_add_listener(bar->output, &output_listener, bar);
		if (bar->xdg_output_manager != NULL) {
			add_xdg_output(bar);
		}
	} else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
		bar->layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
	} else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0) {
		bar->xdg_output_manager = wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
	//} else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
	//	bar->xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
	}else if (!strcmp(interface, wl_shm_interface.name)) {
		bar->wl_shm = wl_registry_bind (registry, name, &wl_shm_interface, 1);
	}
}

static void handle_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
	_trace2("data pointer %p", data);
	struct nukebar *bar = data;
	registry = (void*)registry;
	if (bar->registry != registry) {
		_warn("registry pointers are different, %p vs %p", bar->registry, registry);
	}
	name = (uint32_t)name;
#if 0
	wl_list_for_each_safe(output, tmp, &bar->outputs, link) {
		if (output->wl_name == name) {
			swaybar_output_free(output);
			return;
		}
	}
	wl_list_for_each_safe(output, tmp, &bar->unused_outputs, link) {
		if (output->wl_name == name) {
			swaybar_output_free(output);
			return;
		}
	}
	struct swaybar_seat *seat, *tmp_seat;
	wl_list_for_each_safe(seat, tmp_seat, &bar->seats, link) {
		if (seat->wl_name == name) {
			swaybar_seat_free(seat);
			return;
		}
	}
#endif
}

static const struct wl_registry_listener registry_listener = {
	.global = handle_global,
	.global_remove = handle_global_remove,
};

#define DEFAULT_INIT_POS_X 300
#define DEFAULT_INIT_POS_Y 300

static void frame_handle_done(void *data, struct wl_callback *callback, uint32_t time) {
	_trace2("callback%p, time=%d", callback, time);
	time = time;
	wl_callback_destroy(callback);
	struct nukebar *bar = data;

	_trace2("redrawing.. ");
#if 0
	struct nk_color col_red = {0xFF,0x00,0x00,0xA0}; //r,g,b,a
	struct nk_color col_green = {0x00,0xFF,0x00,0xA0}; //r,g,b,a
#endif
	wl_callback_destroy(bar->frame_callback);
	wl_surface_damage(bar->surface, 0, 0, bar->width, bar->height);

	bar->frame_callback = wl_surface_frame(bar->surface);
	wl_surface_attach(bar->surface, bar->front_buffer, 0, 0);
	wl_callback_add_listener(bar->frame_callback, &frame_listener, bar);
	if (!render(bar, time)) {
		_error("failed to render frame, stopping.");
		bar->stop = true;
	}
	wl_surface_commit(bar->surface);
}

static const struct wl_callback_listener frame_listener = {
	.done = frame_handle_done,
};

static bool bar_init(struct nukebar* win)
{
	const void *tex = {0};

	win->font_tex.pixels = win->tex_scratch;
	win->font_tex.format = NK_FONT_ATLAS_ALPHA8;
	win->font_tex.w = win->font_tex.h = 0;

	if (0 == nk_init_default(&(win->ctx), 0)) {
		_error("unable to initialize nuklear");
		return false;
	}
	_debug("initialized nuklear");

	nk_font_atlas_init_default(&(win->atlas));
	nk_font_atlas_begin(&(win->atlas));
	tex = nk_font_atlas_bake(&(win->atlas), &(win->font_tex.w), &(win->font_tex.h), win->font_tex.format);
	if (!tex) {
		_error("unable to allocate font atlas");
		return false;
	}

	switch(win->font_tex.format) {
	case NK_FONT_ATLAS_ALPHA8:
		win->font_tex.pitch = win->font_tex.w * 1;
		break;
	case NK_FONT_ATLAS_RGBA32:
		win->font_tex.pitch = win->font_tex.w * 4;
		break;
	};
	/* Store the font texture in tex scratch memory */
	memcpy(win->font_tex.pixels, tex, win->font_tex.pitch * win->font_tex.h);
	nk_font_atlas_end(&(win->atlas), nk_handle_ptr(NULL), NULL);
	if (win->atlas.default_font)
		nk_style_set_font(&(win->ctx), &(win->atlas.default_font->handle));
	nk_style_load_all_cursors(&(win->ctx), win->atlas.cursors);

	bar_scissor(win, 0, 0, win->width, win->height);
	_trace2("initialized font");

	return true;
}

bool wayland_init(struct nukebar *bar)
{
	bar->width = DEFAULT_WIDTH_PX;
	bar->height = DEFAULT_HEIGHT_PX;

	bar->display = wl_display_connect(NULL);
	if (bar->display == NULL) {
		_error("no wayland display found, is a wayland composer running? \n");
		return false;
	}

	bar->registry = wl_display_get_registry(bar->display);
	wl_registry_add_listener(bar->registry, &registry_listener, bar);
	wl_display_roundtrip(bar->display);

	wl_display_dispatch(bar->display);

	bar->surface = wl_compositor_create_surface(bar->compositor);
	//bar->xdg_surface = xdg_wm_base_get_xdg_surface(bar->xdg_wm_base, bar->surface);

	if (bar->compositor == NULL || bar->layer_shell == NULL || bar->xdg_output_manager == NULL) {
		_error("Unable to connect to the compositor");
		return false;
	}

	//bar->xdg_toplevel = xdg_surface_get_toplevel(bar->xdg_surface);
	//xdg_surface_add_listener(bar->xdg_surface, &xdg_surface_listener, bar);
	//xdg_toplevel_add_listener(bar->xdg_toplevel, &xdg_toplevel_listener, bar);
	bar->frame_callback = wl_surface_frame(bar->surface);

	wl_surface_commit(bar->surface);

	size_t size = bar->width * bar->height * 4;
	char *xdg_runtime_dir = getenv ("XDG_RUNTIME_DIR");
	int fd = open (xdg_runtime_dir, O_TMPFILE|O_RDWR|O_EXCL, 0600);
	ftruncate (fd, size);
	bar->data = mmap (NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	struct wl_shm_pool *pool = wl_shm_create_pool (bar->wl_shm, fd, size);
	bar->front_buffer = wl_shm_pool_create_buffer (pool, 0, bar->width, bar->height, bar->width*4, WL_SHM_FORMAT_XRGB8888);
	wl_shm_pool_destroy (pool);
	close (fd);

	wl_display_roundtrip(bar->display);

	//3. Clear window and start rendering loop
	bar_wayland_surf_clear(bar);
	wl_surface_set_buffer_scale(bar->surface, 1);
	wl_surface_attach (bar->surface, bar->front_buffer, 0, 0);
	wl_surface_damage_buffer(bar->surface, 0, 0, INT32_MAX, INT32_MAX);
	wl_surface_commit (bar->surface);

	//free(configs);

	return true;
}

void wayland_destroy(struct nukebar *bar)
{
	nk_free(&(bar->ctx));
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
