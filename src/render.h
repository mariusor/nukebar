#ifndef NUKEBAR_RENDER_H
#define NUKEBAR_RENDER_H

static uint32_t nk_color_to_xrgb8888(struct nk_color col)
{
    return (col.a << 24) + (col.r << 16) + (col.g << 8) + col.b;
}
static void bar_scissor(struct nukebar*, const float, const float, const float, const float);

static struct nk_color bar_int2color(const unsigned int i, wayland_pl pl)
{
    struct nk_color col = {0,0,0,0};

    switch (pl) {
    case PIXEL_LAYOUT_RGBX_8888:
        col.r = (i >> 24) & 0xff;
        col.g = (i >> 16) & 0xff;
        col.b = (i >> 8) & 0xff;
        col.a = i & 0xff;
        break;
    case PIXEL_LAYOUT_XRGB_8888:
        col.a = (i >> 24) & 0xff;
        col.r = (i >> 16) & 0xff;
        col.g = (i >> 8) & 0xff;
        col.b = i & 0xff;
        break;

    default:
        _error("nk_rawfb_int2color(): Unsupported pixel layout.");
        break;
    }
    return col;
}
static struct nk_color bar_getpixel(const struct nukebar* win, const int x0, const int y0)
{
    struct nk_color col = {0, 0, 0, 0};
    uint32_t *ptr;
    //unsigned int pixel;

    if (y0 < win->height && y0 >= 0 && x0 >= 0 && x0 < win->width) {
        ptr = win->data + (y0 * win->width);

        col = bar_int2color(*ptr, PIXEL_LAYOUT_XRGB_8888);
    }

    return col;
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
        _error("out of bound!");
    }
}
static void bar_blendpixel(const struct nukebar* win, const int x0, const int y0, struct nk_color col)
{
    struct nk_color col2;
    unsigned char inv_a;
    if (col.a == 0)
        return;

    inv_a = 0xff - col.a;
    col2 = bar_getpixel(win, x0, y0);
    col.r = (col.r * col.a + col2.r * inv_a) >> 8;
    col.g = (col.g * col.a + col2.g * inv_a) >> 8;
    col.b = (col.b * col.a + col2.b * inv_a) >> 8;
    bar_ctx_setpixel(win, x0, y0, col);
}

static struct nk_color bar_img_getpixel(const struct wayland_img *img, const int x0, const int y0)
{
    struct nk_color col = {0, 0, 0, 0};
    unsigned char *ptr;
    unsigned int pixel;
    NK_ASSERT(img);
    if (y0 < img->h && y0 >= 0 && x0 >= 0 && x0 < img->w) {
        ptr = img->pixels + (img->pitch * y0);

        if (img->format == NK_FONT_ATLAS_ALPHA8) {
            col.a = ptr[x0];
            col.b = col.g = col.r = 0xff;
        } else {
            pixel = ((unsigned int *)ptr)[x0];
            col = bar_int2color(pixel, img->pl);
        }
    } return col;
}

static void bar_copy_image(const struct nukebar *win, const struct wayland_img *src,
    const struct nk_rect *dst_rect,
    const struct nk_rect *src_rect,
    const struct nk_rect *dst_scissors,
    const struct nk_color *fg)
{
    short i, j;
    struct nk_color col;
    float xinc = src_rect->w / dst_rect->w;
    float yinc = src_rect->h / dst_rect->h;
    float xoff = src_rect->x, yoff = src_rect->y;

    // Simple nearest filtering rescaling
    // TODO: use bilinear filter
    for (j = 0; j < (short)dst_rect->h; j++) {
        for (i = 0; i < (short)dst_rect->w; i++) {
            if (dst_scissors) {
                if (i + (int)(dst_rect->x + 0.5f) < dst_scissors->x || i + (int)(dst_rect->x + 0.5f) >= dst_scissors->w)
                    continue;
                if (j + (int)(dst_rect->y + 0.5f) < dst_scissors->y || j + (int)(dst_rect->y + 0.5f) >= dst_scissors->h)
                    continue;
            }
            col = bar_img_getpixel(src, (int)xoff, (int) yoff);
	    if (col.r || col.g || col.b)
	    {
		col.r = fg->r;
		col.g = fg->g;
		col.b = fg->b;
	    }
            bar_blendpixel(win, i + (int)(dst_rect->x + 0.5f), j + (int)(dst_rect->y + 0.5f), col);
            xoff += xinc;
        }
        xoff = src_rect->x;
        yoff += yinc;
    }
}
static void bar_font_query_font_glyph(nk_handle handle, const float height, struct nk_user_font_glyph *glyph, const nk_rune codepoint, const nk_rune next_codepoint)
{
    float scale;
    const struct nk_font_glyph *g;
    struct nk_font *font;
    NK_ASSERT(glyph);
    NK_UNUSED(next_codepoint);

    font = (struct nk_font*)handle.ptr;
    NK_ASSERT(font);
    NK_ASSERT(font->glyphs);
    if (!font || !glyph)
        return;

    scale = height/font->info.height;
    g = nk_font_find_glyph(font, codepoint);
    glyph->width = (g->x1 - g->x0) * scale;
    glyph->height = (g->y1 - g->y0) * scale;
    glyph->offset = nk_vec2(g->x0 * scale, g->y0 * scale);
    glyph->xadvance = (g->xadvance * scale);
    glyph->uv[0] = nk_vec2(g->u0, g->v0);
    glyph->uv[1] = nk_vec2(g->u1, g->v1);
}

void bar_draw_text(const struct nukebar *win, const struct nk_user_font *font, const struct nk_rect rect, const char *text, const int len, const float font_height, const struct nk_color fg)
{
    float x = 0;
    int text_len = 0;
    nk_rune unicode = 0;
    nk_rune next = 0;
    int glyph_len = 0;
    int next_glyph_len = 0;
    struct nk_user_font_glyph g;
    if (!len || !text) return;

    x = 0;
    glyph_len = nk_utf_decode(text, &unicode, len);
    if (!glyph_len) return;

    // draw every glyph image
    while (text_len < len && glyph_len) {
        struct nk_rect src_rect;
        struct nk_rect dst_rect;
        float char_width = 0;
        if (unicode == NK_UTF_INVALID) break;

        // query currently drawn glyph information
        next_glyph_len = nk_utf_decode(text + text_len + glyph_len, &next, (int)len - text_len);
        bar_font_query_font_glyph(font->userdata, font_height, &g, unicode,
                    (next == NK_UTF_INVALID) ? '\0' : next);

        //calculate and draw glyph drawing rectangle and image
        char_width = g.xadvance;
        src_rect.x = g.uv[0].x * win->font_tex.w;
        src_rect.y = g.uv[0].y * win->font_tex.h;
        src_rect.w = g.uv[1].x * win->font_tex.w - g.uv[0].x * win->font_tex.w;
        src_rect.h = g.uv[1].y * win->font_tex.h - g.uv[0].y * win->font_tex.h;

        dst_rect.x = x + g.offset.x + rect.x;
        dst_rect.y = g.offset.y + rect.y;
        dst_rect.w = ceilf(g.width);
        dst_rect.h = ceilf(g.height);

        // Use software rescaling to blit glyph from font_text to framebuffer
        bar_copy_image(win, &(win->font_tex), &dst_rect, &src_rect, &(win->scissors), &fg);

        // offset next glyph
        text_len += glyph_len;
        x += char_width;
        glyph_len = next_glyph_len;
        unicode = next;
    }
}
static float color[3] = {0};
static size_t dec = 0;

static const struct wl_callback_listener frame_listener;

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
    const struct nk_command_scissor *s;
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
            s = (const struct nk_command_scissor*)cmd;
            bar_scissor(win, s->x, s->y, s->w, s->h);
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
            //bar_fill_triangle(win, t->a.x, t->a.y, t->b.x, t->b.y, t->c.x, t->c.y, t->color);
            break;

        case NK_COMMAND_POLYGON:
            _debug("NK_COMMAND_POLYGON");
            //const struct nk_command_polygon *p =(const struct nk_command_polygon*)cmd;
            //nk_rawfb_stroke_polygon(rawfb, p->points, p->point_count, p->line_thickness,p->color);
            break;

        case NK_COMMAND_POLYGON_FILLED:
            _debug("NK_COMMAND_POLYGON_FILLED");
            //p = (const struct nk_command_polygon_filled *)cmd;
            //bar_fill_polygon(win, p->points, p->point_count, p->color);
            break;

        case NK_COMMAND_POLYLINE:
            _debug("NK_COMMAND_POLYLINE");
            //const struct nk_command_polyline *p = (const struct nk_command_polyline *)cmd;
            //nk_rawfb_stroke_polyline(rawfb, p->points, p->point_count, p->line_thickness, p->color);
            break;

        case NK_COMMAND_TEXT:
            _debug("text");
            //tx = (const struct nk_command_text*)cmd;
            //bar_draw_text(win, tx->font, nk_rect(tx->x, tx->y, tx->w, tx->h), tx->string, tx->length, tx->height, tx->foreground);
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
    _trace("time %d - ms %d", time, ms);
    size_t inc = (dec + 1) % 3;
    color[inc] += ms / 2000.0f;
    color[dec] -= ms / 2000.0f;
    if (color[dec] < 0.0f) {
        color[inc] = 1.0f;
        color[dec] = 0.0f;
        dec = inc;
    }
    bar->last_frame = ts;

    _debug("start new frame");
    // And draw a new frame
    if (nk_begin(&(bar->ctx), "NukeBAR", nk_rect(50, 50, 200, 200), NK_WINDOW_BORDER)) {
            nk_layout_row_static(&(bar->ctx), 30, 80, 1);
            if (nk_button_label(&(bar->ctx), "button")){
                _info("button pressed");
            }
    }
    nk_end(&(bar->ctx));
    _debug("nk_end");

    if (nk_window_is_closed(&(bar->ctx), "NukeBAR")) return false;

    bar_render(bar, nk_rgb(30,30,30), 1);
    nk_input_begin(&(bar->ctx));
    wl_display_dispatch(bar->display);
    nk_input_end(&(bar->ctx));

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
