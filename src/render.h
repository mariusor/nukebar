#ifndef NUKEBAR_RENDER_H
#define NUKEBAR_RENDER_H

static float color[3] = {0};
static size_t dec = 0;

static const struct wl_callback_listener frame_listener;

static bool render(struct nukebar *bar, uint32_t time) {
    time_t sec = time / 1000;
    uint32_t nsec = time * 1000000L - (sec * 1000);

    struct timespec ts = {
        .tv_sec = sec,
        .tv_nsec = nsec,
    };

    long ms = (ts.tv_sec - bar->last_frame.tv_sec) * 1000 + (ts.tv_nsec - bar->last_frame.tv_nsec) / 1000000;
    //_trace("time %d - ms %d", time, ms);
    size_t inc = (dec + 1) % 3;
    color[inc] += ms / 2000.0f;
    color[dec] -= ms / 2000.0f;
    if (color[dec] < 0.0f) {
        color[inc] = 1.0f;
        color[dec] = 0.0f;
        dec = inc;
    }
    bar->last_frame = ts;

    // And draw a new frame
    if (!eglMakeCurrent(bar->egl_display, bar->egl_surface, bar->egl_surface, bar->egl_context)) {
        _error("eglMakeCurrent failed");
        return false;
    }

    glClearColor(color[0], color[1], color[2], 0.4);
    glClear(GL_COLOR_BUFFER_BIT);

    // By default, eglSwapBuffers blocks until we receive the next frame event.
    // This is undesirable since it makes it impossible to process other events
    // (such as input events) while waiting for the next frame event. Setting
    // the swap interval to zero and managing frame events manually prevents
    // this behavior.
    eglSwapInterval(bar->egl_display, 0);

    zwlr_layer_surface_v1_set_size(bar->layer_surface, bar->width, bar->height);
    zwlr_layer_surface_v1_set_margin(bar->layer_surface, 0, 0, 0, 0);

    struct wl_callback *callback = wl_surface_frame(bar->surface);
    wl_callback_add_listener(callback, &frame_listener, bar);

    wl_surface_commit(bar->surface);
    wl_surface_damage(bar->surface, 0, 0, bar->width, bar->height);

    // This will attach a new buffer and commit the surface
    if (!eglSwapBuffers(bar->egl_display, bar->egl_surface)) {
        _error("eglSwapBuffers failed");
        return false;
    }

    return true;
}


#endif // NUKEBAR_RENDER_H
