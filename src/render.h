#ifndef NUKEBAR_RENDER_H
#define NUKEBAR_RENDER_H

static void bar_clear(const struct nukebar *win, const struct nk_color col)
{
    bar_fill_rect(win, 0, 0, win->width, win->height, 0, col);
}

static void bar_render(struct nukebar *win, const struct nk_color clear, const unsigned char enable_clear)
{
    const struct nk_command *cmd;
    const struct nk_command_text *tx;
    const struct nk_command_scissor *s;
    const struct nk_command_rect_filled *rf;
    const struct nk_command_rect *r;
    const struct nk_command_circle *c;
    const struct nk_command_circle_filled *cf;
    const struct nk_command_triangle *t;
    const struct nk_command_triangle_filled *tf;
    const struct nk_command_line *l;
    const struct nk_command_polygon *p;
    const struct nk_command_polygon_filled *pf;
    const struct nk_command_polyline *pl;

    if (enable_clear) {
        bar_clear(win, clear);
    }

    nk_foreach(cmd, (struct nk_context*)&(win->ctx)) {
        switch (cmd->type) {
        case NK_COMMAND_NOP:
            _trace2("NK_COMMAND_NOP");
            break;

        case NK_COMMAND_SCISSOR:
            s = (const struct nk_command_scissor*)cmd;
            _trace2("NK_COMMAND_SCISSOR [%d, %d, %d, %d]", s->x, s->y, s->w, s->h);
            bar_scissor(win, s->x, s->y, s->w, s->h);
            break;

        case NK_COMMAND_LINE:
            l = (const struct nk_command_line *)cmd;
            _trace2("NK_COMMAND_LINE st[%d, %d] end[%d, %d] line[%f, %d %d %d]", l->begin.x, l->begin.y, l->end.x, l->end.y, l->line_thickness, l->color.r, l->color.g, l->color.b);
            bar_stroke_line(win, l->begin.x, l->begin.y, l->end.x, l->end.y, l->line_thickness, l->color);
            break;

        case NK_COMMAND_RECT:
            r = (const struct nk_command_rect *)cmd;
            _trace2("NK_COMMAND_RECT center[%d, %d], w:%d, h:%d %u line[%f, %d %d %d]", r->x, r->y, r->w, r->h, (unsigned short)r->rounding, r->line_thickness, r->color.r, r->color.g, r->color.b);
            bar_stroke_rect(win, r->x, r->y, r->w, r->h, (unsigned short)r->rounding, r->line_thickness, r->color);
            break;

        case NK_COMMAND_RECT_FILLED:
            rf = (const struct nk_command_rect_filled *)cmd;
            _trace2("NK_COMMAND_RECT_FILLED center[%d, %d], w:%d, h:%d %u color %d %d %d", rf->x, rf->y, rf->w, rf->h, (unsigned short)rf->rounding, rf->color.r, rf->color.g, rf->color.b);
            bar_fill_rect(win, rf->x, rf->y, rf->w, rf->h, (unsigned short)rf->rounding, rf->color);
            break;

        case NK_COMMAND_CIRCLE:
            c = (const struct nk_command_circle *)cmd;
            _trace2("NK_COMMAND_CIRCLE center[%d,%d], radius[%d, %d], line[%d, %d %d %d]", c->x, c->y, c->w, c->h, c->line_thickness, c->color.r, c->color.g, c->color.b);
            // todo(marius): pick it up from rawfb example
            //bar_stroke_circle(win, c->x, c->y, c->w, c->h, c->line_thickness, c->color);
            break;

        case NK_COMMAND_CIRCLE_FILLED:
            cf = (const struct nk_command_circle_filled *)cmd;
            _trace2("NK_COMMAND_CIRCLE_FILLED center[%d,%d], radius[%d, %d], fill[%d %d %d]", cf->x, cf->y, cf->w, cf->h, cf->color.r, cf->color.g, cf->color.b);
            // todo(marius): pick it up from rawfb example
            //bar_fill_circle(win, cf->x, cf->y, cf->w, cf->h, cf->color);
            break;

        case NK_COMMAND_TRIANGLE:
            t = (const struct nk_command_triangle*)cmd;
            _trace2("NK_COMMAND_TRIANGLE A[%d,%d], B[%d, %d], C[%d,%d] line[%d, %d %d %d]", t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->line_thickness, t->color.r, t->color.g, t->color.b);
            // todo(marius): pick it up from rawfb example
            //bar_stroke_triangle(win, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->line_thickness, t->color);
            break;

        case NK_COMMAND_TRIANGLE_FILLED:
            tf = (const struct nk_command_triangle_filled *)cmd;
            _trace2("NK_COMMAND_TRIANGLE_FILLED A[%d,%d], B[%d, %d], C[%d,%d] fill[%d %d %d]", tf->a.x, tf->a.y, tf->b.x, tf->b.y, tf->c.x, tf->c.y, tf->color.r, tf->color.g, tf->color.b);
            bar_fill_triangle(win, tf->a.x, tf->a.y, tf->b.x, tf->b.y, tf->c.x, tf->c.y, tf->color);
            break;

        case NK_COMMAND_POLYGON:
            p = (const struct nk_command_polygon*)cmd;
            _trace2("NK_COMMAND_POLYGON points[%d], point_count[%d], line[%f, %d %d %d]", p->points, p->point_count, p->line_thickness,p->color.r, p->color.g, p->color.b);
            // todo(marius): pick it up from rawfb example
            //bar_stroke_polygon(win, p->points, p->point_count, p->line_thickness,p->color);
            break;

        case NK_COMMAND_POLYGON_FILLED:
            pf = (const struct nk_command_polygon_filled *)cmd;
            _trace2("NK_COMMAND_POLYGON_FILLED points[%d], count[%d], color[%d %d %d]", pf->points, pf->point_count, pf->color.r, pf->color.g, pf->color.b);
            // todo(marius): pick it up from rawfb example
            //bar_fill_polygon(win, p->points, p->point_count, p->color);
            break;

        case NK_COMMAND_POLYLINE:
            pl = (const struct nk_command_polyline *)cmd;
            _trace2("NK_COMMAND_POLYLINE points[%d] count[%d] thickness[%d] color[%d %d %d]", pl->points, pl->point_count, pl->line_thickness, pl->color.r, pl->color.g, pl->color.b);
            // todo(marius): pick it up from rawfb example
            //bar_stroke_polyline(win, pl->points, pl->point_count, pl->line_thickness, pl->color);
            break;

        case NK_COMMAND_TEXT:
            tx = (const struct nk_command_text*)cmd;
            _trace2("NK_COMMAND_TEXT rect(%d, %d, %d, %d) %s - l %d, %d color(%f)", tx->x, tx->y, tx->w, tx->h, tx->string, tx->length, tx->height, tx->foreground);
            bar_draw_text(win, tx->font, nk_rect(tx->x, tx->y, tx->w, tx->h), tx->string, tx->length, tx->height, tx->foreground);
            break;

        case NK_COMMAND_CURVE:
             _trace2("NK_COMMAND_CURVE");
            //const struct nk_command_curve *q = (const struct nk_command_curve *)cmd;
            // todo(marius): pick it up from rawfb example
            //bar_stroke_curve(win, q->begin, q->ctrl[0], q->ctrl[1], q->end, 22, q->line_thickness, q->color);
            break;

        case NK_COMMAND_RECT_MULTI_COLOR:
            _trace2("NK_COMMAND_RECT_MULTI_COLOR");
            //const struct nk_command_rect_multi_color *q = (const struct nk_command_rect_multi_color *)cmd;
            // todo(marius): pick it up from rawfb example
            //bar_draw_rect_multi_color(win, q->x, q->y, q->w, q->h, q->left, q->top, q->right, q->bottom);
            break;

        case NK_COMMAND_IMAGE:
           _trace2("NK_COMMAND_IMAGE");
           // const struct nk_command_image *q = (const struct nk_command_image *)cmd;
           // bar_drawimage(win, q->x, q->y, q->w, q->h, &q->img, &q->col);
            break;

        case NK_COMMAND_ARC:
            _trace2("NK_COMMAND_ARC");
            assert(0 && "NK_COMMAND_ARC not implemented\n");
            break;

        case NK_COMMAND_ARC_FILLED:
            _trace2("NK_COMMAND_ARC \n");
            assert(0 && "NK_COMMAND_ARC_FILLED not implemented\n");
            break;

        default:
            _warn("unhandled OP: %d", cmd->type);
            break;
        }
    }
    nk_clear(&(win->ctx));
}

static const struct wl_callback_listener frame_listener;
static bool render(struct nukebar *bar, uint32_t time) {
    time_t sec = time / 1000;
    uint32_t nsec = time * 1000000L - (sec * 1000);

    struct timespec ts = {
        .tv_sec = sec,
        .tv_nsec = nsec,
    };

    long ms = (ts.tv_sec - bar->last_frame.tv_sec) * 1000 + (ts.tv_nsec - bar->last_frame.tv_nsec) / 1000000;
    if (ms > 0) {
        _trace("time %d - ms %d", time, ms);
    }
    size_t inc = (dec + 1) % 3;
    color[inc] += ms / 2000.0f;
    color[dec] -= ms / 2000.0f;
    if (color[dec] < 0.0f) {
        color[inc] = 1.0f;
        color[dec] = 0.0f;
        dec = inc;
    }
    bar->last_frame = ts;

    assert(bar->display);
    _debug("start new frame");

    _trace2("nk_begin");
    if (nk_begin(&(bar->ctx), "NukeBAR", nk_rect(0, 0, bar->width, bar->height), NK_WINDOW_BORDER | NK_WINDOW_NO_INPUT | NK_WINDOW_BACKGROUND)) {
        nk_layout_row_static(&(bar->ctx), bar->height, bar->width, 1);
#if 0
        if (nk_button_label(&(bar->ctx), "button")) {
            _info("button pressed");
        }
#endif
    }
    nk_end(&(bar->ctx));
    _debug("nk_end");

    if (nk_window_is_closed(&(bar->ctx), "NukeBAR")) {
        return false;
    }

    bar_render(bar, nk_rgb(30,30,30), 0);

    _trace2("nk_input");
    nk_input_begin(&(bar->ctx));
    wl_display_dispatch(bar->display);
    nk_input_end(&(bar->ctx));

    struct wl_callback *callback = wl_surface_frame(bar->surface);
    wl_callback_add_listener(callback, &frame_listener, bar);

    wl_surface_commit(bar->surface);
    // @todo(marius): here we need to pass the area that was rendered by nuklear
    // for start, we assume the whole bar needs repainting
    wl_surface_damage(bar->surface, 0, 0, bar->width, bar->height);

    // This will attach a new buffer and commit the surface
    return true;
}

#endif // NUKEBAR_RENDER_H
