/*
 * Copyright (c) 2010 Bobby Bingham

 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation;
 * version 2 of the License.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

/**
 * @file
 * aspect ratio modification video filters
 */

#include "libavutil/mathematics.h"
#include "avfilter.h"

typedef struct {
    AVRational aspect;
} AspectContext;

static av_cold int init(AVFilterContext *ctx, const char *args, void *opaque)
{
    AspectContext *aspect = ctx->priv;
    double  ratio;
    int64_t gcd;
    char c = 0;

    if (args) {
        if (sscanf(args, "%d:%d%c", &aspect->aspect.num, &aspect->aspect.den, &c) != 2)
            if (sscanf(args, "%lf%c", &ratio, &c) == 1)
                aspect->aspect = av_d2q(ratio, 100);

        if (c || aspect->aspect.num <= 0 || aspect->aspect.den <= 0) {
            av_log(ctx, AV_LOG_ERROR,
                   "Invalid string '%s' for aspect ratio.\n", args);
            return AVERROR(EINVAL);
        }

        gcd = av_gcd(FFABS(aspect->aspect.num), FFABS(aspect->aspect.den));
        if (gcd) {
            aspect->aspect.num /= gcd;
            aspect->aspect.den /= gcd;
        }
    }

    if (aspect->aspect.den == 0)
        aspect->aspect = (AVRational) {0, 1};

    av_log(ctx, AV_LOG_INFO, "a:%d/%d\n", aspect->aspect.num, aspect->aspect.den);
    return 0;
}

/* for setdar filter, convert from frame aspect ratio to pixel aspect ratio */
static int setsar_config_props(AVFilterLink *inlink)
{
    AspectContext *aspect = inlink->dst->priv;

    inlink->sample_aspect_ratio = aspect->aspect;

    return 0;
}

AVFilter avfilter_vf_setsar = {
    .name      = "setsar",
    .description = NULL_IF_CONFIG_SMALL("Set the pixel sample aspect ratio."),

    .init      = init,

    .priv_size = sizeof(AspectContext),

    .inputs    = (AVFilterPad[]) {{ .name             = "default",
                                    .type             = AVMEDIA_TYPE_VIDEO,
                                    .config_props     = setsar_config_props,
                                    .get_video_buffer = avfilter_null_get_video_buffer,
                                    .start_frame      = avfilter_null_start_frame,
                                    .end_frame        = avfilter_null_end_frame },
                                  { .name = NULL}},

    .outputs   = (AVFilterPad[]) {{ .name             = "default",
                                    .type             = AVMEDIA_TYPE_VIDEO, },
                                  { .name = NULL}},
};
