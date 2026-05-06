// Copyright (c) 2021-present Sparky Studios. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Based on ADPCM-XQ, Copyright (c) 2015 David Bryant.
// https://github.com/dbry/adpcm-xq

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>

#include <Utils/Audio/Compression/ADPCM/ADPCM.h>

namespace SparkyStudios::Audio::Amplitude::Compression::ADPCM
{
    /********************************* 4-bit ADPCM encoder ********************************/

    /* step table */
    static const AmUInt16 stepTable[89] = { 7,     8,     9,     10,    11,    12,    13,    14,    16,    17,    19,   21,    23,
                                            25,    28,    31,    34,    37,    41,    45,    50,    55,    60,    66,   73,    80,
                                            88,    97,    107,   118,   130,   143,   157,   173,   190,   209,   230,  253,   279,
                                            307,   337,   371,   408,   449,   494,   544,   598,   658,   724,   796,  876,   963,
                                            1060,  1166,  1282,  1411,  1552,  1707,  1878,  2066,  2272,  2499,  2749, 3024,  3327,
                                            3660,  4026,  4428,  4871,  5358,  5894,  6484,  7132,  7845,  8630,  9493, 10442, 11487,
                                            12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767 };

    /* step index tables */
    static const AmInt32 indexTable[] = {
        /* adpcm data size is 4 */
        -1, -1, -1, -1, 2, 4, 6, 8
    };

    static inline AmInt32 decodeNibble(int nibble, int step)
    {
        AmInt32 delta = step >> 3;
        delta += (nibble & 1) ? (step >> 2) : 0;
        delta += (nibble & 2) ? (step >> 1) : 0;
        delta += (nibble & 4) ? step : 0;
        return (nibble & 8) ? -delta : delta;
    }

    static void set_decode_parameters(Context* ctx, AmConstInt32Buffer init_pcmdata, AmConstInt8Buffer init_index)
    {
        for (int ch = 0; ch < ctx->numChannels; ch++)
        {
            ctx->channels[ch].pcmData = init_pcmdata[ch];
            ctx->channels[ch].index = init_index[ch];
        }
    }

    static AmUInt64 minimum_error(
        const Channel* pchan, int nch, AmInt32 csample, AmConstInt16Buffer sample, int depth, int* best_nibble, AmUInt64 max_error)
    {
        const AmInt32 delta = csample - pchan->pcmData;
        Channel chan = *pchan;
        const int step = stepTable[chan.index];
        int nibble;

        if (delta < 0)
        {
            const int mag = (-delta << 2) / step;
            nibble = 0x8 | (mag > 7 ? 7 : mag);
        }
        else
        {
            const int mag = (delta << 2) / step;
            nibble = mag > 7 ? 7 : mag;
        }

        chan.pcmData += decodeNibble(nibble, step);
        chan.pcmData = std::clamp(chan.pcmData, -32768, 32767);
        if (best_nibble)
            *best_nibble = nibble;
        auto min_error = static_cast<AmUInt64>(chan.pcmData - csample) * static_cast<AmUInt64>(chan.pcmData - csample);

        if (!depth || min_error >= max_error)
            return min_error;

        chan.index += indexTable[nibble & 0x07];
        chan.index = std::clamp(chan.index, static_cast<AmInt8>(0), static_cast<AmInt8>(88));
        min_error += minimum_error(&chan, nch, sample[nch], sample + nch, depth - 1, nullptr, max_error - min_error);

        for (int nibble2 = 0; nibble2 <= 0xF; ++nibble2)
        {
            if (nibble2 == nibble)
                continue;

            // Skip nibbles whose signed delta is too far from the initial estimate.
            const int d1 = nibble < 8 ? nibble + 1 : 7 - nibble;
            const int d2 = nibble2 < 8 ? nibble2 + 1 : 7 - nibble2;
            if ((nibble2 & 0x7) != 0x7 && std::abs(d1 - d2) > 3)
                continue;

            chan = *pchan;
            chan.pcmData += decodeNibble(nibble2, step);
            chan.pcmData = std::clamp(chan.pcmData, -32768, 32767);

            auto error = static_cast<AmUInt64>(chan.pcmData - csample) * static_cast<AmUInt64>(chan.pcmData - csample);
            const AmUInt64 threshold = max_error < min_error ? max_error : min_error;

            if (error < threshold)
            {
                chan.index += indexTable[nibble2 & 0x07];
                chan.index = std::clamp(chan.index, static_cast<AmInt8>(0), static_cast<AmInt8>(88));
                error += minimum_error(&chan, nch, sample[nch], sample + nch, depth - 1, nullptr, threshold - error);

                if (error < min_error)
                {
                    if (best_nibble)
                        *best_nibble = nibble2;
                    min_error = error;
                }
            }
        }

        return min_error;
    }

    static uint8_t encode_sample(Context* ctx, int ch, const AmInt16* sample, int num_samples)
    {
        Channel* pchan = ctx->channels + ch;
        AmInt32 csample = *sample;
        int depth = num_samples - 1, nibble;
        int step = stepTable[pchan->index];

        if (ctx->noiseShaping == eNSM_DYNAMIC)
        {
            AmInt32 sam = (3 * pchan->history[0] - pchan->history[1]) >> 1;
            AmInt32 temp = csample - (((pchan->weight * sam) + 512) >> 10);
            AmInt32 shaping_weight;

            if (sam && temp)
                pchan->weight -= (((sam ^ temp) >> 29) & 4) - 2;
            pchan->history[1] = pchan->history[0];
            pchan->history[0] = csample;

            shaping_weight = (pchan->weight < 256) ? 1024 : 1536 - (pchan->weight * 2);
            temp = -((shaping_weight * pchan->error + 512) >> 10);

            if (shaping_weight < 0 && temp)
            {
                if (temp == pchan->error)
                    temp = (temp < 0) ? temp + 1 : temp - 1;

                pchan->error = -csample;
                csample += temp;
            }
            else
                pchan->error = -(csample += temp);
        }
        else if (ctx->noiseShaping == eNSM_STATIC)
        {
            pchan->error = -(csample -= pchan->error);
        }

        if (depth > ctx->lookAhead)
            depth = ctx->lookAhead;

        minimum_error(pchan, ctx->numChannels, csample, sample, depth, &nibble, UINT64_MAX);

        pchan->pcmData += decodeNibble(nibble, step);
        pchan->index += indexTable[nibble & 0x07];
        pchan->index = std::clamp(pchan->index, static_cast<AmInt8>(0), static_cast<AmInt8>(88));
        pchan->pcmData = std::clamp(pchan->pcmData, -32768, 32767);

        if (ctx->noiseShaping)
            pchan->error += pchan->pcmData;

        return nibble;
    }

    static void encode_chunks(Context* ctx, uint8_t** outbuf, size_t& outbufsize, const AmInt16** inbuf, int inbufcount)
    {
        const AmInt16* pcmbuf;
        int chunks, ch, i;

        chunks = (inbufcount - 1) / 8;
        outbufsize += (chunks * 4) * ctx->numChannels;

        while (chunks--)
        {
            for (ch = 0; ch < ctx->numChannels; ch++)
            {
                pcmbuf = *inbuf + ch;

                for (i = 0; i < 4; i++)
                {
                    **outbuf = encode_sample(ctx, ch, pcmbuf, chunks * 8 + (3 - i) * 2 + 2);
                    pcmbuf += ctx->numChannels;
                    **outbuf |= encode_sample(ctx, ch, pcmbuf, chunks * 8 + (3 - i) * 2 + 1) << 4;
                    pcmbuf += ctx->numChannels;
                    (*outbuf)++;
                }
            }

            *inbuf += 8 * ctx->numChannels;
        }
    }

    /**
     * @brief Creates an ADPCM codec context with the given parameters.
     *
     * @param numChannels    Number of audio channels (1 or 2).
     * @param lookAhead      Encoder look-ahead depth (0 = no look-ahead).
     * @param noiseShaping   Noise-shaping mode applied during encoding.
     * @param initialDeltas  Pointer to a 2-element array of initial step deltas used to
     *                       seed the step-index table.  The second element is read even
     *                       when @p numChannels is 1 — it is simply ignored in that case.
     *                       Passing a single-element array is undefined behaviour.
     *
     * @return A shared pointer to the newly allocated @ref Context.
     */
    std::shared_ptr<Context> CreateContext(int numChannels, int lookAhead, NoiseShapingMode noiseShaping, AmInt32 initialDeltas[2])
    {
        auto ctx = ampoolshared(eMemoryPoolKind_Codec, Context);
        int ch, i;

        std::memset(ctx.get(), 0, sizeof(Context));
        ctx->noiseShaping = noiseShaping;
        ctx->numChannels = numChannels;
        ctx->lookAhead = lookAhead;

        // given the supplied initial deltas, search for and store the closest index

        for (ch = 0; ch < numChannels; ++ch)
        {
            for (i = 0; i <= 88; i++)
            {
                if (i == 88 || initialDeltas[ch] < (static_cast<AmInt32>(stepTable[i]) + stepTable[i + 1]) / 2)
                {
                    ctx->channels[ch].index = i;
                    break;
                }
            }
        }

        return ctx;
    }

    /**
     * @brief Encodes PCM samples into ADPCM.
     *
     * @param ctx         Encoder context created with @ref CreateContext.
     * @param out         Output buffer.  Caller must provide at least
     *                    @c (sampleCount-1)/(channels^3) + channels*4 bytes.
     * @param outSize     Receives the number of bytes written to @p out.
     * @param in          Interleaved signed 16-bit PCM input samples.
     * @param sampleCount Total number of PCM samples across all channels.
     *
     * @return @c true on success.
     */
    bool Compress(Context* ctx, AmUInt8Buffer out, AmSize& outSize, AmConstInt16Buffer in, AmSize sampleCount)
    {
        AmInt32 init_pcmdata[2];
        AmInt8 init_index[2];

        outSize = 0;

        if (!sampleCount)
            return true;

        for (int ch = 0; ch < ctx->numChannels; ch++)
            init_index[ch] = ctx->channels[ch].index;

        for (int ch = 0; ch < ctx->numChannels; ch++)
        {
            init_pcmdata[ch] = *in++;
            out[0] = init_pcmdata[ch];
            out[1] = init_pcmdata[ch] >> 8;
            out[2] = init_index[ch];
            out[3] = 0;

            out += 4;
            outSize += 4;
        }

        set_decode_parameters(ctx, init_pcmdata, init_index);
        encode_chunks(ctx, &out, outSize, &in, sampleCount);

        return true;
    }

    /**
     * @brief Decodes an ADPCM block into signed 16-bit PCM samples.
     *
     * @param out      Output buffer for decoded PCM.  Caller must provide at least
     *                 @c ((inSize - channels*4) / (channels*4)) * 8 * channels + channels
     *                 int16 slots (i.e. samplesPerBlock * channels worst-case).
     * @param in       Raw ADPCM input block (header + encoded nibble data).
     * @param inSize   Size in bytes of the input block.
     * @param channels Number of interleaved audio channels (1 or 2).
     *
     * @return Total number of PCM samples decoded (across all channels), or 0 on error.
     */
    AmInt32 Decompress(AmInt16Buffer out, AmConstUInt8Buffer in, AmSize inSize, AmUInt32 channels)
    {
        AmUInt32 ch, samples = 1, chunks;
        AmInt32 pcmData[2];
        AmInt8 index[2];

        if (inSize < static_cast<AmSize>(channels) * 4)
            return 0;

        for (ch = 0; ch < channels; ch++)
        {
            *out++ = pcmData[ch] = static_cast<AmInt16>(in[0] | (in[1] << 8));
            index[ch] = in[2];

            if (index[ch] < 0 || index[ch] > 88 || in[3]) // sanitize the input a little...
                return 0;

            inSize -= 4;
            in += 4;
        }

        chunks = inSize / (channels * 4);
        samples += chunks * 8;

        while (chunks--)
        {
            AmUInt32 ch, i;

            for (ch = 0; ch < channels; ++ch)
            {
                for (i = 0; i < 4; ++i)
                {
                    const int low  = *in & 0xF;
                    const int high = (*in >> 4) & 0xF;

                    int step = stepTable[index[ch]];
                    pcmData[ch] += decodeNibble(low, step);
                    index[ch]   += indexTable[low & 0x7];
                    index[ch]   = std::clamp(index[ch], static_cast<AmInt8>(0), static_cast<AmInt8>(88));
                    pcmData[ch] = std::clamp(pcmData[ch], -32768, 32767);
                    out[i * 2 * channels] = pcmData[ch];

                    step = stepTable[index[ch]];
                    pcmData[ch] += decodeNibble(high, step);
                    index[ch]   += indexTable[high & 0x7];
                    index[ch]   = std::clamp(index[ch], static_cast<AmInt8>(0), static_cast<AmInt8>(88));
                    pcmData[ch] = std::clamp(pcmData[ch], -32768, 32767);
                    out[(i * 2 + 1) * channels] = pcmData[ch];

                    in++;
                }

                out++;
            }

            out += channels * 7;
        }

        return samples;
    }
} // namespace SparkyStudios::Audio::Amplitude::Compression::ADPCM
