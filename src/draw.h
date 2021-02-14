#ifndef NUKEBAR_DRAW_H
#define NUKEBAR_DRAW_H

static float color[3] = {0};
static size_t dec = 0;

static void bar_wayland_surf_clear(struct nukebar* bar)
{
    int x, y;
    int pix_idx;

    for (y = 0; y < bar->height; y++){
        for (x = 0; x < bar->width; x++){
            pix_idx = y * bar->width + x;
            bar->data[pix_idx] = 0xFF000000;
        }
    }
}

static void redraw(void *data, struct wl_callback *callback, uint32_t time)
{
}

static void bar_scissor(struct nukebar* bar, const float x, const float y, const float w, const float h)
{
    bar->scissors.x = MIN(MAX(x, 0), bar->width);
    bar->scissors.y = MIN(MAX(y, 0), bar->height);
    bar->scissors.w = MIN(MAX(w + x, 0), bar->width);
    bar->scissors.h = MIN(MAX(h + y, 0), bar->height);
}

static inline uint32_t nk_color_to_xrgb8888(struct nk_color col)
{
    return (col.a << 24) + (col.r << 16) + (col.g << 8) + col.b;
}

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

static void bar_ctx_setpixel(const struct nukebar* win, const short x0, const short y0, const struct nk_color col)
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

static void bar_fill_polygon(const struct nukebar* win, const struct nk_vec2i *pnts, int count, const struct nk_color col)
{
    int i = 0;
    //#define MAX_POINTS 64
    int left = 10000, top = 10000, bottom = 0, right = 0;
    int nodes, nodeX[1024], pixelX, pixelY, j, swap ;

    if (count == 0) return;
    if (count > 1024)
        count = 1024;

    /* Get polygon dimensions */
    for (i = 0; i < count; i++) {
        if (left > pnts[i].x)
            left = pnts[i].x;
        if (right < pnts[i].x)
            right = pnts[i].x;
        if (top > pnts[i].y)
            top = pnts[i].y;
        if (bottom < pnts[i].y)
            bottom = pnts[i].y;
    } bottom++; right++;

    /* Polygon scanline algorithm released under public-domain by Darel Rex Finley, 2007 */
    /*  Loop through the rows of the image. */
    for (pixelY = top; pixelY < bottom; pixelY ++) {
        nodes = 0; /*  Build a list of nodes. */
        j = count - 1;
        for (i = 0; i < count; i++) {
            if (((pnts[i].y < pixelY) && (pnts[j].y >= pixelY)) ||
                ((pnts[j].y < pixelY) && (pnts[i].y >= pixelY))) {
                nodeX[nodes++]= (int)((float)pnts[i].x
                                      + ((float)pixelY - (float)pnts[i].y) / ((float)pnts[j].y - (float)pnts[i].y)
                                      * ((float)pnts[j].x - (float)pnts[i].x));
            } j = i;
        }

        /*  Sort the nodes, via a simple “Bubble” sort. */
        i = 0;
        while (i < nodes - 1) {
            if (nodeX[i] > nodeX[i+1]) {
                swap = nodeX[i];
                nodeX[i] = nodeX[i+1];
                nodeX[i+1] = swap;
                if (i) i--;
            } else i++;
        }
        /*  Fill the pixels between node pairs. */
        for (i = 0; i < nodes; i += 2) {
            if (nodeX[i+0] >= right) break;
            if (nodeX[i+1] > left) {
                if (nodeX[i+0] < left) nodeX[i+0] = left ;
                if (nodeX[i+1] > right) nodeX[i+1] = right;
                for (pixelX = nodeX[i]; pixelX < nodeX[i + 1]; pixelX++)
                    bar_ctx_setpixel(win, pixelX, pixelY, col);
            }
        }
    }
}

static void bar_fill_triangle(const struct nukebar *win,
                                     const short x0, const short y0, const short x1, const short y1,
                                     const short x2, const short y2, const struct nk_color col)
{
    struct nk_vec2i pnts[3];
    pnts[0].x = x0;
    pnts[0].y = y0;
    pnts[1].x = x1;
    pnts[1].y = y1;
    pnts[2].x = x2;
    pnts[2].y = y2;
    bar_fill_polygon(win, pnts, 3, col);
}
static void bar_fill_arc(const struct nukebar* win, short x0, short y0, short w, short h, const short s, const struct nk_color col)
{
    /* Bresenham's ellipses - modified to fill one quarter */
    const int a2 = (w * w) / 4;
    const int b2 = (h * h) / 4;
    const int fa2 = 4 * a2, fb2 = 4 * b2;
    int x, y, sigma;
    struct nk_vec2i pnts[3];
    if (w < 1 || h < 1) return;
    if (s != 0 && s != 90 && s != 180 && s != 270)
        return;

    /* Convert upper left to center */
    h = (h + 1) / 2;
    w = (w + 1) / 2;
    x0 += w;
    y0 += h;

    pnts[0].x = x0;
    pnts[0].y = y0;
    pnts[2].x = x0;
    pnts[2].y = y0;

    /* First half */
    for (x = 0, y = h, sigma = 2*b2+a2*(1-2*h); b2*x <= a2*y; x++) {
        if (s == 180) {
            pnts[1].x = x0 + x; pnts[1].y = y0 + y;
        } else if (s == 270) {
            pnts[1].x = x0 - x; pnts[1].y = y0 + y;
        } else if (s == 0) {
            pnts[1].x = x0 + x; pnts[1].y = y0 - y;
        } else if (s == 90) {
            pnts[1].x = x0 - x; pnts[1].y = y0 - y;
        }
        bar_fill_polygon(win, pnts, 3, col);
        pnts[2] = pnts[1];
        if (sigma >= 0) {
            sigma += fa2 * (1 - y);
            y--;
        } sigma += b2 * ((4 * x) + 6);
    }

    /* Second half */
    for (x = w, y = 0, sigma = 2*a2+b2*(1-2*w); a2*y <= b2*x; y++) {
        if (s == 180) {
            pnts[1].x = x0 + x; pnts[1].y = y0 + y;
        } else if (s == 270) {
            pnts[1].x = x0 - x; pnts[1].y = y0 + y;
        } else if (s == 0) {
            pnts[1].x = x0 + x; pnts[1].y = y0 - y;
        } else if (s == 90) {
            pnts[1].x = x0 - x; pnts[1].y = y0 - y;
        }
        bar_fill_polygon(win, pnts, 3, col);
        pnts[2] = pnts[1];
        if (sigma >= 0) {
            sigma += fb2 * (1 - x);
            x--;
        } sigma += a2 * ((4 * y) + 6);
    }
}

static void bar_fill_rect(const struct nukebar* win, const short x, const short y,
              const short w, const short h, const short r, const struct nk_color col)
{
    int i;
    if (r == 0) {
        for (i = 0; i < h; i++)
            bar_stroke_line(win, x, y + i, x + w, y + i, 1, col);
    } else {
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

        bar_fill_arc(win, xc + wc - r, y, (unsigned)r*2, (unsigned)r*2, 0 , col);
        bar_fill_arc(win, x, y, (unsigned)r*2, (unsigned)r*2, 90 , col);
        bar_fill_arc(win, x, yc + hc - r, (unsigned)r*2, (unsigned)r*2, 270 , col);
        bar_fill_arc(win, xc + wc - r, yc + hc - r, (unsigned)r*2, (unsigned)r*2, 180 , col);
    }
}

static void bar_stroke_arc(const struct nukebar* win,
                                  short x0, short y0, short w, short h, const short s,
                                  const short line_thickness, const struct nk_color col)
{
    /* Bresenham's ellipses - modified to draw one quarter */
    const int a2 = (w * w) / 4;
    const int b2 = (h * h) / 4;
    const int fa2 = 4 * a2, fb2 = 4 * b2;
    int x, y, sigma;

    if (s != 0 && s != 90 && s != 180 && s != 270) return;
    if (w < 1 || h < 1) return;

    /* Convert upper left to center */
    h = (h + 1) / 2;
    w = (w + 1) / 2;
    x0 += w; y0 += h;

    /* First half */
    for (x = 0, y = h, sigma = 2*b2+a2*(1-2*h); b2*x <= a2*y; x++) {
        if (s == 180)
            bar_ctx_setpixel(win, x0 + x, y0 + y, col);
        else if (s == 270)
            bar_ctx_setpixel(win, x0 - x, y0 + y, col);
        else if (s == 0)
            bar_ctx_setpixel(win, x0 + x, y0 - y, col);
        else if (s == 90)
            bar_ctx_setpixel(win, x0 - x, y0 - y, col);
        if (sigma >= 0) {
            sigma += fa2 * (1 - y);
            y--;
        } sigma += b2 * ((4 * x) + 6);
    }

    /* Second half */
    for (x = w, y = 0, sigma = 2*a2+b2*(1-2*w); a2*y <= b2*x; y++) {
        if (s == 180)
            bar_ctx_setpixel(win, x0 + x, y0 + y, col);
        else if (s == 270)
            bar_ctx_setpixel(win, x0 - x, y0 + y, col);
        else if (s == 0)
            bar_ctx_setpixel(win, x0 + x, y0 - y, col);
        else if (s == 90)
            bar_ctx_setpixel(win, x0 - x, y0 - y, col);
        if (sigma >= 0) {
            sigma += fb2 * (1 - x);
            x--;
        } sigma += a2 * ((4 * y) + 6);
    }
}

static void bar_stroke_rect(const struct nukebar* win,
                                   const short x, const short y, const short w, const short h,
                                   const short r, const short line_thickness, const struct nk_color col)
{
    if (r == 0) {
        bar_stroke_line(win, x, y, x + w, y, line_thickness, col);
        bar_stroke_line(win, x, y + h, x + w, y + h, line_thickness, col);
        bar_stroke_line(win, x, y, x, y + h, line_thickness, col);
        bar_stroke_line(win, x + w, y, x + w, y + h, line_thickness, col);
    } else {
        const short xc = x + r;
        const short yc = y + r;
        const short wc = (short)(w - 2 * r);
        const short hc = (short)(h - 2 * r);

        bar_stroke_line(win, xc, y, xc + wc, y, line_thickness, col);
        bar_stroke_line(win, x + w, yc, x + w, yc + hc, line_thickness, col);
        bar_stroke_line(win, xc, y + h, xc + wc, y + h, line_thickness, col);
        bar_stroke_line(win, x, yc, x, yc + hc, line_thickness, col);

        bar_stroke_arc(win, xc + wc - r, y,
                              (unsigned)r*2, (unsigned)r*2, 0 , line_thickness, col);
        bar_stroke_arc(win, x, y,
                              (unsigned)r*2, (unsigned)r*2, 90 , line_thickness, col);
        bar_stroke_arc(win, x, yc + hc - r,
                              (unsigned)r*2, (unsigned)r*2, 270 , line_thickness, col);
        bar_stroke_arc(win, xc + wc - r, yc + hc - r,
                              (unsigned)r*2, (unsigned)r*2, 180 , line_thickness, col);
    }
}
#endif // NUKEBAR_DRAW_H