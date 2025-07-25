/*
 * Copyright (C) 2001-2011 Michael Niedermayer <michaelni@gmx.at>
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "config.h"
#include "swscale.h"
#include "swscale_internal.h"
#include "rgb2rgb.h"
#include "libavutil/intreadwrite.h"
#include "libavutil/avutil.h"
#include "libavutil/mathematics.h"
#include "libavutil/mem_internal.h"
#include "libavutil/bswap.h"
#include "libavutil/pixdesc.h"
#include "libavutil/avassert.h"
#include "libavutil/avconfig.h"

DECLARE_ALIGNED(8, static const uint8_t, dithers)[8][8][8]={
{
  {   0,  1,  0,  1,  0,  1,  0,  1,},
  {   1,  0,  1,  0,  1,  0,  1,  0,},
  {   0,  1,  0,  1,  0,  1,  0,  1,},
  {   1,  0,  1,  0,  1,  0,  1,  0,},
  {   0,  1,  0,  1,  0,  1,  0,  1,},
  {   1,  0,  1,  0,  1,  0,  1,  0,},
  {   0,  1,  0,  1,  0,  1,  0,  1,},
  {   1,  0,  1,  0,  1,  0,  1,  0,},
},{
  {   1,  2,  1,  2,  1,  2,  1,  2,},
  {   3,  0,  3,  0,  3,  0,  3,  0,},
  {   1,  2,  1,  2,  1,  2,  1,  2,},
  {   3,  0,  3,  0,  3,  0,  3,  0,},
  {   1,  2,  1,  2,  1,  2,  1,  2,},
  {   3,  0,  3,  0,  3,  0,  3,  0,},
  {   1,  2,  1,  2,  1,  2,  1,  2,},
  {   3,  0,  3,  0,  3,  0,  3,  0,},
},{
  {   2,  4,  3,  5,  2,  4,  3,  5,},
  {   6,  0,  7,  1,  6,  0,  7,  1,},
  {   3,  5,  2,  4,  3,  5,  2,  4,},
  {   7,  1,  6,  0,  7,  1,  6,  0,},
  {   2,  4,  3,  5,  2,  4,  3,  5,},
  {   6,  0,  7,  1,  6,  0,  7,  1,},
  {   3,  5,  2,  4,  3,  5,  2,  4,},
  {   7,  1,  6,  0,  7,  1,  6,  0,},
},{
  {   4,  8,  7, 11,  4,  8,  7, 11,},
  {  12,  0, 15,  3, 12,  0, 15,  3,},
  {   6, 10,  5,  9,  6, 10,  5,  9,},
  {  14,  2, 13,  1, 14,  2, 13,  1,},
  {   4,  8,  7, 11,  4,  8,  7, 11,},
  {  12,  0, 15,  3, 12,  0, 15,  3,},
  {   6, 10,  5,  9,  6, 10,  5,  9,},
  {  14,  2, 13,  1, 14,  2, 13,  1,},
},{
  {   9, 17, 15, 23,  8, 16, 14, 22,},
  {  25,  1, 31,  7, 24,  0, 30,  6,},
  {  13, 21, 11, 19, 12, 20, 10, 18,},
  {  29,  5, 27,  3, 28,  4, 26,  2,},
  {   8, 16, 14, 22,  9, 17, 15, 23,},
  {  24,  0, 30,  6, 25,  1, 31,  7,},
  {  12, 20, 10, 18, 13, 21, 11, 19,},
  {  28,  4, 26,  2, 29,  5, 27,  3,},
},{
  {  18, 34, 30, 46, 17, 33, 29, 45,},
  {  50,  2, 62, 14, 49,  1, 61, 13,},
  {  26, 42, 22, 38, 25, 41, 21, 37,},
  {  58, 10, 54,  6, 57,  9, 53,  5,},
  {  16, 32, 28, 44, 19, 35, 31, 47,},
  {  48,  0, 60, 12, 51,  3, 63, 15,},
  {  24, 40, 20, 36, 27, 43, 23, 39,},
  {  56,  8, 52,  4, 59, 11, 55,  7,},
},{
  {  18, 34, 30, 46, 17, 33, 29, 45,},
  {  50,  2, 62, 14, 49,  1, 61, 13,},
  {  26, 42, 22, 38, 25, 41, 21, 37,},
  {  58, 10, 54,  6, 57,  9, 53,  5,},
  {  16, 32, 28, 44, 19, 35, 31, 47,},
  {  48,  0, 60, 12, 51,  3, 63, 15,},
  {  24, 40, 20, 36, 27, 43, 23, 39,},
  {  56,  8, 52,  4, 59, 11, 55,  7,},
},{
  {  36, 68, 60, 92, 34, 66, 58, 90,},
  { 100,  4,124, 28, 98,  2,122, 26,},
  {  52, 84, 44, 76, 50, 82, 42, 74,},
  { 116, 20,108, 12,114, 18,106, 10,},
  {  32, 64, 56, 88, 38, 70, 62, 94,},
  {  96,  0,120, 24,102,  6,126, 30,},
  {  48, 80, 40, 72, 54, 86, 46, 78,},
  { 112, 16,104,  8,118, 22,110, 14,},
}};


static void fillPlane(uint8_t *plane, int stride, int width, int height, int y,
                      uint8_t val)
{
    int i;
    uint8_t *ptr = plane + stride * y;
    for (i = 0; i < height; i++) {
        memset(ptr, val, width);
        ptr += stride;
    }
}

void ff_copyPlane(const uint8_t *src, int srcStride,
                  int srcSliceY, int srcSliceH, int width,
                  uint8_t *dst, int dstStride)
{
    dst += dstStride * srcSliceY;
    if (dstStride == srcStride && srcStride > 0) {
        memcpy(dst, src, srcSliceH * dstStride);
    } else {
        int i;
        for (i = 0; i < srcSliceH; i++) {
            memcpy(dst, src, width);
            src += srcStride;
            dst += dstStride;
        }
    }
}

static int planarToNv12Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY,
                               int srcSliceH, uint8_t *const dstParam[],
                               const int dstStride[])
{
    uint8_t *dst = dstParam[1] + dstStride[1] * srcSliceY / 2;

    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dstParam[0], dstStride[0]);

    if (c->opts.dst_format == AV_PIX_FMT_NV12)
        interleaveBytes(src[1], src[2], dst, c->chrSrcW, (srcSliceH + 1) / 2,
                        srcStride[1], srcStride[2], dstStride[1]);
    else
        interleaveBytes(src[2], src[1], dst, c->chrSrcW, (srcSliceH + 1) / 2,
                        srcStride[2], srcStride[1], dstStride[1]);

    return srcSliceH;
}

static int nv12ToPlanarWrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY,
                               int srcSliceH, uint8_t *const dstParam[],
                               const int dstStride[])
{
    uint8_t *dst1 = dstParam[1] + dstStride[1] * srcSliceY / 2;
    uint8_t *dst2 = dstParam[2] + dstStride[2] * srcSliceY / 2;

    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dstParam[0], dstStride[0]);

    if (c->opts.src_format == AV_PIX_FMT_NV12)
        deinterleaveBytes(src[1], dst1, dst2, c->chrSrcW, (srcSliceH + 1) / 2,
                          srcStride[1], dstStride[1], dstStride[2]);
    else
        deinterleaveBytes(src[1], dst2, dst1, c->chrSrcW, (srcSliceH + 1) / 2,
                          srcStride[1], dstStride[2], dstStride[1]);

    return srcSliceH;
}

static int planarToNv24Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY,
                               int srcSliceH, uint8_t *const dstParam[],
                               const int dstStride[])
{
    uint8_t *dst = dstParam[1] + dstStride[1] * srcSliceY;

    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dstParam[0], dstStride[0]);

    if (c->opts.dst_format == AV_PIX_FMT_NV24)
        interleaveBytes(src[1], src[2], dst, c->chrSrcW, srcSliceH,
                        srcStride[1], srcStride[2], dstStride[1]);
    else
        interleaveBytes(src[2], src[1], dst, c->chrSrcW, srcSliceH,
                        srcStride[2], srcStride[1], dstStride[1]);

    return srcSliceH;
}

static int nv24ToPlanarWrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY,
                               int srcSliceH, uint8_t *const dstParam[],
                               const int dstStride[])
{
    uint8_t *dst1 = dstParam[1] + dstStride[1] * srcSliceY;
    uint8_t *dst2 = dstParam[2] + dstStride[2] * srcSliceY;

    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dstParam[0], dstStride[0]);

    if (c->opts.src_format == AV_PIX_FMT_NV24)
        deinterleaveBytes(src[1], dst1, dst2, c->chrSrcW, srcSliceH,
                          srcStride[1], dstStride[1], dstStride[2]);
    else
        deinterleaveBytes(src[1], dst2, dst1, c->chrSrcW, srcSliceH,
                          srcStride[1], dstStride[2], dstStride[1]);

    return srcSliceH;
}

static void nv24_to_yuv420p_chroma(uint8_t *dst1, int dstStride1,
                                   uint8_t *dst2, int dstStride2,
                                   const uint8_t *src, int srcStride,
                                   int w, int h)
{
    const uint8_t *src1 = src;
    const uint8_t *src2 = src + srcStride;
    // average 4 pixels into 1 (interleaved U and V)
    for (int y = 0; y < h; y += 2) {
        if (y + 1 == h)
            src2 = src1;
        for (int x = 0; x < w; x++) {
            dst1[x] = (src1[4 * x + 0] + src1[4 * x + 2] +
                       src2[4 * x + 0] + src2[4 * x + 2]) >> 2;
            dst2[x] = (src1[4 * x + 1] + src1[4 * x + 3] +
                       src2[4 * x + 1] + src2[4 * x + 3]) >> 2;
        }
        src1 += srcStride * 2;
        src2 += srcStride * 2;
        dst1 += dstStride1;
        dst2 += dstStride2;
    }
}

static int nv24ToYuv420Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *dst1 = dstParam[1] + dstStride[1] * srcSliceY / 2;
    uint8_t *dst2 = dstParam[2] + dstStride[2] * srcSliceY / 2;

    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dstParam[0], dstStride[0]);

    if (c->opts.src_format == AV_PIX_FMT_NV24)
        nv24_to_yuv420p_chroma(dst1, dstStride[1], dst2, dstStride[2],
                               src[1], srcStride[1], c->opts.src_w / 2, srcSliceH);
    else
        nv24_to_yuv420p_chroma(dst2, dstStride[2], dst1, dstStride[1],
                               src[1], srcStride[1], c->opts.src_w / 2, srcSliceH);

    return srcSliceH;
}

static int planarToP01xWrapper(SwsInternal *c, const uint8_t *const src8[],
                               const int srcStride[], int srcSliceY,
                               int srcSliceH, uint8_t *const dstParam8[],
                               const int dstStride[])
{
    const AVPixFmtDescriptor *src_format = av_pix_fmt_desc_get(c->opts.src_format);
    const AVPixFmtDescriptor *dst_format = av_pix_fmt_desc_get(c->opts.dst_format);
    const uint16_t **src = (const uint16_t**)src8;
    uint16_t *dstY = (uint16_t*)(dstParam8[0] + dstStride[0] * srcSliceY);
    uint16_t *dstUV = (uint16_t*)(dstParam8[1] + dstStride[1] * srcSliceY / 2);
    int x, y;

    /* Calculate net shift required for values. */
    const int shift[3] = {
        dst_format->comp[0].depth + dst_format->comp[0].shift -
        src_format->comp[0].depth - src_format->comp[0].shift,
        dst_format->comp[1].depth + dst_format->comp[1].shift -
        src_format->comp[1].depth - src_format->comp[1].shift,
        dst_format->comp[2].depth + dst_format->comp[2].shift -
        src_format->comp[2].depth - src_format->comp[2].shift,
    };

    av_assert0(!(srcStride[0] % 2 || srcStride[1] % 2 || srcStride[2] % 2 ||
                 dstStride[0] % 2 || dstStride[1] % 2));

    for (y = 0; y < srcSliceH; y++) {
        uint16_t *tdstY = dstY;
        const uint16_t *tsrc0 = src[0];
        for (x = c->opts.src_w; x > 0; x--) {
            *tdstY++ = *tsrc0++ << shift[0];
        }
        src[0] += srcStride[0] / 2;
        dstY += dstStride[0] / 2;

        if (!(y & 1)) {
            uint16_t *tdstUV = dstUV;
            const uint16_t *tsrc1 = src[1];
            const uint16_t *tsrc2 = src[2];
            for (x = c->opts.src_w / 2; x > 0; x--) {
                *tdstUV++ = *tsrc1++ << shift[1];
                *tdstUV++ = *tsrc2++ << shift[2];
            }
            src[1] += srcStride[1] / 2;
            src[2] += srcStride[2] / 2;
            dstUV += dstStride[1] / 2;
        }
    }

    return srcSliceH;
}

#if AV_HAVE_BIGENDIAN
#define output_pixel(p, v) do { \
        uint16_t *pp = (p); \
        AV_WL16(pp, (v)); \
    } while(0)
#else
#define output_pixel(p, v) (*p) = (v)
#endif

static int planar8ToP01xleWrapper(SwsInternal *c, const uint8_t *const src[],
                                  const int srcStride[], int srcSliceY,
                                  int srcSliceH, uint8_t *const dstParam8[],
                                  const int dstStride[])
{
    const uint8_t *src0 = src[0], *src1 = src[1], *src2 = src[2];
    uint16_t *dstY = (uint16_t*)(dstParam8[0] + dstStride[0] * srcSliceY);
    uint16_t *dstUV = (uint16_t*)(dstParam8[1] + dstStride[1] * srcSliceY / 2);
    int x, y, t;

    av_assert0(!(dstStride[0] % 2 || dstStride[1] % 2));

    for (y = 0; y < srcSliceH; y++) {
        uint16_t *tdstY = dstY;
        const uint8_t *tsrc0 = src0;
        for (x = c->opts.src_w; x > 0; x--) {
            t = *tsrc0++;
            output_pixel(tdstY++, t << 8);
        }
        src0 += srcStride[0];
        dstY += dstStride[0] / 2;

        if (!(y & 1)) {
            uint16_t *tdstUV = dstUV;
            const uint8_t *tsrc1 = src1;
            const uint8_t *tsrc2 = src2;
            for (x = c->opts.src_w / 2; x > 0; x--) {
                t = *tsrc1++;
                output_pixel(tdstUV++, t << 8);
                t = *tsrc2++;
                output_pixel(tdstUV++, t << 8);
            }
            src1 += srcStride[1];
            src2 += srcStride[2];
            dstUV += dstStride[1] / 2;
        }
    }

    return srcSliceH;
}

#undef output_pixel

static int planarToYuy2Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *dst = dstParam[0] + dstStride[0] * srcSliceY;

    yv12toyuy2(src[0], src[1], src[2], dst, c->opts.src_w, srcSliceH, srcStride[0],
               srcStride[1], dstStride[0]);

    return srcSliceH;
}

static int planarToUyvyWrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *dst = dstParam[0] + dstStride[0] * srcSliceY;

    yv12touyvy(src[0], src[1], src[2], dst, c->opts.src_w, srcSliceH, srcStride[0],
               srcStride[1], dstStride[0]);

    return srcSliceH;
}

static int yuv422pToYuy2Wrapper(SwsInternal *c, const uint8_t *const src[],
                                const int srcStride[], int srcSliceY, int srcSliceH,
                                uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *dst = dstParam[0] + dstStride[0] * srcSliceY;

    yuv422ptoyuy2(src[0], src[1], src[2], dst, c->opts.src_w, srcSliceH, srcStride[0],
                  srcStride[1], dstStride[0]);

    return srcSliceH;
}

static int yuv422pToUyvyWrapper(SwsInternal *c, const uint8_t *const src[],
                                const int srcStride[], int srcSliceY, int srcSliceH,
                                uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *dst = dstParam[0] + dstStride[0] * srcSliceY;

    yuv422ptouyvy(src[0], src[1], src[2], dst, c->opts.src_w, srcSliceH, srcStride[0],
                  srcStride[1], dstStride[0]);

    return srcSliceH;
}

static int yuyvToYuv420Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *ydst = dstParam[0] + dstStride[0] * srcSliceY;
    uint8_t *udst = dstParam[1] + dstStride[1] * srcSliceY / 2;
    uint8_t *vdst = dstParam[2] + dstStride[2] * srcSliceY / 2;

    yuyvtoyuv420(ydst, udst, vdst, src[0], c->opts.src_w, srcSliceH, dstStride[0],
                 dstStride[1], srcStride[0]);

    if (dstParam[3])
        fillPlane(dstParam[3], dstStride[3], c->opts.src_w, srcSliceH, srcSliceY, 255);

    return srcSliceH;
}

static int yuyvToYuv422Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *ydst = dstParam[0] + dstStride[0] * srcSliceY;
    uint8_t *udst = dstParam[1] + dstStride[1] * srcSliceY;
    uint8_t *vdst = dstParam[2] + dstStride[2] * srcSliceY;

    yuyvtoyuv422(ydst, udst, vdst, src[0], c->opts.src_w, srcSliceH, dstStride[0],
                 dstStride[1], srcStride[0]);

    return srcSliceH;
}

static int uyvyToYuv420Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *ydst = dstParam[0] + dstStride[0] * srcSliceY;
    uint8_t *udst = dstParam[1] + dstStride[1] * srcSliceY / 2;
    uint8_t *vdst = dstParam[2] + dstStride[2] * srcSliceY / 2;

    uyvytoyuv420(ydst, udst, vdst, src[0], c->opts.src_w, srcSliceH, dstStride[0],
                 dstStride[1], srcStride[0]);

    if (dstParam[3])
        fillPlane(dstParam[3], dstStride[3], c->opts.src_w, srcSliceH, srcSliceY, 255);

    return srcSliceH;
}

static int uyvyToYuv422Wrapper(SwsInternal *c, const uint8_t *const src[],
                               const int srcStride[], int srcSliceY, int srcSliceH,
                               uint8_t *const dstParam[], const int dstStride[])
{
    uint8_t *ydst = dstParam[0] + dstStride[0] * srcSliceY;
    uint8_t *udst = dstParam[1] + dstStride[1] * srcSliceY;
    uint8_t *vdst = dstParam[2] + dstStride[2] * srcSliceY;

    uyvytoyuv422(ydst, udst, vdst, src[0], c->opts.src_w, srcSliceH, dstStride[0],
                 dstStride[1], srcStride[0]);

    return srcSliceH;
}

static void gray8aToPacked32(const uint8_t *src, uint8_t *dst, int num_pixels,
                             const uint8_t *palette)
{
    int i;
    for (i = 0; i < num_pixels; i++)
        ((uint32_t *) dst)[i] = ((const uint32_t *) palette)[src[i << 1]] | (src[(i << 1) + 1] << 24);
}

static void gray8aToPacked32_1(const uint8_t *src, uint8_t *dst, int num_pixels,
                               const uint8_t *palette)
{
    int i;

    for (i = 0; i < num_pixels; i++)
        ((uint32_t *) dst)[i] = ((const uint32_t *) palette)[src[i << 1]] | src[(i << 1) + 1];
}

static void gray8aToPacked24(const uint8_t *src, uint8_t *dst, int num_pixels,
                             const uint8_t *palette)
{
    int i;

    for (i = 0; i < num_pixels; i++) {
        //FIXME slow?
        dst[0] = palette[src[i << 1] * 4 + 0];
        dst[1] = palette[src[i << 1] * 4 + 1];
        dst[2] = palette[src[i << 1] * 4 + 2];
        dst += 3;
    }
}

static void gray8aToPlanar8(const uint8_t *src, uint8_t *dst0, uint8_t *dst1,
                            uint8_t *dst2, uint8_t *dstA, int num_pixels,
                            const uint8_t *palette)
{
    for (int i = 0; i < num_pixels; i++) {
        const uint8_t *rgb = &palette[src[i << 1] * 4];
        dst0[i] = rgb[0];
        dst1[i] = rgb[1];
        dst2[i] = rgb[2];
        if (dstA)
            dstA[i] = src[(i << 1) + 1];
    }
}

static void pal8ToPlanar8(const uint8_t *src, uint8_t *dst0, uint8_t *dst1,
                          uint8_t *dst2, uint8_t *dstA, int num_pixels,
                          const uint8_t *palette)
{
    for (int i = 0; i < num_pixels; i++) {
        const uint8_t *rgba = &palette[src[i] * 4];
        dst0[i] = rgba[0];
        dst1[i] = rgba[1];
        dst2[i] = rgba[2];
        if (dstA)
            dstA[i] = rgba[3];
    }
}

static int bswap_16bpc(SwsInternal *c, const uint8_t *const src[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[])
{
    int i, j, p;

    for (p = 0; p < 4; p++) {
        int srcstr = srcStride[p] / 2;
        int dststr = dstStride[p] / 2;
        uint16_t       *dstPtr =       (uint16_t *) dst[p];
        const uint16_t *srcPtr = (const uint16_t *) src[p];
        int min_stride         = FFMIN(FFABS(srcstr), FFABS(dststr));
        if(!dstPtr || !srcPtr)
            continue;
        dstPtr += (srcSliceY >> c->chrDstVSubSample) * dststr;
        for (i = 0; i < (srcSliceH >> c->chrDstVSubSample); i++) {
            for (j = 0; j < min_stride; j++) {
                dstPtr[j] = av_bswap16(srcPtr[j]);
            }
            srcPtr += srcstr;
            dstPtr += dststr;
        }
    }

    return srcSliceH;
}

static int bswap_32bpc(SwsInternal *c, const uint8_t *const src[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[])
{
    int i, j, p;

    for (p = 0; p < 4; p++) {
        int srcstr = srcStride[p] / 4;
        int dststr = dstStride[p] / 4;
        uint32_t       *dstPtr =       (uint32_t *) dst[p];
        const uint32_t *srcPtr = (const uint32_t *) src[p];
        int min_stride         = FFMIN(FFABS(srcstr), FFABS(dststr));
        if(!dstPtr || !srcPtr)
            continue;
        dstPtr += (srcSliceY >> c->chrDstVSubSample) * dststr;
        for (i = 0; i < (srcSliceH >> c->chrDstVSubSample); i++) {
            for (j = 0; j < min_stride; j++) {
                dstPtr[j] = av_bswap32(srcPtr[j]);
            }
            srcPtr += srcstr;
            dstPtr += dststr;
        }
    }

    return srcSliceH;
}


static int palToRgbWrapper(SwsInternal *c, const uint8_t *const src[], const int srcStride[],
                           int srcSliceY, int srcSliceH, uint8_t *const dst[],
                           const int dstStride[])
{
    const enum AVPixelFormat srcFormat = c->opts.src_format;
    const enum AVPixelFormat dstFormat = c->opts.dst_format;
    void (*conv)(const uint8_t *src, uint8_t *dst, int num_pixels,
                 const uint8_t *palette) = NULL;
    int i;
    uint8_t *dstPtr = dst[0] + dstStride[0] * srcSliceY;
    const uint8_t *srcPtr = src[0];

    if (srcFormat == AV_PIX_FMT_YA8) {
        switch (dstFormat) {
        case AV_PIX_FMT_RGB32  : conv = gray8aToPacked32; break;
        case AV_PIX_FMT_BGR32  : conv = gray8aToPacked32; break;
        case AV_PIX_FMT_BGR32_1: conv = gray8aToPacked32_1; break;
        case AV_PIX_FMT_RGB32_1: conv = gray8aToPacked32_1; break;
        case AV_PIX_FMT_RGB24  : conv = gray8aToPacked24; break;
        case AV_PIX_FMT_BGR24  : conv = gray8aToPacked24; break;
        }
    } else if (usePal(srcFormat)) {
        switch (dstFormat) {
        case AV_PIX_FMT_RGB32  : conv = sws_convertPalette8ToPacked32; break;
        case AV_PIX_FMT_BGR32  : conv = sws_convertPalette8ToPacked32; break;
        case AV_PIX_FMT_BGR32_1: conv = sws_convertPalette8ToPacked32; break;
        case AV_PIX_FMT_RGB32_1: conv = sws_convertPalette8ToPacked32; break;
        case AV_PIX_FMT_RGB24  : conv = sws_convertPalette8ToPacked24; break;
        case AV_PIX_FMT_BGR24  : conv = sws_convertPalette8ToPacked24; break;
        }
    }

    if (!conv)
        av_log(c, AV_LOG_ERROR, "internal error %s -> %s converter\n",
               av_get_pix_fmt_name(srcFormat), av_get_pix_fmt_name(dstFormat));
    else {
        for (i = 0; i < srcSliceH; i++) {
            conv(srcPtr, dstPtr, c->opts.src_w, (uint8_t *) c->pal_rgb);
            srcPtr += srcStride[0];
            dstPtr += dstStride[0];
        }
    }

    return srcSliceH;
}

static int palToGbrpWrapper(SwsInternal *c, const uint8_t *const src[],
                            const int srcStride[], int srcSliceY, int srcSliceH,
                            uint8_t *const dst[], const int dstStride[])
{
    const enum AVPixelFormat srcFormat = c->opts.src_format;
    const enum AVPixelFormat dstFormat = c->opts.dst_format;
    void (*conv)(const uint8_t *src, uint8_t *dstG, uint8_t *dstB, uint8_t *dstR,
                 uint8_t *dstA, int num_pixels, const uint8_t *palette) = NULL;

    const int num_planes = isALPHA(dstFormat) ? 4 : 3;
    const uint8_t *srcPtr = src[0];
    uint8_t *dstPtr[4] = {0};
    for (int i = 0; i < num_planes; i++)
        dstPtr[i] = dst[i] + dstStride[i] * srcSliceY;

    if (srcFormat == AV_PIX_FMT_YA8) {
        switch (dstFormat) {
        case AV_PIX_FMT_GBRP:  conv = gray8aToPlanar8; break;
        case AV_PIX_FMT_GBRAP: conv = gray8aToPlanar8; break;
        }
    } else if (usePal(srcFormat)) {
        switch (dstFormat) {
        case AV_PIX_FMT_GBRP:  conv = pal8ToPlanar8; break;
        case AV_PIX_FMT_GBRAP: conv = pal8ToPlanar8; break;
        }
    }

    av_assert1(conv);
    for (int y = 0; y < srcSliceH; y++) {
        conv(srcPtr, dstPtr[0], dstPtr[1], dstPtr[2], dstPtr[3], c->opts.src_w,
            (uint8_t *) c->pal_rgb);
        srcPtr += srcStride[0];
        for (int i = 0; i < num_planes; i++)
            dstPtr[i] += dstStride[i];
    }

    return srcSliceH;
}

static void packed16togbra16(const uint8_t *src, int srcStride,
                             uint16_t *dst[], const int dstStride[], int srcSliceH,
                             int src_alpha, int swap, int shift, int width)
{
    int x, h, i;
    int dst_alpha = dst[3] != NULL;
    for (h = 0; h < srcSliceH; h++) {
        uint16_t *src_line = (uint16_t *)(src + srcStride * h);
        switch (swap) {
        case 3:
            if (src_alpha && dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[1][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[2][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[3][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                }
            } else if (dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[1][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[2][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[3][x] = av_bswap16(0xFFFF >> shift);
                }
            } else if (src_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[1][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[2][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    src_line++;
                }
            } else {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[1][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                    dst[2][x] = av_bswap16(av_bswap16(*src_line++) >> shift);
                }
            }
            break;
        case 2:
            if (src_alpha && dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++ >> shift);
                    dst[1][x] = av_bswap16(*src_line++ >> shift);
                    dst[2][x] = av_bswap16(*src_line++ >> shift);
                    dst[3][x] = av_bswap16(*src_line++ >> shift);
                }
            } else if (dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++ >> shift);
                    dst[1][x] = av_bswap16(*src_line++ >> shift);
                    dst[2][x] = av_bswap16(*src_line++ >> shift);
                    dst[3][x] = av_bswap16(0xFFFF >> shift);
                }
            } else if (src_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++ >> shift);
                    dst[1][x] = av_bswap16(*src_line++ >> shift);
                    dst[2][x] = av_bswap16(*src_line++ >> shift);
                    src_line++;
                }
            } else {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++ >> shift);
                    dst[1][x] = av_bswap16(*src_line++ >> shift);
                    dst[2][x] = av_bswap16(*src_line++ >> shift);
                }
            }
            break;
        case 1:
            if (src_alpha && dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++) >> shift;
                    dst[1][x] = av_bswap16(*src_line++) >> shift;
                    dst[2][x] = av_bswap16(*src_line++) >> shift;
                    dst[3][x] = av_bswap16(*src_line++) >> shift;
                }
            } else if (dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++) >> shift;
                    dst[1][x] = av_bswap16(*src_line++) >> shift;
                    dst[2][x] = av_bswap16(*src_line++) >> shift;
                    dst[3][x] = 0xFFFF >> shift;
                }
            } else if (src_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++) >> shift;
                    dst[1][x] = av_bswap16(*src_line++) >> shift;
                    dst[2][x] = av_bswap16(*src_line++) >> shift;
                    src_line++;
                }
            } else {
                for (x = 0; x < width; x++) {
                    dst[0][x] = av_bswap16(*src_line++) >> shift;
                    dst[1][x] = av_bswap16(*src_line++) >> shift;
                    dst[2][x] = av_bswap16(*src_line++) >> shift;
                }
            }
            break;
        default:
            if (src_alpha && dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = *src_line++ >> shift;
                    dst[1][x] = *src_line++ >> shift;
                    dst[2][x] = *src_line++ >> shift;
                    dst[3][x] = *src_line++ >> shift;
                }
            } else if (dst_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = *src_line++ >> shift;
                    dst[1][x] = *src_line++ >> shift;
                    dst[2][x] = *src_line++ >> shift;
                    dst[3][x] = 0xFFFF >> shift;
                }
            } else if (src_alpha) {
                for (x = 0; x < width; x++) {
                    dst[0][x] = *src_line++ >> shift;
                    dst[1][x] = *src_line++ >> shift;
                    dst[2][x] = *src_line++ >> shift;
                    src_line++;
                }
            } else {
                for (x = 0; x < width; x++) {
                    dst[0][x] = *src_line++ >> shift;
                    dst[1][x] = *src_line++ >> shift;
                    dst[2][x] = *src_line++ >> shift;
                }
            }
        }
        for (i = 0; i < 4 && dst[i]; i++)
            dst[i] += dstStride[i] >> 1;
    }
}

static void packed30togbra10(const uint8_t *src, int srcStride,
                             uint16_t *dst[], const int dstStride[], int srcSliceH,
                             int swap, int bpc, int width)
{
    int x, h, i;
    int dst_alpha = dst[3] != NULL;
    int scale_high = bpc - 10, scale_low = 10 - scale_high;
    uint16_t alpha_val = (1U << bpc) - 1;
    for (h = 0; h < srcSliceH; h++) {
        uint32_t *src_line = (uint32_t *)(src + srcStride * h);
        unsigned component;

        switch (swap) {
        case 3:
        case 2:
            if (dst_alpha) {
                for (x = 0; x < width; x++) {
                    unsigned p = AV_RL32(src_line);
                    component = (p >> 20) & 0x3FF;
                    dst[0][x] = av_bswap16(component << scale_high | component >> scale_low);
                    component = (p >> 10) & 0x3FF;
                    dst[1][x] = av_bswap16(component << scale_high | component >> scale_low);
                    component =  p        & 0x3FF;
                    dst[2][x] = av_bswap16(component << scale_high | component >> scale_low);
                    dst[3][x] = av_bswap16(alpha_val);
                    src_line++;
                }
            } else {
                for (x = 0; x < width; x++) {
                    unsigned p = AV_RL32(src_line);
                    component = (p >> 20) & 0x3FF;
                    dst[0][x] = av_bswap16(component << scale_high | component >> scale_low);
                    component = (p >> 10) & 0x3FF;
                    dst[1][x] = av_bswap16(component << scale_high | component >> scale_low);
                    component =  p        & 0x3FF;
                    dst[2][x] = av_bswap16(component << scale_high | component >> scale_low);
                    src_line++;
                }
            }
            break;
        default:
            if (dst_alpha) {
                for (x = 0; x < width; x++) {
                    unsigned p = AV_RL32(src_line);
                    component = (p >> 20) & 0x3FF;
                    dst[0][x] = component << scale_high | component >> scale_low;
                    component = (p >> 10) & 0x3FF;
                    dst[1][x] = component << scale_high | component >> scale_low;
                    component =  p        & 0x3FF;
                    dst[2][x] = component << scale_high | component >> scale_low;
                    dst[3][x] = alpha_val;
                    src_line++;
                }
            } else {
                for (x = 0; x < width; x++) {
                    unsigned p = AV_RL32(src_line);
                    component = (p >> 20) & 0x3FF;
                    dst[0][x] = component << scale_high | component >> scale_low;
                    component = (p >> 10) & 0x3FF;
                    dst[1][x] = component << scale_high | component >> scale_low;
                    component =  p        & 0x3FF;
                    dst[2][x] = component << scale_high | component >> scale_low;
                    src_line++;
                }
            }
            break;
        }
        for (i = 0; i < 4 && dst[i]; i++)
            dst[i] += dstStride[i] >> 1;
    }
}

static int Rgb16ToPlanarRgb16Wrapper(SwsInternal *c, const uint8_t *const src[],
                                     const int srcStride[], int srcSliceY, int srcSliceH,
                                     uint8_t *const dst[], const int dstStride[])
{
    uint16_t *dst2013[] = { (uint16_t *)dst[2], (uint16_t *)dst[0], (uint16_t *)dst[1], (uint16_t *)dst[3] };
    uint16_t *dst1023[] = { (uint16_t *)dst[1], (uint16_t *)dst[0], (uint16_t *)dst[2], (uint16_t *)dst[3] };
    int stride2013[] = { dstStride[2], dstStride[0], dstStride[1], dstStride[3] };
    int stride1023[] = { dstStride[1], dstStride[0], dstStride[2], dstStride[3] };
    const AVPixFmtDescriptor *src_format = av_pix_fmt_desc_get(c->opts.src_format);
    const AVPixFmtDescriptor *dst_format = av_pix_fmt_desc_get(c->opts.dst_format);
    int bpc = dst_format->comp[0].depth;
    int alpha = src_format->flags & AV_PIX_FMT_FLAG_ALPHA;
    int swap = 0;
    int i;

    if ( HAVE_BIGENDIAN && !(src_format->flags & AV_PIX_FMT_FLAG_BE) ||
        !HAVE_BIGENDIAN &&   src_format->flags & AV_PIX_FMT_FLAG_BE)
        swap++;
    if ( HAVE_BIGENDIAN && !(dst_format->flags & AV_PIX_FMT_FLAG_BE) ||
        !HAVE_BIGENDIAN &&   dst_format->flags & AV_PIX_FMT_FLAG_BE)
        swap += 2;

    if ((dst_format->flags & (AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB)) !=
        (AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB) || bpc < 9) {
        av_log(c, AV_LOG_ERROR, "unsupported conversion to planar RGB %s -> %s\n",
               src_format->name, dst_format->name);
        return srcSliceH;
    }

    for (i = 0; i < 4 && dst[i]; i++) {
        dst2013[i] += stride2013[i] * srcSliceY / 2;
        dst1023[i] += stride1023[i] * srcSliceY / 2;
    }

    switch (c->opts.src_format) {
    case AV_PIX_FMT_RGB48LE:
    case AV_PIX_FMT_RGB48BE:
    case AV_PIX_FMT_RGBA64LE:
    case AV_PIX_FMT_RGBA64BE:
        packed16togbra16(src[0], srcStride[0],
                         dst2013, stride2013, srcSliceH, alpha, swap,
                         16 - bpc, c->opts.src_w);
        break;
    case AV_PIX_FMT_X2RGB10LE:
        av_assert0(bpc >= 10);
        packed30togbra10(src[0], srcStride[0],
                         dst2013, stride2013, srcSliceH, swap,
                         bpc, c->opts.src_w);
        break;
    case AV_PIX_FMT_BGR48LE:
    case AV_PIX_FMT_BGR48BE:
    case AV_PIX_FMT_BGRA64LE:
    case AV_PIX_FMT_BGRA64BE:
        packed16togbra16(src[0], srcStride[0],
                         dst1023, stride1023, srcSliceH, alpha, swap,
                         16 - bpc, c->opts.src_w);
        break;
    case AV_PIX_FMT_X2BGR10LE:
        av_assert0(bpc >= 10);
        packed30togbra10(src[0], srcStride[0],
                         dst1023, stride1023, srcSliceH, swap,
                         bpc, c->opts.src_w);
        break;
    default:
        av_log(c, AV_LOG_ERROR,
               "unsupported conversion to planar RGB %s -> %s\n",
               src_format->name, dst_format->name);
    }

    return srcSliceH;
}

static void gbr16ptopacked16(const uint16_t *src[], const int srcStride[],
                             uint8_t *dst, int dstStride, int srcSliceH,
                             int alpha, int swap, int bpp, int width)
{
    int x, h, i;
    int src_alpha = src[3] != NULL;
    int scale_high = 16 - bpp, scale_low = (bpp - 8) * 2;
    for (h = 0; h < srcSliceH; h++) {
        uint16_t *dest = (uint16_t *)(dst + dstStride * h);
        uint16_t component;

        switch(swap) {
        case 3:
            if (alpha && !src_alpha) {
                for (x = 0; x < width; x++) {
                    component = av_bswap16(src[0][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[1][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[2][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    *dest++ = 0xffff;
                }
            } else if (alpha && src_alpha) {
                for (x = 0; x < width; x++) {
                    component = av_bswap16(src[0][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[1][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[2][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[3][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                }
            } else {
                for (x = 0; x < width; x++) {
                    component = av_bswap16(src[0][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[1][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                    component = av_bswap16(src[2][x]);
                    *dest++ = av_bswap16(component << scale_high | component >> scale_low);
                }
            }
            break;
        case 2:
            if (alpha && !src_alpha) {
                for (x = 0; x < width; x++) {
                    *dest++ = av_bswap16(src[0][x] << scale_high | src[0][x] >> scale_low);
                    *dest++ = av_bswap16(src[1][x] << scale_high | src[1][x] >> scale_low);
                    *dest++ = av_bswap16(src[2][x] << scale_high | src[2][x] >> scale_low);
                    *dest++ = 0xffff;
                }
            } else if (alpha && src_alpha) {
                for (x = 0; x < width; x++) {
                    *dest++ = av_bswap16(src[0][x] << scale_high | src[0][x] >> scale_low);
                    *dest++ = av_bswap16(src[1][x] << scale_high | src[1][x] >> scale_low);
                    *dest++ = av_bswap16(src[2][x] << scale_high | src[2][x] >> scale_low);
                    *dest++ = av_bswap16(src[3][x] << scale_high | src[3][x] >> scale_low);
                }
            } else {
                for (x = 0; x < width; x++) {
                    *dest++ = av_bswap16(src[0][x] << scale_high | src[0][x] >> scale_low);
                    *dest++ = av_bswap16(src[1][x] << scale_high | src[1][x] >> scale_low);
                    *dest++ = av_bswap16(src[2][x] << scale_high | src[2][x] >> scale_low);
                }
            }
            break;
        case 1:
            if (alpha && !src_alpha) {
                for (x = 0; x < width; x++) {
                    *dest++ = av_bswap16(src[0][x]) << scale_high | av_bswap16(src[0][x]) >> scale_low;
                    *dest++ = av_bswap16(src[1][x]) << scale_high | av_bswap16(src[1][x]) >> scale_low;
                    *dest++ = av_bswap16(src[2][x]) << scale_high | av_bswap16(src[2][x]) >> scale_low;
                    *dest++ = 0xffff;
                }
            } else if (alpha && src_alpha) {
                for (x = 0; x < width; x++) {
                    *dest++ = av_bswap16(src[0][x]) << scale_high | av_bswap16(src[0][x]) >> scale_low;
                    *dest++ = av_bswap16(src[1][x]) << scale_high | av_bswap16(src[1][x]) >> scale_low;
                    *dest++ = av_bswap16(src[2][x]) << scale_high | av_bswap16(src[2][x]) >> scale_low;
                    *dest++ = av_bswap16(src[3][x]) << scale_high | av_bswap16(src[3][x]) >> scale_low;
                }
            } else {
                for (x = 0; x < width; x++) {
                    *dest++ = av_bswap16(src[0][x]) << scale_high | av_bswap16(src[0][x]) >> scale_low;
                    *dest++ = av_bswap16(src[1][x]) << scale_high | av_bswap16(src[1][x]) >> scale_low;
                    *dest++ = av_bswap16(src[2][x]) << scale_high | av_bswap16(src[2][x]) >> scale_low;
                }
            }
            break;
        default:
            if (alpha && !src_alpha) {
                for (x = 0; x < width; x++) {
                    *dest++ = src[0][x] << scale_high | src[0][x] >> scale_low;
                    *dest++ = src[1][x] << scale_high | src[1][x] >> scale_low;
                    *dest++ = src[2][x] << scale_high | src[2][x] >> scale_low;
                    *dest++ = 0xffff;
                }
            } else if (alpha && src_alpha) {
                for (x = 0; x < width; x++) {
                    *dest++ = src[0][x] << scale_high | src[0][x] >> scale_low;
                    *dest++ = src[1][x] << scale_high | src[1][x] >> scale_low;
                    *dest++ = src[2][x] << scale_high | src[2][x] >> scale_low;
                    *dest++ = src[3][x] << scale_high | src[3][x] >> scale_low;
                }
            } else {
                for (x = 0; x < width; x++) {
                    *dest++ = src[0][x] << scale_high | src[0][x] >> scale_low;
                    *dest++ = src[1][x] << scale_high | src[1][x] >> scale_low;
                    *dest++ = src[2][x] << scale_high | src[2][x] >> scale_low;
                }
            }
        }
        for (i = 0; i < 3 + src_alpha; i++)
            src[i] += srcStride[i] >> 1;
    }
}

static void gbr16ptopacked30(const uint16_t *src[], const int srcStride[],
                             uint8_t *dst, int dstStride, int srcSliceH,
                             int swap, int bpp, int width)
{
    int x, h, i;
    int shift = bpp - 10;
    av_assert0(bpp >= 0);
    for (h = 0; h < srcSliceH; h++) {
        uint8_t *dest = dst + dstStride * h;

        switch(swap) {
        case 3:
        case 1:
            for (x = 0; x < width; x++) {
                unsigned C0 = av_bswap16(src[0][x]) >> shift;
                unsigned C1 = av_bswap16(src[1][x]) >> shift;
                unsigned C2 = av_bswap16(src[2][x]) >> shift;
                AV_WL32(dest + 4 * x, (3U << 30) + (C0 << 20) + (C1 << 10) + C2);
            }
            break;
        default:
            for (x = 0; x < width; x++) {
                unsigned C0 = src[0][x] >> shift;
                unsigned C1 = src[1][x] >> shift;
                unsigned C2 = src[2][x] >> shift;
                AV_WL32(dest + 4 * x, (3U << 30) + (C0 << 20) + (C1 << 10) + C2);
            }
            break;
        }
        for (i = 0; i < 3; i++)
            src[i] += srcStride[i] >> 1;
    }
}


static int planarRgb16ToRgb16Wrapper(SwsInternal *c, const uint8_t *const src[],
                                     const int srcStride[], int srcSliceY, int srcSliceH,
                                     uint8_t *const dst[], const int dstStride[])
{
    const uint16_t *src102[] = { (uint16_t *)src[1], (uint16_t *)src[0], (uint16_t *)src[2], (uint16_t *)src[3] };
    const uint16_t *src201[] = { (uint16_t *)src[2], (uint16_t *)src[0], (uint16_t *)src[1], (uint16_t *)src[3] };
    int stride102[] = { srcStride[1], srcStride[0], srcStride[2], srcStride[3] };
    int stride201[] = { srcStride[2], srcStride[0], srcStride[1], srcStride[3] };
    const AVPixFmtDescriptor *src_format = av_pix_fmt_desc_get(c->opts.src_format);
    const AVPixFmtDescriptor *dst_format = av_pix_fmt_desc_get(c->opts.dst_format);
    int bits_per_sample = src_format->comp[0].depth;
    int swap = 0;
    if ( HAVE_BIGENDIAN && !(src_format->flags & AV_PIX_FMT_FLAG_BE) ||
        !HAVE_BIGENDIAN &&   src_format->flags & AV_PIX_FMT_FLAG_BE)
        swap++;
    if ( HAVE_BIGENDIAN && !(dst_format->flags & AV_PIX_FMT_FLAG_BE) ||
        !HAVE_BIGENDIAN &&   dst_format->flags & AV_PIX_FMT_FLAG_BE)
        swap += 2;

    if ((src_format->flags & (AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB)) !=
        (AV_PIX_FMT_FLAG_PLANAR | AV_PIX_FMT_FLAG_RGB) ||
        bits_per_sample <= 8) {
        av_log(c, AV_LOG_ERROR, "unsupported planar RGB conversion %s -> %s\n",
               src_format->name, dst_format->name);
        return srcSliceH;
    }
    switch (c->opts.dst_format) {
    case AV_PIX_FMT_BGR48LE:
    case AV_PIX_FMT_BGR48BE:
        gbr16ptopacked16(src102, stride102,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, 0, swap, bits_per_sample, c->opts.src_w);
        break;
    case AV_PIX_FMT_RGB48LE:
    case AV_PIX_FMT_RGB48BE:
        gbr16ptopacked16(src201, stride201,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, 0, swap, bits_per_sample, c->opts.src_w);
        break;
    case AV_PIX_FMT_RGBA64LE:
    case AV_PIX_FMT_RGBA64BE:
         gbr16ptopacked16(src201, stride201,
                          dst[0] + srcSliceY * dstStride[0], dstStride[0],
                          srcSliceH, 1, swap, bits_per_sample, c->opts.src_w);
        break;
    case AV_PIX_FMT_BGRA64LE:
    case AV_PIX_FMT_BGRA64BE:
        gbr16ptopacked16(src102, stride102,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, 1, swap, bits_per_sample, c->opts.src_w);
        break;
    case AV_PIX_FMT_X2RGB10LE:
        gbr16ptopacked30(src201, stride201,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, swap, bits_per_sample, c->opts.src_w);
        break;
    case AV_PIX_FMT_X2BGR10LE:
        gbr16ptopacked30(src102, stride102,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, swap, bits_per_sample, c->opts.src_w);
        break;
    default:
        av_log(c, AV_LOG_ERROR,
               "unsupported planar RGB conversion %s -> %s\n",
               src_format->name, dst_format->name);
    }

    return srcSliceH;
}

static void gbr24ptopacked24(const uint8_t *src[], const int srcStride[],
                             uint8_t *dst, int dstStride, int srcSliceH,
                             int width)
{
    int x, h, i;
    for (h = 0; h < srcSliceH; h++) {
        uint8_t *dest = dst + dstStride * h;
        for (x = 0; x < width; x++) {
            *dest++ = src[0][x];
            *dest++ = src[1][x];
            *dest++ = src[2][x];
        }

        for (i = 0; i < 3; i++)
            src[i] += srcStride[i];
    }
}

static void gbr24ptopacked32(const uint8_t *src[], const int srcStride[],
                             uint8_t *dst, int dstStride, int srcSliceH,
                             int alpha_first, int width)
{
    int x, h, i;
    for (h = 0; h < srcSliceH; h++) {
        uint8_t *dest = dst + dstStride * h;

        if (alpha_first) {
            for (x = 0; x < width; x++) {
                *dest++ = 0xff;
                *dest++ = src[0][x];
                *dest++ = src[1][x];
                *dest++ = src[2][x];
            }
        } else {
            for (x = 0; x < width; x++) {
                *dest++ = src[0][x];
                *dest++ = src[1][x];
                *dest++ = src[2][x];
                *dest++ = 0xff;
            }
        }

        for (i = 0; i < 3; i++)
            src[i] += srcStride[i];
    }
}

static void gbraptopacked32(const uint8_t *src[], const int srcStride[],
                            uint8_t *dst, int dstStride, int srcSliceH,
                            int alpha_first, int width)
{
    int x, h, i;
    for (h = 0; h < srcSliceH; h++) {
        uint8_t *dest = dst + dstStride * h;

        if (alpha_first) {
            for (x = 0; x < width; x++) {
                *dest++ = src[3][x];
                *dest++ = src[0][x];
                *dest++ = src[1][x];
                *dest++ = src[2][x];
            }
        } else {
            for (x = 0; x < width; x++) {
                *dest++ = src[0][x];
                *dest++ = src[1][x];
                *dest++ = src[2][x];
                *dest++ = src[3][x];
            }
        }

        for (i = 0; i < 4; i++)
            src[i] += srcStride[i];
    }
}

static int planarRgbaToRgbWrapper(SwsInternal *c, const uint8_t *const src[],
                                  const int srcStride[], int srcSliceY, int srcSliceH,
                                  uint8_t *const dst[], const int dstStride[])
{
    int alpha_first = 0;
    const uint8_t *src102[] = { src[1], src[0], src[2], src[3] };
    const uint8_t *src201[] = { src[2], src[0], src[1], src[3] };
    int stride102[] = { srcStride[1], srcStride[0], srcStride[2], srcStride[3] };
    int stride201[] = { srcStride[2], srcStride[0], srcStride[1], srcStride[3] };

    if (c->opts.src_format != AV_PIX_FMT_GBRAP) {
        av_log(c, AV_LOG_ERROR, "unsupported planar RGB conversion %s -> %s\n",
               av_get_pix_fmt_name(c->opts.src_format),
               av_get_pix_fmt_name(c->opts.dst_format));
        return srcSliceH;
    }

    switch (c->opts.dst_format) {
    case AV_PIX_FMT_BGR24:
        gbr24ptopacked24(src102, stride102,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, c->opts.src_w);
        break;

    case AV_PIX_FMT_RGB24:
        gbr24ptopacked24(src201, stride201,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, c->opts.src_w);
        break;

    case AV_PIX_FMT_ARGB:
        alpha_first = 1;
    case AV_PIX_FMT_RGBA:
        gbraptopacked32(src201, stride201,
                        dst[0] + srcSliceY * dstStride[0], dstStride[0],
                        srcSliceH, alpha_first, c->opts.src_w);
        break;

    case AV_PIX_FMT_ABGR:
        alpha_first = 1;
    case AV_PIX_FMT_BGRA:
        gbraptopacked32(src102, stride102,
                        dst[0] + srcSliceY * dstStride[0], dstStride[0],
                        srcSliceH, alpha_first, c->opts.src_w);
        break;

    default:
        av_log(c, AV_LOG_ERROR,
               "unsupported planar RGB conversion %s -> %s\n",
               av_get_pix_fmt_name(c->opts.src_format),
               av_get_pix_fmt_name(c->opts.dst_format));
    }

    return srcSliceH;
}

static int planarRgbToRgbWrapper(SwsInternal *c, const uint8_t *const src[],
                                 const int srcStride[], int srcSliceY, int srcSliceH,
                                 uint8_t *const dst[], const int dstStride[])
{
    int alpha_first = 0;
    const uint8_t *src102[] = { src[1], src[0], src[2] };
    const uint8_t *src201[] = { src[2], src[0], src[1] };
    int stride102[] = { srcStride[1], srcStride[0], srcStride[2] };
    int stride201[] = { srcStride[2], srcStride[0], srcStride[1] };

    if (c->opts.src_format != AV_PIX_FMT_GBRP) {
        av_log(c, AV_LOG_ERROR, "unsupported planar RGB conversion %s -> %s\n",
               av_get_pix_fmt_name(c->opts.src_format),
               av_get_pix_fmt_name(c->opts.dst_format));
        return srcSliceH;
    }

    switch (c->opts.dst_format) {
    case AV_PIX_FMT_BGR24:
        gbr24ptopacked24(src102, stride102,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, c->opts.src_w);
        break;

    case AV_PIX_FMT_RGB24:
        gbr24ptopacked24(src201, stride201,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, c->opts.src_w);
        break;

    case AV_PIX_FMT_ARGB:
        alpha_first = 1;
    case AV_PIX_FMT_RGBA:
        gbr24ptopacked32(src201, stride201,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, alpha_first, c->opts.src_w);
        break;

    case AV_PIX_FMT_ABGR:
        alpha_first = 1;
    case AV_PIX_FMT_BGRA:
        gbr24ptopacked32(src102, stride102,
                         dst[0] + srcSliceY * dstStride[0], dstStride[0],
                         srcSliceH, alpha_first, c->opts.src_w);
        break;

    default:
        av_log(c, AV_LOG_ERROR,
               "unsupported planar RGB conversion %s -> %s\n",
               av_get_pix_fmt_name(c->opts.src_format),
               av_get_pix_fmt_name(c->opts.dst_format));
    }

    return srcSliceH;
}

static int planarRgbToplanarRgbWrapper(SwsInternal *c,
                                       const uint8_t *const src[], const int srcStride[],
                                       int srcSliceY, int srcSliceH,
                                       uint8_t *const dst[], const int dstStride[])
{
    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dst[0], dstStride[0]);
    ff_copyPlane(src[1], srcStride[1], srcSliceY, srcSliceH, c->opts.src_w,
                 dst[1], dstStride[1]);
    ff_copyPlane(src[2], srcStride[2], srcSliceY, srcSliceH, c->opts.src_w,
                 dst[2], dstStride[2]);
    if (dst[3]) {
        if (is16BPS(c->opts.dst_format) || isNBPS(c->opts.dst_format)) {
            const AVPixFmtDescriptor *desc_dst = av_pix_fmt_desc_get(c->opts.dst_format);
            fillPlane16(dst[3], dstStride[3], c->opts.src_w, srcSliceH, srcSliceY, 1,
                        desc_dst->comp[3].depth, isBE(c->opts.dst_format));
        } else {
            fillPlane(dst[3], dstStride[3], c->opts.src_w, srcSliceH, srcSliceY, 255);
        }
    }

    return srcSliceH;
}

static void packedtogbr24p(const uint8_t *src, int srcStride,
                           uint8_t *const dst[], const int dstStride[], int srcSliceH,
                           int alpha_first, int inc_size, int width)
{
    uint8_t *dest[3];
    int x, h;

    dest[0] = dst[0];
    dest[1] = dst[1];
    dest[2] = dst[2];

    if (alpha_first)
        src++;

    for (h = 0; h < srcSliceH; h++) {
        for (x = 0; x < width; x++) {
            dest[0][x] = src[0];
            dest[1][x] = src[1];
            dest[2][x] = src[2];

            src += inc_size;
        }
        src     += srcStride - width * inc_size;
        dest[0] += dstStride[0];
        dest[1] += dstStride[1];
        dest[2] += dstStride[2];
    }
}

static int rgbToPlanarRgbWrapper(SwsInternal *c, const uint8_t *const src[],
                                 const int srcStride[], int srcSliceY, int srcSliceH,
                                 uint8_t *const dst[], const int dstStride[])
{
    int alpha_first = 0;
    int stride102[] = { dstStride[1], dstStride[0], dstStride[2] };
    int stride201[] = { dstStride[2], dstStride[0], dstStride[1] };
    uint8_t *dst102[] = { dst[1] + srcSliceY * dstStride[1],
                          dst[0] + srcSliceY * dstStride[0],
                          dst[2] + srcSliceY * dstStride[2] };
    uint8_t *dst201[] = { dst[2] + srcSliceY * dstStride[2],
                          dst[0] + srcSliceY * dstStride[0],
                          dst[1] + srcSliceY * dstStride[1] };

    switch (c->opts.src_format) {
    case AV_PIX_FMT_RGB24:
        packedtogbr24p((const uint8_t *) src[0], srcStride[0], dst201,
                       stride201, srcSliceH, alpha_first, 3, c->opts.src_w);
        break;
    case AV_PIX_FMT_BGR24:
        packedtogbr24p((const uint8_t *) src[0], srcStride[0], dst102,
                       stride102, srcSliceH, alpha_first, 3, c->opts.src_w);
        break;
    case AV_PIX_FMT_ARGB:
        alpha_first = 1;
    case AV_PIX_FMT_RGBA:
        packedtogbr24p((const uint8_t *) src[0], srcStride[0], dst201,
                       stride201, srcSliceH, alpha_first, 4, c->opts.src_w);
        break;
    case AV_PIX_FMT_ABGR:
        alpha_first = 1;
    case AV_PIX_FMT_BGRA:
        packedtogbr24p((const uint8_t *) src[0], srcStride[0], dst102,
                       stride102, srcSliceH, alpha_first, 4, c->opts.src_w);
        break;
    default:
        av_log(c, AV_LOG_ERROR,
               "unsupported planar RGB conversion %s -> %s\n",
               av_get_pix_fmt_name(c->opts.src_format),
               av_get_pix_fmt_name(c->opts.dst_format));
    }

    return srcSliceH;
}

static void packed24togbrap(const uint8_t *src, int srcStride,
                            uint8_t *const dst[], const int dstStride[],
                            int srcSliceH, int width)
{
    uint8_t *dest[4];
    int x, h;

    dest[0] = dst[0];
    dest[1] = dst[1];
    dest[2] = dst[2];
    dest[3] = dst[3];

    for (h = 0; h < srcSliceH; h++) {
        for (x = 0; x < width; x++) {
            dest[0][x] = src[x * 3 + 0];
            dest[1][x] = src[x * 3 + 1];
            dest[2][x] = src[x * 3 + 2];
            dest[3][x] = 0xff;
        }
        src     += srcStride;
        dest[0] += dstStride[0];
        dest[1] += dstStride[1];
        dest[2] += dstStride[2];
        dest[3] += dstStride[3];
    }
}

static void packed32togbrap(const uint8_t *src, int srcStride,
                            uint8_t *const dst[], const int dstStride[],
                            int srcSliceH, int alpha_first, int width)
{
    uint8_t *dest[4];
    int x, h;

    dest[0] = dst[0];
    dest[1] = dst[1];
    dest[2] = dst[2];
    dest[3] = dst[3];

    for (h = 0; h < srcSliceH; h++) {
        if (alpha_first) {
            for (x = 0; x < width; x++) {
                dest[0][x] = src[x * 4 + 1];
                dest[1][x] = src[x * 4 + 2];
                dest[2][x] = src[x * 4 + 3];
                dest[3][x] = src[x * 4 + 0];
            }
        } else {
            for (x = 0; x < width; x++) {
                dest[0][x] = src[x * 4 + 0];
                dest[1][x] = src[x * 4 + 1];
                dest[2][x] = src[x * 4 + 2];
                dest[3][x] = src[x * 4 + 3];
            }
        }
        src     += srcStride;
        dest[0] += dstStride[0];
        dest[1] += dstStride[1];
        dest[2] += dstStride[2];
        dest[3] += dstStride[3];
    }
}

static int rgbToPlanarRgbaWrapper(SwsInternal *c, const uint8_t *const src[],
                                  const int srcStride[], int srcSliceY, int srcSliceH,
                                  uint8_t *const dst[], const int dstStride[])
{
    int alpha_first = 0;
    int stride102[] = { dstStride[1], dstStride[0], dstStride[2], dstStride[3] };
    int stride201[] = { dstStride[2], dstStride[0], dstStride[1], dstStride[3] };
    uint8_t *dst102[] = { dst[1] + srcSliceY * dstStride[1],
                          dst[0] + srcSliceY * dstStride[0],
                          dst[2] + srcSliceY * dstStride[2],
                          dst[3] + srcSliceY * dstStride[3] };
    uint8_t *dst201[] = { dst[2] + srcSliceY * dstStride[2],
                          dst[0] + srcSliceY * dstStride[0],
                          dst[1] + srcSliceY * dstStride[1],
                          dst[3] + srcSliceY * dstStride[3] };

    switch (c->opts.src_format) {
    case AV_PIX_FMT_RGB24:
        packed24togbrap((const uint8_t *) src[0], srcStride[0], dst201,
                        stride201, srcSliceH, c->opts.src_w);
        break;
    case AV_PIX_FMT_BGR24:
        packed24togbrap((const uint8_t *) src[0], srcStride[0], dst102,
                        stride102, srcSliceH, c->opts.src_w);
        break;
    case AV_PIX_FMT_ARGB:
        alpha_first = 1;
    case AV_PIX_FMT_RGBA:
        packed32togbrap((const uint8_t *) src[0], srcStride[0], dst201,
                        stride201, srcSliceH, alpha_first, c->opts.src_w);
        break;
    case AV_PIX_FMT_ABGR:
        alpha_first = 1;
    case AV_PIX_FMT_BGRA:
        packed32togbrap((const uint8_t *) src[0], srcStride[0], dst102,
                        stride102, srcSliceH, alpha_first, c->opts.src_w);
        break;
    default:
        av_log(c, AV_LOG_ERROR,
               "unsupported planar RGB conversion %s -> %s\n",
               av_get_pix_fmt_name(c->opts.src_format),
               av_get_pix_fmt_name(c->opts.dst_format));
    }

    return srcSliceH;
}

#define BAYER_GBRG
#define BAYER_8
#define BAYER_RENAME(x) bayer_gbrg8_to_##x
#include "bayer_template.c"

#define BAYER_GBRG
#define BAYER_16LE
#define BAYER_RENAME(x) bayer_gbrg16le_to_##x
#include "bayer_template.c"

#define BAYER_GBRG
#define BAYER_16BE
#define BAYER_RENAME(x) bayer_gbrg16be_to_##x
#include "bayer_template.c"

#define BAYER_GRBG
#define BAYER_8
#define BAYER_RENAME(x) bayer_grbg8_to_##x
#include "bayer_template.c"

#define BAYER_GRBG
#define BAYER_16LE
#define BAYER_RENAME(x) bayer_grbg16le_to_##x
#include "bayer_template.c"

#define BAYER_GRBG
#define BAYER_16BE
#define BAYER_RENAME(x) bayer_grbg16be_to_##x
#include "bayer_template.c"

#define BAYER_BGGR
#define BAYER_8
#define BAYER_RENAME(x) bayer_bggr8_to_##x
#include "bayer_template.c"

#define BAYER_BGGR
#define BAYER_16LE
#define BAYER_RENAME(x) bayer_bggr16le_to_##x
#include "bayer_template.c"

#define BAYER_BGGR
#define BAYER_16BE
#define BAYER_RENAME(x) bayer_bggr16be_to_##x
#include "bayer_template.c"

#define BAYER_RGGB
#define BAYER_8
#define BAYER_RENAME(x) bayer_rggb8_to_##x
#include "bayer_template.c"

#define BAYER_RGGB
#define BAYER_16LE
#define BAYER_RENAME(x) bayer_rggb16le_to_##x
#include "bayer_template.c"

#define BAYER_RGGB
#define BAYER_16BE
#define BAYER_RENAME(x) bayer_rggb16be_to_##x
#include "bayer_template.c"

static int bayer_to_rgb24_wrapper(SwsInternal *c, const uint8_t *const src[],
                                  const int srcStride[], int srcSliceY, int srcSliceH,
                                  uint8_t *const dst[], const int dstStride[])
{
    uint8_t *dstPtr= dst[0] + srcSliceY * dstStride[0];
    const uint8_t *srcPtr= src[0];
    int i;
    void (*copy)       (const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int width);
    void (*interpolate)(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int width);

    switch(c->opts.src_format) {
#define CASE(pixfmt, prefix) \
    case pixfmt: copy        = bayer_##prefix##_to_rgb24_copy; \
                 interpolate = bayer_##prefix##_to_rgb24_interpolate; \
                 break;
    CASE(AV_PIX_FMT_BAYER_BGGR8,    bggr8)
    CASE(AV_PIX_FMT_BAYER_BGGR16LE, bggr16le)
    CASE(AV_PIX_FMT_BAYER_BGGR16BE, bggr16be)
    CASE(AV_PIX_FMT_BAYER_RGGB8,    rggb8)
    CASE(AV_PIX_FMT_BAYER_RGGB16LE, rggb16le)
    CASE(AV_PIX_FMT_BAYER_RGGB16BE, rggb16be)
    CASE(AV_PIX_FMT_BAYER_GBRG8,    gbrg8)
    CASE(AV_PIX_FMT_BAYER_GBRG16LE, gbrg16le)
    CASE(AV_PIX_FMT_BAYER_GBRG16BE, gbrg16be)
    CASE(AV_PIX_FMT_BAYER_GRBG8,    grbg8)
    CASE(AV_PIX_FMT_BAYER_GRBG16LE, grbg16le)
    CASE(AV_PIX_FMT_BAYER_GRBG16BE, grbg16be)
#undef CASE
    default: return 0;
    }

    av_assert0(srcSliceH > 1);

    copy(srcPtr, srcStride[0], dstPtr, dstStride[0], c->opts.src_w);
    srcPtr += 2 * srcStride[0];
    dstPtr += 2 * dstStride[0];

    for (i = 2; i < srcSliceH - 2; i += 2) {
        interpolate(srcPtr, srcStride[0], dstPtr, dstStride[0], c->opts.src_w);
        srcPtr += 2 * srcStride[0];
        dstPtr += 2 * dstStride[0];
    }

    if (i + 1 == srcSliceH) {
        copy(srcPtr, -srcStride[0], dstPtr, -dstStride[0], c->opts.src_w);
    } else if (i < srcSliceH)
        copy(srcPtr, srcStride[0], dstPtr, dstStride[0], c->opts.src_w);
    return srcSliceH;
}

static int bayer_to_rgb48_wrapper(SwsInternal *c, const uint8_t *const src[],
                                  const int srcStride[], int srcSliceY, int srcSliceH,
                                  uint8_t *const dst[], const int dstStride[])
{
    uint8_t *dstPtr= dst[0] + srcSliceY * dstStride[0];
    const uint8_t *srcPtr= src[0];
    int i;
    void (*copy)       (const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int width);
    void (*interpolate)(const uint8_t *src, int src_stride, uint8_t *dst, int dst_stride, int width);

    switch(c->opts.src_format) {
#define CASE(pixfmt, prefix) \
    case pixfmt: copy        = bayer_##prefix##_to_rgb48_copy; \
                 interpolate = bayer_##prefix##_to_rgb48_interpolate; \
                 break;
    CASE(AV_PIX_FMT_BAYER_BGGR8,    bggr8)
    CASE(AV_PIX_FMT_BAYER_BGGR16LE, bggr16le)
    CASE(AV_PIX_FMT_BAYER_BGGR16BE, bggr16be)
    CASE(AV_PIX_FMT_BAYER_RGGB8,    rggb8)
    CASE(AV_PIX_FMT_BAYER_RGGB16LE, rggb16le)
    CASE(AV_PIX_FMT_BAYER_RGGB16BE, rggb16be)
    CASE(AV_PIX_FMT_BAYER_GBRG8,    gbrg8)
    CASE(AV_PIX_FMT_BAYER_GBRG16LE, gbrg16le)
    CASE(AV_PIX_FMT_BAYER_GBRG16BE, gbrg16be)
    CASE(AV_PIX_FMT_BAYER_GRBG8,    grbg8)
    CASE(AV_PIX_FMT_BAYER_GRBG16LE, grbg16le)
    CASE(AV_PIX_FMT_BAYER_GRBG16BE, grbg16be)
#undef CASE
    default: return 0;
    }

    av_assert0(srcSliceH > 1);

    copy(srcPtr, srcStride[0], dstPtr, dstStride[0], c->opts.src_w);
    srcPtr += 2 * srcStride[0];
    dstPtr += 2 * dstStride[0];

    for (i = 2; i < srcSliceH - 2; i += 2) {
        interpolate(srcPtr, srcStride[0], dstPtr, dstStride[0], c->opts.src_w);
        srcPtr += 2 * srcStride[0];
        dstPtr += 2 * dstStride[0];
    }

    if (i + 1 == srcSliceH) {
        copy(srcPtr, -srcStride[0], dstPtr, -dstStride[0], c->opts.src_w);
    } else if (i < srcSliceH)
        copy(srcPtr, srcStride[0], dstPtr, dstStride[0], c->opts.src_w);
    return srcSliceH;
}

static int bayer_to_yv12_wrapper(SwsInternal *c, const uint8_t *const src[],
                                 const int srcStride[], int srcSliceY, int srcSliceH,
                                 uint8_t *const dst[], const int dstStride[])
{
    const uint8_t *srcPtr= src[0];
    uint8_t *dstY= dst[0] + srcSliceY * dstStride[0];
    uint8_t *dstU= dst[1] + srcSliceY * dstStride[1] / 2;
    uint8_t *dstV= dst[2] + srcSliceY * dstStride[2] / 2;
    int i;
    void (*copy)       (const uint8_t *src, int src_stride, uint8_t *dstY, uint8_t *dstU, uint8_t *dstV, int luma_stride, int width, const int32_t *rgb2yuv);
    void (*interpolate)(const uint8_t *src, int src_stride, uint8_t *dstY, uint8_t *dstU, uint8_t *dstV, int luma_stride, int width, const int32_t *rgb2yuv);

    switch(c->opts.src_format) {
#define CASE(pixfmt, prefix) \
    case pixfmt: copy        = bayer_##prefix##_to_yv12_copy; \
                 interpolate = bayer_##prefix##_to_yv12_interpolate; \
                 break;
    CASE(AV_PIX_FMT_BAYER_BGGR8,    bggr8)
    CASE(AV_PIX_FMT_BAYER_BGGR16LE, bggr16le)
    CASE(AV_PIX_FMT_BAYER_BGGR16BE, bggr16be)
    CASE(AV_PIX_FMT_BAYER_RGGB8,    rggb8)
    CASE(AV_PIX_FMT_BAYER_RGGB16LE, rggb16le)
    CASE(AV_PIX_FMT_BAYER_RGGB16BE, rggb16be)
    CASE(AV_PIX_FMT_BAYER_GBRG8,    gbrg8)
    CASE(AV_PIX_FMT_BAYER_GBRG16LE, gbrg16le)
    CASE(AV_PIX_FMT_BAYER_GBRG16BE, gbrg16be)
    CASE(AV_PIX_FMT_BAYER_GRBG8,    grbg8)
    CASE(AV_PIX_FMT_BAYER_GRBG16LE, grbg16le)
    CASE(AV_PIX_FMT_BAYER_GRBG16BE, grbg16be)
#undef CASE
    default: return 0;
    }

    av_assert0(srcSliceH > 1);

    copy(srcPtr, srcStride[0], dstY, dstU, dstV, dstStride[0], c->opts.src_w, c->input_rgb2yuv_table);
    srcPtr += 2 * srcStride[0];
    dstY   += 2 * dstStride[0];
    dstU   +=     dstStride[1];
    dstV   +=     dstStride[1];

    for (i = 2; i < srcSliceH - 2; i += 2) {
        interpolate(srcPtr, srcStride[0], dstY, dstU, dstV, dstStride[0], c->opts.src_w, c->input_rgb2yuv_table);
        srcPtr += 2 * srcStride[0];
        dstY   += 2 * dstStride[0];
        dstU   +=     dstStride[1];
        dstV   +=     dstStride[1];
    }

    if (i + 1 == srcSliceH) {
        copy(srcPtr, -srcStride[0], dstY, dstU, dstV, -dstStride[0], c->opts.src_w, c->input_rgb2yuv_table);
    } else if (i < srcSliceH)
        copy(srcPtr, srcStride[0], dstY, dstU, dstV, dstStride[0], c->opts.src_w, c->input_rgb2yuv_table);
    return srcSliceH;
}

#define isRGBA32(x) (            \
           (x) == AV_PIX_FMT_ARGB   \
        || (x) == AV_PIX_FMT_RGBA   \
        || (x) == AV_PIX_FMT_BGRA   \
        || (x) == AV_PIX_FMT_ABGR   \
        )

#define isRGBA64(x) (                \
           (x) == AV_PIX_FMT_RGBA64LE   \
        || (x) == AV_PIX_FMT_RGBA64BE   \
        || (x) == AV_PIX_FMT_BGRA64LE   \
        || (x) == AV_PIX_FMT_BGRA64BE   \
        )

#define isRGB48(x) (                \
           (x) == AV_PIX_FMT_RGB48LE   \
        || (x) == AV_PIX_FMT_RGB48BE   \
        || (x) == AV_PIX_FMT_BGR48LE   \
        || (x) == AV_PIX_FMT_BGR48BE   \
        )

#define isAYUV(x) (                 \
           (x) == AV_PIX_FMT_AYUV   \
        || (x) == AV_PIX_FMT_VUYA   \
        || (x) == AV_PIX_FMT_VUYX   \
        || (x) == AV_PIX_FMT_UYVA   \
        )

#define isX2RGB(x) (                   \
           (x) == AV_PIX_FMT_X2RGB10LE \
        || (x) == AV_PIX_FMT_X2BGR10LE \
        )

/* {RGB,BGR}{15,16,24,32,32_1} -> {RGB,BGR}{15,16,24,32} */
typedef void (* rgbConvFn) (const uint8_t *, uint8_t *, int);
static rgbConvFn findRgbConvFn(SwsInternal *c)
{
    const enum AVPixelFormat srcFormat = c->opts.src_format;
    const enum AVPixelFormat dstFormat = c->opts.dst_format;
    const int srcId = c->srcFormatBpp;
    const int dstId = c->dstFormatBpp;
    rgbConvFn conv = NULL;

#define IS_NOT_NE(bpp, desc) \
    (((bpp + 7) >> 3) == 2 && \
     (!(desc->flags & AV_PIX_FMT_FLAG_BE) != !HAVE_BIGENDIAN))

#define CONV_IS(src, dst) (srcFormat == AV_PIX_FMT_##src && dstFormat == AV_PIX_FMT_##dst)

    if (isRGBA32(srcFormat) && isRGBA32(dstFormat)) {
        if (     CONV_IS(ABGR, RGBA)
              || CONV_IS(ARGB, BGRA)
              || CONV_IS(BGRA, ARGB)
              || CONV_IS(RGBA, ABGR)) conv = shuffle_bytes_3210;
        else if (CONV_IS(ABGR, ARGB)
              || CONV_IS(ARGB, ABGR)) conv = shuffle_bytes_0321;
        else if (CONV_IS(ABGR, BGRA)
              || CONV_IS(ARGB, RGBA)) conv = shuffle_bytes_1230;
        else if (CONV_IS(BGRA, RGBA)
              || CONV_IS(RGBA, BGRA)) conv = shuffle_bytes_2103;
        else if (CONV_IS(BGRA, ABGR)
              || CONV_IS(RGBA, ARGB)) conv = shuffle_bytes_3012;
    } else if (isRGB48(srcFormat) && isRGB48(dstFormat)) {
        if      (CONV_IS(RGB48LE, BGR48LE)
              || CONV_IS(BGR48LE, RGB48LE)
              || CONV_IS(RGB48BE, BGR48BE)
              || CONV_IS(BGR48BE, RGB48BE)) conv = rgb48tobgr48_nobswap;
        else if (CONV_IS(RGB48LE, BGR48BE)
              || CONV_IS(BGR48LE, RGB48BE)
              || CONV_IS(RGB48BE, BGR48LE)
              || CONV_IS(BGR48BE, RGB48LE)) conv = rgb48tobgr48_bswap;
    } else if (isRGB48(srcFormat) && isRGBA64(dstFormat)) {
        if      (CONV_IS(RGB48LE, BGRA64LE)
              || CONV_IS(BGR48LE, RGBA64LE)
              || CONV_IS(RGB48BE, BGRA64BE)
              || CONV_IS(BGR48BE, RGBA64BE)) conv = rgb48tobgr64_nobswap;
        else if (CONV_IS(RGB48LE, BGRA64BE)
              || CONV_IS(BGR48LE, RGBA64BE)
              || CONV_IS(RGB48BE, BGRA64LE)
              || CONV_IS(BGR48BE, RGBA64LE)) conv = rgb48tobgr64_bswap;
        if      (CONV_IS(RGB48LE, RGBA64LE)
              || CONV_IS(BGR48LE, BGRA64LE)
              || CONV_IS(RGB48BE, RGBA64BE)
              || CONV_IS(BGR48BE, BGRA64BE)) conv = rgb48to64_nobswap;
        else if (CONV_IS(RGB48LE, RGBA64BE)
              || CONV_IS(BGR48LE, BGRA64BE)
              || CONV_IS(RGB48BE, RGBA64LE)
              || CONV_IS(BGR48BE, BGRA64LE)) conv = rgb48to64_bswap;
    } else if (isRGBA64(srcFormat) && isRGB48(dstFormat)) {
        if      (CONV_IS(RGBA64LE, BGR48LE)
              || CONV_IS(BGRA64LE, RGB48LE)
              || CONV_IS(RGBA64BE, BGR48BE)
              || CONV_IS(BGRA64BE, RGB48BE)) conv = rgb64tobgr48_nobswap;
        else if (CONV_IS(RGBA64LE, BGR48BE)
              || CONV_IS(BGRA64LE, RGB48BE)
              || CONV_IS(RGBA64BE, BGR48LE)
              || CONV_IS(BGRA64BE, RGB48LE)) conv = rgb64tobgr48_bswap;
        else if (CONV_IS(RGBA64LE, RGB48LE)
              || CONV_IS(BGRA64LE, BGR48LE)
              || CONV_IS(RGBA64BE, RGB48BE)
              || CONV_IS(BGRA64BE, BGR48BE)) conv = rgb64to48_nobswap;
        else if (CONV_IS(RGBA64LE, RGB48BE)
              || CONV_IS(BGRA64LE, BGR48BE)
              || CONV_IS(RGBA64BE, RGB48LE)
              || CONV_IS(BGRA64BE, BGR48LE)) conv = rgb64to48_bswap;
    } else if (isX2RGB(srcFormat) && isRGB48(dstFormat)) {
        if      (CONV_IS(X2RGB10LE, RGB48LE)
              || CONV_IS(X2BGR10LE, BGR48LE)) conv = HAVE_BIGENDIAN ? x2rgb10to48_bswap
                                                                    : x2rgb10to48_nobswap;
        else if (CONV_IS(X2RGB10LE, RGB48BE)
              || CONV_IS(X2BGR10LE, BGR48BE)) conv = HAVE_BIGENDIAN ? x2rgb10to48_nobswap
                                                                    : x2rgb10to48_bswap;
        else if (CONV_IS(X2RGB10LE, BGR48LE)
              || CONV_IS(X2BGR10LE, RGB48LE)) conv = HAVE_BIGENDIAN ? x2rgb10tobgr48_bswap
                                                                    : x2rgb10tobgr48_nobswap;
        else if (CONV_IS(X2RGB10LE, BGR48BE)
              || CONV_IS(X2BGR10LE, RGB48BE)) conv = HAVE_BIGENDIAN ? x2rgb10tobgr48_nobswap
                                                                    : x2rgb10tobgr48_bswap;
    } else if (isX2RGB(srcFormat) && isRGBA64(dstFormat)) {
        if      (CONV_IS(X2RGB10LE, RGBA64LE)
              || CONV_IS(X2BGR10LE, BGRA64LE)) conv = HAVE_BIGENDIAN ? x2rgb10to64_bswap
                                                                     : x2rgb10to64_nobswap;
        else if (CONV_IS(X2RGB10LE, RGBA64BE)
              || CONV_IS(X2BGR10LE, BGRA64BE)) conv = HAVE_BIGENDIAN ? x2rgb10to64_nobswap
                                                                     : x2rgb10to64_bswap;
        else if (CONV_IS(X2RGB10LE, BGRA64LE)
              || CONV_IS(X2BGR10LE, RGBA64LE)) conv = HAVE_BIGENDIAN ? x2rgb10tobgr64_bswap
                                                                     : x2rgb10tobgr64_nobswap;
        else if (CONV_IS(X2RGB10LE, BGRA64BE)
              || CONV_IS(X2BGR10LE, RGBA64BE)) conv = HAVE_BIGENDIAN ? x2rgb10tobgr64_nobswap
                                                                     : x2rgb10tobgr64_bswap;
    } else if (isAYUV(srcFormat) && isAYUV(dstFormat)) {
        /* VUYX only for dst, to avoid copying undefined bytes */
        if (     CONV_IS(AYUV, VUYA)
              || CONV_IS(AYUV, VUYX)
              || CONV_IS(VUYA, AYUV)) conv = shuffle_bytes_3210;
        else if (CONV_IS(AYUV, UYVA)) conv = shuffle_bytes_2130;
        else if (CONV_IS(VUYA, UYVA)) conv = shuffle_bytes_1203;
        else if (CONV_IS(UYVA, AYUV)) conv = shuffle_bytes_3102;
        else if (CONV_IS(UYVA, VUYA)
              || CONV_IS(UYVA, VUYX)) conv = shuffle_bytes_2013;
    } else
    /* BGR -> BGR */
    if ((isBGRinInt(srcFormat) && isBGRinInt(dstFormat)) ||
        (isRGBinInt(srcFormat) && isRGBinInt(dstFormat))) {
        switch (srcId | (dstId << 16)) {
        case 0x000F000C: conv = rgb12to15; break;
        case 0x000F0010: conv = rgb16to15; break;
        case 0x000F0018: conv = rgb24to15; break;
        case 0x000F0020: conv = rgb32to15; break;
        case 0x0010000F: conv = rgb15to16; break;
        case 0x00100018: conv = rgb24to16; break;
        case 0x00100020: conv = rgb32to16; break;
        case 0x0018000F: conv = rgb15to24; break;
        case 0x00180010: conv = rgb16to24; break;
        case 0x00180020: conv = rgb32to24; break;
        case 0x0020000F: conv = rgb15to32; break;
        case 0x00200010: conv = rgb16to32; break;
        case 0x00200018: conv = rgb24to32; break;
        }
    } else if ((isBGRinInt(srcFormat) && isRGBinInt(dstFormat)) ||
               (isRGBinInt(srcFormat) && isBGRinInt(dstFormat))) {
        switch (srcId | (dstId << 16)) {
        case 0x000C000C: conv = rgb12tobgr12; break;
        case 0x000F000F: conv = rgb15tobgr15; break;
        case 0x000F0010: conv = rgb16tobgr15; break;
        case 0x000F0018: conv = rgb24tobgr15; break;
        case 0x000F0020: conv = rgb32tobgr15; break;
        case 0x0010000F: conv = rgb15tobgr16; break;
        case 0x00100010: conv = rgb16tobgr16; break;
        case 0x00100018: conv = rgb24tobgr16; break;
        case 0x00100020: conv = rgb32tobgr16; break;
        case 0x0018000F: conv = rgb15tobgr24; break;
        case 0x00180010: conv = rgb16tobgr24; break;
        case 0x00180018: conv = rgb24tobgr24; break;
        case 0x00180020: conv = rgb32tobgr24; break;
        case 0x0020000F: conv = rgb15tobgr32; break;
        case 0x00200010: conv = rgb16tobgr32; break;
        case 0x00200018: conv = rgb24tobgr32; break;
        }
    }

    if ((dstFormat == AV_PIX_FMT_RGB32_1 || dstFormat == AV_PIX_FMT_BGR32_1) && !isRGBA32(srcFormat) && ALT32_CORR<0)
        return NULL;

    // Maintain symmetry between endianness
    if (c->opts.flags & SWS_BITEXACT)
        if ((dstFormat == AV_PIX_FMT_RGB32   || dstFormat == AV_PIX_FMT_BGR32  ) && !isRGBA32(srcFormat) && ALT32_CORR>0)
            return NULL;

    return conv;
}

/* {RGB,BGR}{15,16,24,32,32_1} -> {RGB,BGR}{15,16,24,32} */
static int rgbToRgbWrapper(SwsInternal *c, const uint8_t *const src[], const int srcStride[],
                           int srcSliceY, int srcSliceH, uint8_t *const dst[],
                           const int dstStride[])

{
    const enum AVPixelFormat srcFormat = c->opts.src_format;
    const enum AVPixelFormat dstFormat = c->opts.dst_format;
    const AVPixFmtDescriptor *desc_src = av_pix_fmt_desc_get(c->opts.src_format);
    const AVPixFmtDescriptor *desc_dst = av_pix_fmt_desc_get(c->opts.dst_format);
    const int srcBpp = (c->srcFormatBpp + 7) >> 3;
    const int dstBpp = (c->dstFormatBpp + 7) >> 3;
    rgbConvFn conv = findRgbConvFn(c);

    if (!conv) {
        av_log(c, AV_LOG_ERROR, "internal error %s -> %s converter\n",
               av_get_pix_fmt_name(srcFormat), av_get_pix_fmt_name(dstFormat));
    } else {
        const uint8_t *srcPtr = src[0];
              uint8_t *dstPtr = dst[0];
        int src_bswap = IS_NOT_NE(c->srcFormatBpp, desc_src);
        int dst_bswap = IS_NOT_NE(c->dstFormatBpp, desc_dst);

        if ((srcFormat == AV_PIX_FMT_RGB32_1 || srcFormat == AV_PIX_FMT_BGR32_1) &&
            !isRGBA32(dstFormat))
            srcPtr += ALT32_CORR;

        if ((dstFormat == AV_PIX_FMT_RGB32_1 || dstFormat == AV_PIX_FMT_BGR32_1) &&
            !isRGBA32(srcFormat)) {
            int i;
            av_assert0(ALT32_CORR == 1);
            for (i = 0; i < srcSliceH; i++)
                dstPtr[dstStride[0] * (srcSliceY + i)] = 255;
            dstPtr += ALT32_CORR;
        }

        if (dstStride[0] * srcBpp == srcStride[0] * dstBpp && srcStride[0] > 0 &&
            !(srcStride[0] % srcBpp) && !dst_bswap && !src_bswap)
            conv(srcPtr, dstPtr + dstStride[0] * srcSliceY,
                 (srcSliceH - 1) * srcStride[0] + c->opts.src_w * srcBpp);
        else {
            int i, j;
            dstPtr += dstStride[0] * srcSliceY;

            for (i = 0; i < srcSliceH; i++) {
                if(src_bswap) {
                    for(j=0; j<c->opts.src_w; j++)
                        ((uint16_t*)c->formatConvBuffer)[j] = av_bswap16(((uint16_t*)srcPtr)[j]);
                    conv(c->formatConvBuffer, dstPtr, c->opts.src_w * srcBpp);
                }else
                    conv(srcPtr, dstPtr, c->opts.src_w * srcBpp);
                if(dst_bswap)
                    for(j=0; j<c->opts.src_w; j++)
                        ((uint16_t*)dstPtr)[j] = av_bswap16(((uint16_t*)dstPtr)[j]);
                srcPtr += srcStride[0];
                dstPtr += dstStride[0];
            }
        }
    }
    return srcSliceH;
}

static int bgr24ToYv12Wrapper(SwsInternal *c, const uint8_t *const src[],
                              const int srcStride[], int srcSliceY, int srcSliceH,
                              uint8_t *const dst[], const int dstStride[])
{
    ff_rgb24toyv12(
        src[0],
        dst[0] +  srcSliceY       * dstStride[0],
        dst[1] + (srcSliceY >> 1) * dstStride[1],
        dst[2] + (srcSliceY >> 1) * dstStride[2],
        c->opts.src_w, srcSliceH,
        dstStride[0], dstStride[1], srcStride[0],
        c->input_rgb2yuv_table);
    if (dst[3])
        fillPlane(dst[3], dstStride[3], c->opts.src_w, srcSliceH, srcSliceY, 255);
    return srcSliceH;
}

static int yvu9ToYv12Wrapper(SwsInternal *c, const uint8_t *const src[],
                             const int srcStride[], int srcSliceY, int srcSliceH,
                             uint8_t *const dst[], const int dstStride[])
{
    ff_copyPlane(src[0], srcStride[0], srcSliceY, srcSliceH, c->opts.src_w,
                 dst[0], dstStride[0]);

    planar2x(src[1], dst[1] + dstStride[1] * (srcSliceY >> 1), c->chrSrcW,
             srcSliceH >> 2, srcStride[1], dstStride[1]);
    planar2x(src[2], dst[2] + dstStride[2] * (srcSliceY >> 1), c->chrSrcW,
             srcSliceH >> 2, srcStride[2], dstStride[2]);
    if (dst[3])
        fillPlane(dst[3], dstStride[3], c->opts.src_w, srcSliceH, srcSliceY, 255);
    return srcSliceH;
}

static int uint_y_to_float_y_wrapper(SwsInternal *c, const uint8_t *const src[],
                                     const int srcStride[], int srcSliceY,
                                     int srcSliceH, uint8_t *const dst[], const int dstStride[])
{
    int y, x;
    ptrdiff_t dstStrideFloat = dstStride[0] >> 2;
    const uint8_t *srcPtr = src[0];
    float *dstPtr = (float *)(dst[0] + dstStride[0] * srcSliceY);

    for (y = 0; y < srcSliceH; ++y){
        for (x = 0; x < c->opts.src_w; ++x){
            dstPtr[x] = c->uint2float_lut[srcPtr[x]];
        }
        srcPtr += srcStride[0];
        dstPtr += dstStrideFloat;
    }

    return srcSliceH;
}

static int float_y_to_uint_y_wrapper(SwsInternal *c,
                                     const uint8_t *const src[],
                                     const int srcStride[], int srcSliceY,
                                     int srcSliceH, uint8_t *const dst[],
                                     const int dstStride[])
{
    int y, x;
    ptrdiff_t srcStrideFloat = srcStride[0] >> 2;
    const float *srcPtr = (const float *)src[0];
    uint8_t *dstPtr = dst[0] + dstStride[0] * srcSliceY;

    for (y = 0; y < srcSliceH; ++y){
        for (x = 0; x < c->opts.src_w; ++x){
            dstPtr[x] = av_clip_uint8(lrintf(255.0f * srcPtr[x]));
        }
        srcPtr += srcStrideFloat;
        dstPtr += dstStride[0];
    }

    return srcSliceH;
}

/* unscaled copy like stuff (assumes nearly identical formats) */
static int packedCopyWrapper(SwsInternal *c, const uint8_t *const src[],
                             const int srcStride[], int srcSliceY, int srcSliceH,
                             uint8_t *const dst[], const int dstStride[])
{
    if (dstStride[0] == srcStride[0] && srcStride[0] > 0)
        memcpy(dst[0] + dstStride[0] * srcSliceY, src[0], srcSliceH * dstStride[0]);
    else {
        int i;
        const uint8_t *srcPtr = src[0];
        uint8_t *dstPtr = dst[0] + dstStride[0] * srcSliceY;
        int length = 0;

        /* universal length finder */
        while (length + c->opts.src_w <= FFABS(dstStride[0]) &&
               length + c->opts.src_w <= FFABS(srcStride[0]))
            length += c->opts.src_w;
        av_assert1(length != 0);

        for (i = 0; i < srcSliceH; i++) {
            memcpy(dstPtr, srcPtr, length);
            srcPtr += srcStride[0];
            dstPtr += dstStride[0];
        }
    }
    return srcSliceH;
}

#define DITHER_COPY(dst, dstStride, src, srcStride, bswap, dbswap)\
    unsigned shift= src_depth-dst_depth, tmp;\
    unsigned bias = 1 << (shift - 1);\
    if (c->opts.dither == SWS_DITHER_NONE) {\
        for (i = 0; i < height; i++) {\
            for (j = 0; j < length-7; j+=8) {\
                tmp = ((bswap(src[j+0]) >> src_shift) + bias)>>shift; dst[j+0] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+1]) >> src_shift) + bias)>>shift; dst[j+1] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+2]) >> src_shift) + bias)>>shift; dst[j+2] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+3]) >> src_shift) + bias)>>shift; dst[j+3] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+4]) >> src_shift) + bias)>>shift; dst[j+4] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+5]) >> src_shift) + bias)>>shift; dst[j+5] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+6]) >> src_shift) + bias)>>shift; dst[j+6] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+7]) >> src_shift) + bias)>>shift; dst[j+7] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
            }\
            for (; j < length; j++) {\
                tmp = (bswap(src[j]) + bias)>>shift; dst[j] = dbswap(tmp - (tmp>>dst_depth) << dst_shift);\
            }\
            dst += dstStride;\
            src += srcStride;\
        }\
    } else if (shiftonly) {\
        for (i = 0; i < height; i++) {\
            const uint8_t *dither= dithers[shift-1][i&7];\
            for (j = 0; j < length-7; j+=8) {\
                tmp = ((bswap(src[j+0]) >> src_shift) + dither[0])>>shift; dst[j+0] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+1]) >> src_shift) + dither[1])>>shift; dst[j+1] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+2]) >> src_shift) + dither[2])>>shift; dst[j+2] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+3]) >> src_shift) + dither[3])>>shift; dst[j+3] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+4]) >> src_shift) + dither[4])>>shift; dst[j+4] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+5]) >> src_shift) + dither[5])>>shift; dst[j+5] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+6]) >> src_shift) + dither[6])>>shift; dst[j+6] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
                tmp = ((bswap(src[j+7]) >> src_shift) + dither[7])>>shift; dst[j+7] = dbswap((tmp - (tmp>>dst_depth)) << dst_shift);\
            }\
            for (; j < length; j++) {\
                tmp = (bswap(src[j]) + dither[j&7])>>shift; dst[j] = dbswap(tmp - (tmp>>dst_depth) << dst_shift);\
            }\
            dst += dstStride;\
            src += srcStride;\
        }\
    } else {\
        for (i = 0; i < height; i++) {\
            const uint8_t *dither= dithers[shift-1][i&7];\
            for (j = 0; j < length-7; j+=8) {\
                tmp = bswap(src[j+0]) >> src_shift; dst[j+0] = dbswap(((tmp - (tmp>>dst_depth) + dither[0])>>shift) << dst_shift);\
                tmp = bswap(src[j+1]) >> src_shift; dst[j+1] = dbswap(((tmp - (tmp>>dst_depth) + dither[1])>>shift) << dst_shift);\
                tmp = bswap(src[j+2]) >> src_shift; dst[j+2] = dbswap(((tmp - (tmp>>dst_depth) + dither[2])>>shift) << dst_shift);\
                tmp = bswap(src[j+3]) >> src_shift; dst[j+3] = dbswap(((tmp - (tmp>>dst_depth) + dither[3])>>shift) << dst_shift);\
                tmp = bswap(src[j+4]) >> src_shift; dst[j+4] = dbswap(((tmp - (tmp>>dst_depth) + dither[4])>>shift) << dst_shift);\
                tmp = bswap(src[j+5]) >> src_shift; dst[j+5] = dbswap(((tmp - (tmp>>dst_depth) + dither[5])>>shift) << dst_shift);\
                tmp = bswap(src[j+6]) >> src_shift; dst[j+6] = dbswap(((tmp - (tmp>>dst_depth) + dither[6])>>shift) << dst_shift);\
                tmp = bswap(src[j+7]) >> src_shift; dst[j+7] = dbswap(((tmp - (tmp>>dst_depth) + dither[7])>>shift) << dst_shift);\
            }\
            for (; j < length; j++) {\
                tmp = bswap(src[j]); dst[j] = dbswap((tmp - (tmp>>dst_depth) + dither[j&7])>>shift);\
            }\
            dst += dstStride;\
            src += srcStride;\
        }\
    }

static int planarCopyWrapper(SwsInternal *c, const uint8_t *const src[],
                             const int srcStride[], int srcSliceY, int srcSliceH,
                             uint8_t *const dst[], const int dstStride[])
{
    const AVPixFmtDescriptor *desc_src = av_pix_fmt_desc_get(c->opts.src_format);
    const AVPixFmtDescriptor *desc_dst = av_pix_fmt_desc_get(c->opts.dst_format);
    int plane, i, j;
    for (plane = 0; plane < 4 && dst[plane] != NULL; plane++) {
        int length = (plane == 0 || plane == 3) ? c->opts.src_w  : AV_CEIL_RSHIFT(c->opts.src_w,   c->chrDstHSubSample);
        int y =      (plane == 0 || plane == 3) ? srcSliceY: AV_CEIL_RSHIFT(srcSliceY, c->chrDstVSubSample);
        int height = (plane == 0 || plane == 3) ? srcSliceH: AV_CEIL_RSHIFT(srcSliceH, c->chrDstVSubSample);
        const uint8_t *srcPtr = src[plane];
        uint8_t *dstPtr = dst[plane] + dstStride[plane] * y;
        int shiftonly = plane == 1 || plane == 2 || (!c->opts.src_range && plane == 0);
        if (plane == 1 && isSemiPlanarYUV(c->opts.dst_format))
            length *= 2;

        // ignore palette for GRAY8
        if (plane == 1 && desc_dst->nb_components < 3) continue;
        if (!src[plane] || (plane == 1 && desc_src->nb_components < 3) || (plane == 3 && desc_src->nb_components <= 3)) {
            if (is16BPS(c->opts.dst_format) || isNBPS(c->opts.dst_format)) {
                fillPlane16(dst[plane], dstStride[plane], length, height, y,
                        plane == 3, desc_dst->comp[plane].depth,
                        isBE(c->opts.dst_format));
            } else {
                fillPlane(dst[plane], dstStride[plane], length, height, y,
                        (plane == 3) ? 255 : 128);
            }
        } else {
            if(isNBPS(c->opts.src_format) || isNBPS(c->opts.dst_format)
               || (is16BPS(c->opts.src_format) != is16BPS(c->opts.dst_format))
            ) {
                const int src_depth = desc_src->comp[plane].depth;
                const int dst_depth = desc_dst->comp[plane].depth;
                const int src_shift = desc_src->comp[plane].shift;
                const int dst_shift = desc_dst->comp[plane].shift;
                const uint16_t *srcPtr2 = (const uint16_t *) srcPtr;
                uint16_t *dstPtr2 = (uint16_t*)dstPtr;

                if (dst_depth == 8) {
                    av_assert1(src_depth > 8);
                    if(isBE(c->opts.src_format) == HAVE_BIGENDIAN){
                        DITHER_COPY(dstPtr, dstStride[plane], srcPtr2, srcStride[plane]/2, , )
                    } else {
                        DITHER_COPY(dstPtr, dstStride[plane], srcPtr2, srcStride[plane]/2, av_bswap16, )
                    }
                } else if (src_depth == 8) {
                    for (i = 0; i < height; i++) {
                        #define COPY816(w)\
                        if (shiftonly) {\
                            for (j = 0; j < length; j++)\
                                w(&dstPtr2[j], (srcPtr[j]<<(dst_depth-8)) << dst_shift);\
                        } else {\
                            for (j = 0; j < length; j++)\
                                w(&dstPtr2[j], ((srcPtr[j]<<(dst_depth-8)) |\
                                                (srcPtr[j]>>(2*8-dst_depth))) << dst_shift);\
                        }
                        if(isBE(c->opts.dst_format)){
                            COPY816(AV_WB16)
                        } else {
                            COPY816(AV_WL16)
                        }
                        dstPtr2 += dstStride[plane]/2;
                        srcPtr  += srcStride[plane];
                    }
                } else if (src_depth <= dst_depth) {
                    unsigned shift = dst_depth - src_depth;
                    for (i = 0; i < height; i++) {
                        j = 0;
                        if(isBE(c->opts.src_format) == HAVE_BIGENDIAN &&
                           isBE(c->opts.dst_format) == HAVE_BIGENDIAN &&
                           shiftonly) {
#if HAVE_FAST_64BIT
                            for (; j < length - 3; j += 4) {
                                uint64_t v = AV_RN64A(srcPtr2 + j) >> src_shift;
                                AV_WN64A(dstPtr2 + j, (v << shift) << dst_shift);
                            }
#else
                            for (; j < length - 1; j += 2) {
                                uint32_t v = AV_RN32A(srcPtr2 + j) >> src_shift;
                                AV_WN32A(dstPtr2 + j, (v << shift) << dst_shift);
                            }
#endif
                        }
#define COPY_UP(r,w) \
    if(shiftonly){\
        for (; j < length; j++){ \
            unsigned int v= r(&srcPtr2[j]) >> src_shift;\
            w(&dstPtr2[j], (v << shift) << dst_shift);\
        }\
    }else{\
        for (; j < length; j++){ \
            unsigned int v= r(&srcPtr2[j]) >> src_shift;\
            w(&dstPtr2[j], ((v << shift) | (v>>(2*src_depth-dst_depth))) << dst_shift);\
        }\
    }
                        if(isBE(c->opts.src_format)){
                            if(isBE(c->opts.dst_format)){
                                COPY_UP(AV_RB16, AV_WB16)
                            } else {
                                COPY_UP(AV_RB16, AV_WL16)
                            }
                        } else {
                            if(isBE(c->opts.dst_format)){
                                COPY_UP(AV_RL16, AV_WB16)
                            } else {
                                COPY_UP(AV_RL16, AV_WL16)
                            }
                        }
                        dstPtr2 += dstStride[plane]/2;
                        srcPtr2 += srcStride[plane]/2;
                    }
                } else { /* src_depth > dst_depth */
                    if(isBE(c->opts.src_format) == HAVE_BIGENDIAN){
                        if(isBE(c->opts.dst_format) == HAVE_BIGENDIAN){
                            DITHER_COPY(dstPtr2, dstStride[plane]/2, srcPtr2, srcStride[plane]/2, , )
                        } else {
                            DITHER_COPY(dstPtr2, dstStride[plane]/2, srcPtr2, srcStride[plane]/2, , av_bswap16)
                        }
                    }else{
                        if(isBE(c->opts.dst_format) == HAVE_BIGENDIAN){
                            DITHER_COPY(dstPtr2, dstStride[plane]/2, srcPtr2, srcStride[plane]/2, av_bswap16, )
                        } else {
                            DITHER_COPY(dstPtr2, dstStride[plane]/2, srcPtr2, srcStride[plane]/2, av_bswap16, av_bswap16)
                        }
                    }
                }
            } else if (is16BPS(c->opts.src_format) && is16BPS(c->opts.dst_format) &&
                      isBE(c->opts.src_format) != isBE(c->opts.dst_format)) {

                for (i = 0; i < height; i++) {
                    for (j = 0; j < length; j++)
                        ((uint16_t *) dstPtr)[j] = av_bswap16(((const uint16_t *) srcPtr)[j]);
                    srcPtr += srcStride[plane];
                    dstPtr += dstStride[plane];
                }
            } else if (isFloat(c->opts.src_format) && isFloat(c->opts.dst_format) &&
                       isBE(c->opts.src_format) != isBE(c->opts.dst_format)) { /* swap float plane */
                for (i = 0; i < height; i++) {
                    for (j = 0; j < length; j++)
                        ((uint32_t *) dstPtr)[j] = av_bswap32(((const uint32_t *) srcPtr)[j]);
                    srcPtr += srcStride[plane];
                    dstPtr += dstStride[plane];
                }
            } else if (dstStride[plane] == srcStride[plane] &&
                       srcStride[plane] > 0 && srcStride[plane] == length) {
                memcpy(dst[plane] + dstStride[plane] * y, src[plane],
                       height * dstStride[plane]);
            } else {
                if (is16BPS(c->opts.src_format) && is16BPS(c->opts.dst_format))
                    length *= 2;
                else if (desc_src->comp[0].depth == 1)
                    length >>= 3; // monowhite/black
                for (i = 0; i < height; i++) {
                    memcpy(dstPtr, srcPtr, length);
                    srcPtr += srcStride[plane];
                    dstPtr += dstStride[plane];
                }
            }
        }
    }
    return srcSliceH;
}


#define IS_DIFFERENT_ENDIANESS(src_fmt, dst_fmt, pix_fmt)          \
    ((src_fmt == pix_fmt ## BE && dst_fmt == pix_fmt ## LE) ||     \
     (src_fmt == pix_fmt ## LE && dst_fmt == pix_fmt ## BE))


void ff_get_unscaled_swscale(SwsInternal *c)
{
    const enum AVPixelFormat srcFormat = c->opts.src_format;
    const enum AVPixelFormat dstFormat = c->opts.dst_format;
    const int flags = c->opts.flags;
    const int dstH = c->opts.dst_h;
    const int dstW = c->opts.dst_w;
    int needsDither;

    needsDither = isAnyRGB(dstFormat) &&
            c->dstFormatBpp < 24 &&
           (c->dstFormatBpp < c->srcFormatBpp || (!isAnyRGB(srcFormat)));

    /* yv12_to_nv12 */
    if ((srcFormat == AV_PIX_FMT_YUV420P || srcFormat == AV_PIX_FMT_YUVA420P) &&
        (dstFormat == AV_PIX_FMT_NV12 || dstFormat == AV_PIX_FMT_NV21)) {
        c->convert_unscaled = planarToNv12Wrapper;
    }
    /* yv24_to_nv24 */
    if ((srcFormat == AV_PIX_FMT_YUV444P || srcFormat == AV_PIX_FMT_YUVA444P) &&
        (dstFormat == AV_PIX_FMT_NV24 || dstFormat == AV_PIX_FMT_NV42)) {
        c->convert_unscaled = planarToNv24Wrapper;
    }
    /* nv12_to_yv12 */
    if (dstFormat == AV_PIX_FMT_YUV420P &&
        (srcFormat == AV_PIX_FMT_NV12 || srcFormat == AV_PIX_FMT_NV21)) {
        c->convert_unscaled = nv12ToPlanarWrapper;
    }
    /* nv24_to_yv24 */
    if (dstFormat == AV_PIX_FMT_YUV444P &&
        (srcFormat == AV_PIX_FMT_NV24 || srcFormat == AV_PIX_FMT_NV42)) {
        c->convert_unscaled = nv24ToPlanarWrapper;
    }
    /* yuv2bgr */
    if ((srcFormat == AV_PIX_FMT_YUV420P || srcFormat == AV_PIX_FMT_YUV422P ||
         srcFormat == AV_PIX_FMT_YUVA420P) && isAnyRGB(dstFormat) &&
        !(flags & SWS_ACCURATE_RND) && (c->opts.dither == SWS_DITHER_BAYER || c->opts.dither == SWS_DITHER_AUTO) && !(dstH & 1)) {
        c->convert_unscaled = ff_yuv2rgb_get_func_ptr(c);
        c->dst_slice_align = 2;
    }
    /* yuv420p1x_to_p01x */
    if ((srcFormat == AV_PIX_FMT_YUV420P10 || srcFormat == AV_PIX_FMT_YUVA420P10 ||
         srcFormat == AV_PIX_FMT_YUV420P12 ||
         srcFormat == AV_PIX_FMT_YUV420P14 ||
         srcFormat == AV_PIX_FMT_YUV420P16 || srcFormat == AV_PIX_FMT_YUVA420P16) &&
        (dstFormat == AV_PIX_FMT_P010 || dstFormat == AV_PIX_FMT_P016)) {
        c->convert_unscaled = planarToP01xWrapper;
    }
    /* yuv420p_to_p01xle */
    if ((srcFormat == AV_PIX_FMT_YUV420P || srcFormat == AV_PIX_FMT_YUVA420P) &&
        (dstFormat == AV_PIX_FMT_P010LE || dstFormat == AV_PIX_FMT_P016LE)) {
        c->convert_unscaled = planar8ToP01xleWrapper;
    }

    if (srcFormat == AV_PIX_FMT_YUV410P && !(dstH & 3) &&
        (dstFormat == AV_PIX_FMT_YUV420P || dstFormat == AV_PIX_FMT_YUVA420P) &&
        !(flags & SWS_BITEXACT)) {
        c->convert_unscaled = yvu9ToYv12Wrapper;
        c->dst_slice_align = 4;
    }

    /* bgr24toYV12 */
    if (srcFormat == AV_PIX_FMT_BGR24 &&
        (dstFormat == AV_PIX_FMT_YUV420P || dstFormat == AV_PIX_FMT_YUVA420P) &&
        !(flags & SWS_ACCURATE_RND) && !(dstW&1))
        c->convert_unscaled = bgr24ToYv12Wrapper;

    /* AYUV/VUYA/UYVA -> AYUV/VUYA/UYVA */
    if (isAYUV(srcFormat) && isAYUV(dstFormat) && findRgbConvFn(c))
        c->convert_unscaled = rgbToRgbWrapper;

    /* RGB/BGR -> RGB/BGR (no dither needed forms) */
    if (isAnyRGB(srcFormat) && isAnyRGB(dstFormat) && findRgbConvFn(c)
        && (!needsDither || (c->opts.flags&(SWS_FAST_BILINEAR|SWS_POINT))))
        c->convert_unscaled = rgbToRgbWrapper;

    /* RGB to planar RGB */
    if ((srcFormat == AV_PIX_FMT_GBRP && dstFormat == AV_PIX_FMT_GBRAP) ||
        (srcFormat == AV_PIX_FMT_GBRP10 && dstFormat == AV_PIX_FMT_GBRAP10) ||
        (srcFormat == AV_PIX_FMT_GBRP12 && dstFormat == AV_PIX_FMT_GBRAP12) ||
        (srcFormat == AV_PIX_FMT_GBRP14 && dstFormat == AV_PIX_FMT_GBRAP14) ||
        (srcFormat == AV_PIX_FMT_GBRP16 && dstFormat == AV_PIX_FMT_GBRAP16) ||
        (srcFormat == AV_PIX_FMT_GBRAP && dstFormat == AV_PIX_FMT_GBRP) ||
        (srcFormat == AV_PIX_FMT_GBRAP10 && dstFormat == AV_PIX_FMT_GBRP10) ||
        (srcFormat == AV_PIX_FMT_GBRAP12 && dstFormat == AV_PIX_FMT_GBRP12) ||
        (srcFormat == AV_PIX_FMT_GBRAP14 && dstFormat == AV_PIX_FMT_GBRP14) ||
        (srcFormat == AV_PIX_FMT_GBRAP16 && dstFormat == AV_PIX_FMT_GBRP16))
        c->convert_unscaled = planarRgbToplanarRgbWrapper;

#define isByteRGB(f) (             \
        f == AV_PIX_FMT_RGB32   || \
        f == AV_PIX_FMT_RGB32_1 || \
        f == AV_PIX_FMT_RGB24   || \
        f == AV_PIX_FMT_BGR32   || \
        f == AV_PIX_FMT_BGR32_1 || \
        f == AV_PIX_FMT_BGR24)

    if (srcFormat == AV_PIX_FMT_GBRP && isPlanar(srcFormat) && isByteRGB(dstFormat))
        c->convert_unscaled = planarRgbToRgbWrapper;

    if (srcFormat == AV_PIX_FMT_GBRAP && isByteRGB(dstFormat))
        c->convert_unscaled = planarRgbaToRgbWrapper;

    if ((srcFormat == AV_PIX_FMT_RGB48LE  || srcFormat == AV_PIX_FMT_RGB48BE  ||
         srcFormat == AV_PIX_FMT_BGR48LE  || srcFormat == AV_PIX_FMT_BGR48BE  ||
         srcFormat == AV_PIX_FMT_RGBA64LE || srcFormat == AV_PIX_FMT_RGBA64BE ||
         srcFormat == AV_PIX_FMT_BGRA64LE || srcFormat == AV_PIX_FMT_BGRA64BE) &&
        (dstFormat == AV_PIX_FMT_GBRP9LE  || dstFormat == AV_PIX_FMT_GBRP9BE  ||
         dstFormat == AV_PIX_FMT_GBRP10LE || dstFormat == AV_PIX_FMT_GBRP10BE ||
         dstFormat == AV_PIX_FMT_GBRP12LE || dstFormat == AV_PIX_FMT_GBRP12BE ||
         dstFormat == AV_PIX_FMT_GBRP14LE || dstFormat == AV_PIX_FMT_GBRP14BE ||
         dstFormat == AV_PIX_FMT_GBRP16LE || dstFormat == AV_PIX_FMT_GBRP16BE ||
         dstFormat == AV_PIX_FMT_GBRAP10LE || dstFormat == AV_PIX_FMT_GBRAP10BE ||
         dstFormat == AV_PIX_FMT_GBRAP12LE || dstFormat == AV_PIX_FMT_GBRAP12BE ||
         dstFormat == AV_PIX_FMT_GBRAP14LE || dstFormat == AV_PIX_FMT_GBRAP14BE ||
         dstFormat == AV_PIX_FMT_GBRAP16LE || dstFormat == AV_PIX_FMT_GBRAP16BE ))
        c->convert_unscaled = Rgb16ToPlanarRgb16Wrapper;

    if (av_pix_fmt_desc_get(dstFormat)->comp[0].depth >= 10 &&
        isPlanarRGB(dstFormat) && !isFloat(dstFormat) &&
        (srcFormat == AV_PIX_FMT_X2RGB10LE || srcFormat == AV_PIX_FMT_X2BGR10LE))
        c->convert_unscaled = Rgb16ToPlanarRgb16Wrapper;

    if ((srcFormat == AV_PIX_FMT_GBRP9LE  || srcFormat == AV_PIX_FMT_GBRP9BE  ||
         srcFormat == AV_PIX_FMT_GBRP16LE || srcFormat == AV_PIX_FMT_GBRP16BE ||
         srcFormat == AV_PIX_FMT_GBRP10LE || srcFormat == AV_PIX_FMT_GBRP10BE ||
         srcFormat == AV_PIX_FMT_GBRP12LE || srcFormat == AV_PIX_FMT_GBRP12BE ||
         srcFormat == AV_PIX_FMT_GBRP14LE || srcFormat == AV_PIX_FMT_GBRP14BE ||
         srcFormat == AV_PIX_FMT_GBRAP10LE || srcFormat == AV_PIX_FMT_GBRAP10BE ||
         srcFormat == AV_PIX_FMT_GBRAP12LE || srcFormat == AV_PIX_FMT_GBRAP12BE ||
         srcFormat == AV_PIX_FMT_GBRAP14LE || srcFormat == AV_PIX_FMT_GBRAP14BE ||
         srcFormat == AV_PIX_FMT_GBRAP16LE || srcFormat == AV_PIX_FMT_GBRAP16BE) &&
        (dstFormat == AV_PIX_FMT_RGB48LE  || dstFormat == AV_PIX_FMT_RGB48BE  ||
         dstFormat == AV_PIX_FMT_BGR48LE  || dstFormat == AV_PIX_FMT_BGR48BE  ||
         dstFormat == AV_PIX_FMT_RGBA64LE || dstFormat == AV_PIX_FMT_RGBA64BE ||
         dstFormat == AV_PIX_FMT_BGRA64LE || dstFormat == AV_PIX_FMT_BGRA64BE))
        c->convert_unscaled = planarRgb16ToRgb16Wrapper;

    if (av_pix_fmt_desc_get(srcFormat)->comp[0].depth >= 10 &&
        isPlanarRGB(srcFormat) && !isFloat(srcFormat) &&
        (dstFormat == AV_PIX_FMT_X2RGB10LE || dstFormat == AV_PIX_FMT_X2BGR10LE))
        c->convert_unscaled = planarRgb16ToRgb16Wrapper;

    if (av_pix_fmt_desc_get(srcFormat)->comp[0].depth == 8 &&
        isPackedRGB(srcFormat) && dstFormat == AV_PIX_FMT_GBRP)
        c->convert_unscaled = rgbToPlanarRgbWrapper;

    if (av_pix_fmt_desc_get(srcFormat)->comp[0].depth == 8 &&
        isPackedRGB(srcFormat) && dstFormat == AV_PIX_FMT_GBRAP)
        c->convert_unscaled = rgbToPlanarRgbaWrapper;

    if (isBayer(srcFormat)) {
        c->dst_slice_align = 2;
        if (dstFormat == AV_PIX_FMT_RGB24)
            c->convert_unscaled = bayer_to_rgb24_wrapper;
        else if (dstFormat == AV_PIX_FMT_RGB48)
            c->convert_unscaled = bayer_to_rgb48_wrapper;
        else if (dstFormat == AV_PIX_FMT_YUV420P)
            c->convert_unscaled = bayer_to_yv12_wrapper;
        else if (!isBayer(dstFormat)) {
            av_log(c, AV_LOG_ERROR, "unsupported bayer conversion\n");
            av_assert0(0);
        }
    }

    /* bswap 16 bits per pixel/component packed formats */
    if (IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BAYER_BGGR16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BAYER_RGGB16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BAYER_GBRG16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BAYER_GRBG16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BGR444) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BGR48)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BGR555) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BGR565) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_BGRA64) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GRAY9)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GRAY10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GRAY12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GRAY14) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GRAY16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YA16)   ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_AYUV64) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP9)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP14) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP10MSB) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRP12MSB) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRAP10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRAP12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRAP14) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRAP16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_RGB444) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_RGB48)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_RGB555) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_RGB565) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_RGBA64) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_XV36)   ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_XV48)   ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_XYZ12)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV420P9)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV420P10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV420P12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV420P14) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV420P16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV422P9)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV422P10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV422P12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV422P14) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV422P16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV440P10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV440P12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P9)  ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P10) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P12) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P14) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P16) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P10MSB) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_YUV444P12MSB))
        c->convert_unscaled = bswap_16bpc;

    /* bswap 32 bits per pixel/component formats */
    if (IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRPF32) ||
        IS_DIFFERENT_ENDIANESS(srcFormat, dstFormat, AV_PIX_FMT_GBRAPF32))
        c->convert_unscaled = bswap_32bpc;

    if (usePal(srcFormat)) {
        switch (dstFormat) {
        case AV_PIX_FMT_GBRP:
        case AV_PIX_FMT_GBRAP:
            c->convert_unscaled = palToGbrpWrapper;
            break;
        default:
            if (isByteRGB(dstFormat))
                c->convert_unscaled = palToRgbWrapper;
            break;
        }
    }

    if (srcFormat == AV_PIX_FMT_YUV422P) {
        if (dstFormat == AV_PIX_FMT_YUYV422)
            c->convert_unscaled = yuv422pToYuy2Wrapper;
        else if (dstFormat == AV_PIX_FMT_UYVY422)
            c->convert_unscaled = yuv422pToUyvyWrapper;
    }

    /* uint Y to float Y */
    if (srcFormat == AV_PIX_FMT_GRAY8 && dstFormat == AV_PIX_FMT_GRAYF32){
        c->convert_unscaled = uint_y_to_float_y_wrapper;
    }

    /* float Y to uint Y */
    if (srcFormat == AV_PIX_FMT_GRAYF32 && dstFormat == AV_PIX_FMT_GRAY8){
        c->convert_unscaled = float_y_to_uint_y_wrapper;
    }

    /* LQ converters if -sws 0 or -sws 4*/
    if (c->opts.flags&(SWS_FAST_BILINEAR|SWS_POINT)) {
        /* yv12_to_yuy2 */
        if (srcFormat == AV_PIX_FMT_YUV420P || srcFormat == AV_PIX_FMT_YUVA420P) {
            if (dstFormat == AV_PIX_FMT_YUYV422)
                c->convert_unscaled = planarToYuy2Wrapper;
            else if (dstFormat == AV_PIX_FMT_UYVY422)
                c->convert_unscaled = planarToUyvyWrapper;
        }
    }
    if (srcFormat == AV_PIX_FMT_YUYV422 &&
       (dstFormat == AV_PIX_FMT_YUV420P || dstFormat == AV_PIX_FMT_YUVA420P))
        c->convert_unscaled = yuyvToYuv420Wrapper;
    if (srcFormat == AV_PIX_FMT_UYVY422 &&
       (dstFormat == AV_PIX_FMT_YUV420P || dstFormat == AV_PIX_FMT_YUVA420P))
        c->convert_unscaled = uyvyToYuv420Wrapper;
    if (srcFormat == AV_PIX_FMT_YUYV422 && dstFormat == AV_PIX_FMT_YUV422P)
        c->convert_unscaled = yuyvToYuv422Wrapper;
    if (srcFormat == AV_PIX_FMT_UYVY422 && dstFormat == AV_PIX_FMT_YUV422P)
        c->convert_unscaled = uyvyToYuv422Wrapper;
    if (dstFormat == AV_PIX_FMT_YUV420P &&
        (srcFormat == AV_PIX_FMT_NV24 || srcFormat == AV_PIX_FMT_NV42))
        c->convert_unscaled = nv24ToYuv420Wrapper;

#define isPlanarGray(x) (isGray(x) && (x) != AV_PIX_FMT_YA8 && (x) != AV_PIX_FMT_YA16LE && (x) != AV_PIX_FMT_YA16BE)

    /* simple copy */
    if ( srcFormat == dstFormat ||
        (srcFormat == AV_PIX_FMT_YUVA420P && dstFormat == AV_PIX_FMT_YUV420P) ||
        (srcFormat == AV_PIX_FMT_YUV420P && dstFormat == AV_PIX_FMT_YUVA420P) ||
        (isFloat(srcFormat) == isFloat(dstFormat) && isFloat16(srcFormat) == isFloat16(dstFormat)) && ((isPlanarYUV(srcFormat) && isPlanarGray(dstFormat)) ||
        (isPlanarYUV(dstFormat) && isPlanarGray(srcFormat)) ||
        (isPlanarGray(dstFormat) && isPlanarGray(srcFormat)) ||
        (isPlanarYUV(srcFormat) && isPlanarYUV(dstFormat) &&
         c->chrDstHSubSample == c->chrSrcHSubSample &&
         c->chrDstVSubSample == c->chrSrcVSubSample &&
         isSemiPlanarYUV(srcFormat) == isSemiPlanarYUV(dstFormat) &&
         isSwappedChroma(srcFormat) == isSwappedChroma(dstFormat))))
    {
        if (isPacked(c->opts.src_format))
            c->convert_unscaled = packedCopyWrapper;
        else /* Planar YUV or gray */
            c->convert_unscaled = planarCopyWrapper;
    }

#if ARCH_PPC
    ff_get_unscaled_swscale_ppc(c);
#elif ARCH_ARM
    ff_get_unscaled_swscale_arm(c);
#elif ARCH_AARCH64
    ff_get_unscaled_swscale_aarch64(c);
#endif
}

/* Convert the palette to the same packed 32-bit format as the palette */
void sws_convertPalette8ToPacked32(const uint8_t *src, uint8_t *dst,
                                   int num_pixels, const uint8_t *palette)
{
    int i;

    for (i = 0; i < num_pixels; i++)
        ((uint32_t *) dst)[i] = ((const uint32_t *) palette)[src[i]];
}

/* Palette format: ABCD -> dst format: ABC */
void sws_convertPalette8ToPacked24(const uint8_t *src, uint8_t *dst,
                                   int num_pixels, const uint8_t *palette)
{
    int i;

    for (i = 0; i < num_pixels; i++) {
        //FIXME slow?
        dst[0] = palette[src[i] * 4 + 0];
        dst[1] = palette[src[i] * 4 + 1];
        dst[2] = palette[src[i] * 4 + 2];
        dst += 3;
    }
}
