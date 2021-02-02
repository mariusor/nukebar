#ifndef NUKEBAR_RENDER_H
#define NUKEBAR_RENDER_H

static float color[3] = {0};
static size_t dec = 0;

static const struct wl_callback_listener frame_listener;

static uint32_t nk_color_to_xrgb8888(struct nk_color col)
{
    return (col.a << 24) + (col.r << 16) + (col.g << 8) + col.b;
}

static void bar_ctx_setpixel(const struct nukebar* win,
    const short x0, const short y0, const struct nk_color col)
{
    uint32_t c = nk_color_to_xrgb8888(col);
    uint32_t *pixels = win->data;
    unsigned int *ptr;

    pixels += (y0 * win->width);
    ptr = (unsigned int *)pixels + x0;

    if (y0 < win->scissors.h && y0 >= win->scissors.y && x0 >= win->scissors.x && x0 < win->scissors.w){
        *ptr = c;
    }else {
        _warn("out of bound!");
    }
}

static void bar_line_horizontal(const struct nukebar* win, const short x0, const short y, const short x1, const struct nk_color col)
{
    /* This function is called the most. Try to optimize it a bit...
     * It does not check for scissors or image borders.
     * The caller has to make sure it does no exceed bounds. */
    unsigned int i, n;
    unsigned int c[16];
    unsigned char *pixels = (uint8_t*)win->data;
    unsigned int *ptr;

    pixels += (y * (win->width * 4));
    ptr = (unsigned int *)pixels + x0;

    n = x1 - x0;
    for (i = 0; i < sizeof(c) / sizeof(c[0]); i++)
        c[i] = nk_color_to_xrgb8888(col);

    while (n > 16) {
        memcpy((void *)ptr, c, sizeof(c));
        n -= 16; ptr += 16;
    } for (i = 0; i < n; i++)
        ptr[i] = c[i];
}

static void bar_stroke_line(const struct nukebar* win, short x0, short y0, short x1, short y1, const unsigned int line_thickness, const struct nk_color col)
{
    short tmp;
    int dy, dx, stepx, stepy;

    dy = y1 - y0;
    dx = x1 - x0;

    // fast path
    if (dy == 0) {
        if (dx == 0 || y0 >= win->scissors.h || y0 < win->scissors.y){
            return;
        }

        if (dx < 0) {
            // swap x0 and x1
            tmp = x1;
            x1 = x0;
            x0 = tmp;
        }
        x1 = MIN(win->scissors.w, x1);
        x0 = MIN(win->scissors.w, x0);
        x1 = MAX(win->scissors.x, x1);
        x0 = MAX(win->scissors.x, x0);
        bar_line_horizontal(win, x0, y0, x1, col);
        return;
    }
    if (dy < 0) {
        dy = -dy;
        stepy = -1;
    } else stepy = 1;

    if (dx < 0) {
        dx = -dx;
        stepx = -1;
    } else stepx = 1;

    dy <<= 1;
    dx <<= 1;

    bar_ctx_setpixel(win, x0, y0, col);
    if (dx > dy) {
        int fraction = dy - (dx >> 1);
        while (x0 != x1) {
            if (fraction >= 0) {
                y0 += stepy;
                fraction -= dx;
            }
            x0 += stepx;
            fraction += dy;
            bar_ctx_setpixel(win, x0, y0, col);
        }
    } else {
        int fraction = dx - (dy >> 1);
        while (y0 != y1) {
            if (fraction >= 0) {
                x0 += stepx;
                fraction -= dy;
            }
            y0 += stepy;
            fraction += dx;
            bar_ctx_setpixel(win, x0, y0, col);
        }
    }
}

static void
bar_fill_rect(const struct nukebar* win,
    const short x, const short y, const short w, const short h,
    const short r, const struct nk_color col)
{
    int i;
    if (r == 0) {
        for (i = 0; i < h; i++)
            bar_stroke_line(win, x, y + i, x + w, y + i, 1, col);
    } else {
#if 0
        const short xc = x + r;
        const short yc = y + r;
        const short wc = (short)(w - 2 * r);
        const short hc = (short)(h - 2 * r);

        struct nk_vec2i pnts[12];
        pnts[0].x = x;
        pnts[0].y = yc;
        pnts[1].x = xc;
        pnts[1].y = yc;
        pnts[2].x = xc;
        pnts[2].y = y;

        pnts[3].x = xc + wc;
        pnts[3].y = y;
        pnts[4].x = xc + wc;
        pnts[4].y = yc;
        pnts[5].x = x + w;
        pnts[5].y = yc;

        pnts[6].x = x + w;
        pnts[6].y = yc + hc;
        pnts[7].x = xc + wc;
        pnts[7].y = yc + hc;
        pnts[8].x = xc + wc;
        pnts[8].y = y + h;

        pnts[9].x = xc;
        pnts[9].y = y + h;
        pnts[10].x = xc;
        pnts[10].y = yc + hc;
        pnts[11].x = x;
        pnts[11].y = yc + hc;

        bar_fill_polygon(win, pnts, 12, col);

        bar_fill_arc(win, xc + wc - r, y,
                (unsigned)r*2, (unsigned)r*2, 0 , col);
        bar_fill_arc(win, x, y,
                (unsigned)r*2, (unsigned)r*2, 90 , col);
        bar_fill_arc(win, x, yc + hc - r,
                (unsigned)r*2, (unsigned)r*2, 270 , col);
        bar_fill_arc(win, xc + wc - r, yc + hc - r,
                (unsigned)r*2, (unsigned)r*2, 180 , col);
#endif
    }
}
static void bar_clear(const struct nukebar *win, const struct nk_color col)
{
    bar_fill_rect(win, 0, 0, win->width, win->height, 0, col);
}
static void bar_render(struct nukebar *win, const struct nk_color clear, const unsigned char enable_clear)
{
    const struct nk_command *cmd;
    //const struct nk_command_text *tx;
    //const struct nk_command_scissor *s;
    const struct nk_command_rect_filled *rf;
    const struct nk_command_rect *r;
    //const struct nk_command_circle_filled *c;
    ///const struct nk_command_triangle_filled *t;
    const struct nk_command_line *l;
    //const struct nk_command_polygon_filled *p;

    if (enable_clear)
        bar_clear(win, clear);

    nk_foreach(cmd, (struct nk_context*)&(win->ctx)) {
        switch (cmd->type) {
        case NK_COMMAND_NOP:
            _debug("NK_COMMAND_NOP");
            break;

        case NK_COMMAND_SCISSOR:
            _debug("NK_COMMAND_SCISSOR");
            //s = (const struct nk_command_scissor*)cmd;
            //bar_scissor(win, s->x, s->y, s->w, s->h);
            break;

        case NK_COMMAND_LINE:
            l = (const struct nk_command_line *)cmd;
            _debug("stroke_line: begin(%d, %d) end(%d,%d) thickness %f, color: %d", l->begin.x, l->begin.y, l->end.x, l->end.y, l->line_thickness, l->color);
            break;

        case NK_COMMAND_RECT:
            r = (const struct nk_command_rect *)cmd;
            _debug("rect: %d,%d,%d,%d %u, thickness %f, color %d", r->x, r->y, r->w, r->h, (unsigned short)r->rounding, r->line_thickness, r->color);
            break;

        case NK_COMMAND_RECT_FILLED:
            rf = (const struct nk_command_rect_filled *)cmd;
            _debug("rect %d, %d, %d, %d %u color %d", rf->x, rf->y, rf->w, rf->h, (unsigned short)rf->rounding, rf->color);
            break;

        case NK_COMMAND_CIRCLE:
            _debug("NK_COMMAND_CIRCLE");
            //_debug("circle ", c->x, c->y, c->w, c->h, c->line_thickness, c->color);
            break;

        case NK_COMMAND_CIRCLE_FILLED:
            _debug("NK_COMMAND_CIRCLE_FILLED");
            //c = (const struct nk_command_circle_filled *)cmd;
            //_debug("circle filled", c->x, c->y, c->w, c->h, c->color);

            //const struct nk_command_circle_filled *c = (const struct nk_command_circle_filled *)cmd;
            //nk_rawfb_fill_circle(rawfb, c->x, c->y, c->w, c->h, c->color);
            break;

        case NK_COMMAND_TRIANGLE:
            _debug("NK_COMMAND_TRIANGLE");
            //const struct nk_command_triangle*t = (const struct nk_command_triangle*)cmd;
            //nk_rawfb_stroke_triangle(rawfb, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->line_thickness, t->color);
            break;

        case NK_COMMAND_TRIANGLE_FILLED:
            _debug("triangle filled");
            //t = (const struct nk_command_triangle_filled *)cmd;
            //nk_wayland_fill_triangle(win, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->color);
            break;

        case NK_COMMAND_POLYGON:
            _debug("NK_COMMAND_POLYGON");
            //const struct nk_command_polygon *p =(const struct nk_command_polygon*)cmd;
            //nk_rawfb_stroke_polygon(rawfb, p->points, p->point_count, p->line_thickness,p->color);
            break;

        case NK_COMMAND_POLYGON_FILLED:
            _debug("NK_COMMAND_POLYGON_FILLED");
            //p = (const struct nk_command_polygon_filled *)cmd;
            //nk_wayland_fill_polygon(win, p->points, p->point_count, p->color);
            break;

        case NK_COMMAND_POLYLINE:
            _debug("NK_COMMAND_POLYLINE");
            //const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
            //nk_rawfb_stroke_polyline(rawfb, p->points, p->point_count, p->line_thickness, p->color);
            break;

        case NK_COMMAND_TEXT:
            _debug("text");
            //tx = (const struct nk_command_text*)cmd;
            //nk_wayland_draw_text(win, tx->font, nk_rect(tx->x, tx->y, tx->w, tx->h), tx->string, tx->length, tx->height, tx->foreground);
            break;

        case NK_COMMAND_CURVE:
             _debug("NK_COMMAND_CURVE");
            //const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
            //nk_rawfb_stroke_curve(rawfb, q->begin, q->ctrl[0], q->ctrl[1], q->end, 22, q->line_thickness, q->color);
            break;

        case NK_COMMAND_RECT_MULTI_COLOR:
            _debug("NK_COMMAND_RECT_MULTI_COLOR");
            //const struct nk_command_rect_multi_color *q = (const struct nk_command_rect_multi_color *)cmd;
            //nk_rawfb_draw_rect_multi_color(rawfb, q->x, q->y, q->w, q->h, q->left, q->top, q->right, q->bottom);
            break;

        case NK_COMMAND_IMAGE:
            printf("NK_COMMAND_IMAGE");
           // const struct nk_command_image *q = (const struct nk_command_image *)cmd;
           // nk_rawfb_drawimage(rawfb, q->x, q->y, q->w, q->h, &q->img, &q->col);
            break;

        case NK_COMMAND_ARC:
            _debug("NK_COMMAND_ARC");
            assert(0 && "NK_COMMAND_ARC not implemented\n");
            break;

        case NK_COMMAND_ARC_FILLED:
            printf("NK_COMMAND_ARC \n");
            assert(0 && "NK_COMMAND_ARC_FILLED not implemented\n");
            break;

        default:
            _warn("unhandled OP: %d", cmd->type);
            break;
        }
    } nk_clear(&(win->ctx));
}

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

    // By default, eglSwapBuffers blocks until we receive the next frame event.
    // This is undesirable since it makes it impossible to process other events
    // (such as input events) while waiting for the next frame event. Setting
    // the swap interval to zero and managing frame events manually prevents
    // this behavior.
    if (nk_begin(&(bar->ctx), "NukeBAR", nk_rect(50, 50, 200, 200),
        NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|
        NK_WINDOW_CLOSABLE|NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE)) {
            enum {EASY, HARD};
            //static int op = EASY;
            //static int property = 20;

            nk_layout_row_static(&(bar->ctx), 30, 80, 1);
    }
    nk_end(&(bar->ctx));
    if (nk_window_is_closed(&(bar->ctx), "NukeBAR")) return false;

    bar_render(bar, nk_rgb(30,30,30), 1);
    //nk_input_begin((nk_wayland_ctx.ctx));
    wl_display_dispatch(bar->display);
    //nk_input_end(bar->ctx);

    struct wl_callback *callback = wl_surface_frame(bar->surface);
    wl_callback_add_listener(callback, &frame_listener, bar);

    wl_surface_commit(bar->surface);
    wl_surface_damage(bar->surface, 0, 0, bar->width, bar->height);

    // This will attach a new buffer and commit the surface
    if (false) {
        return false;
    }

    return true;
}


#endif // NUKEBAR_RENDER_H
