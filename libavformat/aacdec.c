/*
 * raw ADTS AAC demuxer
 * Copyright (c) 2008 Michael Niedermayer <michaelni@gmx.at>
 * Copyright (c) 2009 Robert Swain ( rob opendot cl )
 *
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

#include "libavutil/intreadwrite.h"
#include "avformat.h"
#include "rawdec.h"
#include "id3v1.h"


static int adts_aac_probe(AVProbeData *p)
{
    int max_frames = 0, first_frames = 0;
    int fsize, frames;
    uint8_t *buf0 = p->buf;
    uint8_t *buf2;
    uint8_t *buf;
    uint8_t *end = buf0 + p->buf_size - 7;

    buf = buf0;

    for(; buf < end; buf= buf2+1) {
        buf2 = buf;

        for(frames = 0; buf2 < end; frames++) {
            uint32_t header = AV_RB16(buf2);
            if((header&0xFFF6) != 0xFFF0)
                break;
            fsize = (AV_RB32(buf2 + 3) >> 13) & 0x1FFF;
            if(fsize < 7)
                break;
            buf2 += fsize;
        }
        max_frames = FFMAX(max_frames, frames);
        if(buf == buf0)
            first_frames= frames;
    }
    if   (first_frames>=3) return AVPROBE_SCORE_MAX/2+1;
    else if(max_frames>500)return AVPROBE_SCORE_MAX/2;
    else if(max_frames>=3) return AVPROBE_SCORE_MAX/4;
    else if(max_frames>=1) return 1;
    else                   return 0;
}

static int adts_aac_read_header(AVFormatContext *s,
                                AVFormatParameters *ap)
{
    AVStream *st;

    st = av_new_stream(s, 0);
    if (!st)
        return AVERROR(ENOMEM);

    st->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    st->codec->codec_id = s->iformat->value;
    st->need_parsing = AVSTREAM_PARSE_FULL;

    ff_id3v1_read(s);

    //LCM of all possible ADTS sample rates
    av_set_pts_info(st, 64, 1, 28224000);

    return 0;
}

AVInputFormat ff_aac_demuxer = {
    .name           = "aac",
    .long_name      = NULL_IF_CONFIG_SMALL("raw ADTS AAC"),
    .read_probe     = adts_aac_probe,
    .read_header    = adts_aac_read_header,
    .read_packet    = ff_raw_read_partial_packet,
    .flags= AVFMT_GENERIC_INDEX,
    .extensions = "aac",
    .value = CODEC_ID_AAC,
};
