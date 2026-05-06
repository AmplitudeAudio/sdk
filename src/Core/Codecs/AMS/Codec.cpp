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

#include <bit>
#include <cstring>
#include <limits>

#include <Core/Codecs/AMS/Codec.h>

using namespace SparkyStudios::Audio::Amplitude::Compression::ADPCM;

namespace SparkyStudios::Audio::Amplitude
{
    static constexpr bool kIsBigEndian = (std::endian::native == std::endian::big);

    namespace
    {
    void little_endian_to_native(void* data, const char* format)
    {
        if constexpr (!kIsBigEndian)
            return; // no-op on little-endian; the data is already in native order

        auto* cp = static_cast<unsigned char*>(data);
        const char* f = format;

        while (*f)
        {
            switch (*f)
            {
            case 'L':
                {
                    const AmInt32 temp = cp[0] + (static_cast<AmInt32>(cp[1]) << 8) + (static_cast<AmInt32>(cp[2]) << 16) +
                        (static_cast<AmInt32>(cp[3]) << 24);
                    std::memcpy(cp, &temp, sizeof(temp));
                    cp += 4;
                }
                break;

            case 'S':
                {
                    const auto temp = static_cast<AmInt16>(cp[0] + (cp[1] << 8));
                    std::memcpy(cp, &temp, sizeof(temp));
                    cp += 2;
                }
                break;

            default:
                if (isdigit(static_cast<unsigned char>(*f)))
                    cp += *f - '0';
                break;
            }

            ++f;
        }
    }

    void native_to_little_endian(void* data, const char* format)
    {
        if constexpr (!kIsBigEndian)
            return; // no-op on little-endian

        auto* cp = static_cast<unsigned char*>(data);
        const char* f = format;

        while (*f)
        {
            switch (*f)
            {
            case 'L':
                {
                    AmInt32 temp;
                    std::memcpy(&temp, cp, sizeof(temp));
                    *cp++ = static_cast<unsigned char>(temp);
                    *cp++ = static_cast<unsigned char>(temp >> 8);
                    *cp++ = static_cast<unsigned char>(temp >> 16);
                    *cp++ = static_cast<unsigned char>(temp >> 24);
                }
                break;

            case 'S':
                {
                    AmInt16 temp;
                    std::memcpy(&temp, cp, sizeof(temp));
                    *cp++ = static_cast<unsigned char>(temp);
                    *cp++ = static_cast<unsigned char>(temp >> 8);
                }
                break;

            default:
                if (isdigit(static_cast<unsigned char>(*f)))
                    cp += *f - '0';
                break;
            }

            ++f;
        }
    }

    /**
     * @brief Parses the RIFF/WAVE/IMA-ADPCM header from @p file and populates @p format and @p blockSize.
     *
     * @pre The file's read cursor must be positioned at byte 0 (start of RIFF header).
     * @post On success, @p format and @p blockSize are fully populated and the cursor sits at the first ADPCM data byte.
     *
     * @return @c true on success; @c false if the file is not a valid IMA-ADPCM WAV or contains unsupported parameters.
     */
    bool ReadHeader(std::shared_ptr<File> file, SoundFormat& format, AmUInt16& blockSize)
    {
        AmInt32 fmt = 0, bits_per_sample, sample_rate, num_channels;
        AmUInt32 fact_samples = 0;
        AmSize num_samples = 0;

        RIFFHeader riff_chunk_header;
        FMTHeader chunk_header;
        WAVEHeaderEx wave_header;

        // read initial RIFF form header

        if (file->Read(reinterpret_cast<AmUInt8Buffer>(&riff_chunk_header), sizeof(RIFFHeader)) != sizeof(RIFFHeader) ||
            std::strncmp(reinterpret_cast<const char*>(riff_chunk_header.chunkID), "RIFF", 4) != 0 ||
            std::strncmp(reinterpret_cast<const char*>(riff_chunk_header.chunkFormat), "WAVE", 4) != 0)
        {
            return false;
        }

        // loop through all elements of the RIFF wav header (until the data chunk)

        bool fmt_seen = false;
        while (true)
        {
            if (file->Read(reinterpret_cast<AmUInt8Buffer>(&chunk_header), sizeof(FMTHeader)) != sizeof(FMTHeader))
            {
                return false;
            }

            little_endian_to_native(&chunk_header, HEADER_FMT_CHUNK);

            // if it's the format chunk, we want to get some info out of there and
            // make sure it's a .wav file we can handle

            if (!std::strncmp(reinterpret_cast<const char*>(chunk_header.chunkID), "fmt ", 4))
            {
                bool supported = true;

                if (chunk_header.chunkSize < 16 || chunk_header.chunkSize > sizeof(WAVEHeaderEx) ||
                    file->Read(reinterpret_cast<AmUInt8Buffer>(&wave_header), chunk_header.chunkSize) != chunk_header.chunkSize)
                {
                    return false;
                }

                little_endian_to_native(&wave_header, HEADER_FMT_WAVE);

                fmt = (wave_header.head.audioFormat == WAVE_FORMAT_EXTENSIBLE && chunk_header.chunkSize == 40)
                    ? wave_header.subFormat
                    : wave_header.head.audioFormat;

                bits_per_sample = (chunk_header.chunkSize == 40 && wave_header.head.validBitsPerSample)
                    ? wave_header.head.validBitsPerSample
                    : wave_header.head.bitsPerSample;

                if (wave_header.head.numChannels < 1 || wave_header.head.numChannels > 2)
                {
                    // Only support up to 2 channels
                    supported = false;
                }
                else if (fmt == WAVE_FORMAT_PCM)
                {
                    // Only support ADPCM
                    supported = false;
                }
                else if (fmt == WAVE_FORMAT_IMA_ADPCM)
                {
                    if (bits_per_sample != 4)
                        supported = false; // Invalid ADPCM format

                    // IMA-ADPCM convention: validBitsPerSample stores samplesPerBlock (not bit depth).
                    if (wave_header.head.validBitsPerSample !=
                        (wave_header.head.blockAlign - wave_header.head.numChannels * 4) * (wave_header.head.numChannels ^ 3) + 1)
                    {
                        return false;
                    }
                }
                else
                {
                    // Unknown format
                    supported = false;
                }

                if (!supported)
                {
                    return false;
                }

                fmt_seen = true;
            }
            else if (!std::strncmp(reinterpret_cast<const char*>(chunk_header.chunkID), "fact", 4))
            {
                if (chunk_header.chunkSize < 4 ||
                    file->Read(reinterpret_cast<AmUInt8Buffer>(&fact_samples), sizeof(fact_samples)) != sizeof(fact_samples))
                {
                    return false;
                }

                if (chunk_header.chunkSize > 4)
                {
                    file->Seek(static_cast<AmInt64>(chunk_header.chunkSize - 4), eFileSeekOrigin_Current);
                }
            }
            else if (!std::strncmp(reinterpret_cast<const char*>(chunk_header.chunkID), "data", 4))
            {
                // on the data chunk, get size and exit parsing loop

                if (!fmt_seen)
                    return false;

                if (!chunk_header.chunkSize)
                {
                    return false;
                }

                if (fmt == WAVE_FORMAT_PCM)
                {
                    if (chunk_header.chunkSize % wave_header.head.blockAlign)
                    {
                        return false;
                    }

                    num_samples = chunk_header.chunkSize / wave_header.head.blockAlign;
                }
                else
                {
                    const int complete_blocks = chunk_header.chunkSize / wave_header.head.blockAlign;
                    const int leftover_bytes = chunk_header.chunkSize % wave_header.head.blockAlign;
                    int samples_last_block;

                    num_samples = complete_blocks * wave_header.head.validBitsPerSample;

                    if (leftover_bytes)
                    {
                        if (leftover_bytes % (wave_header.head.numChannels * 4))
                        {
                            return false;
                        }

                        samples_last_block = (leftover_bytes - (wave_header.head.numChannels * 4)) * (wave_header.head.numChannels ^ 3) + 1;
                        num_samples += samples_last_block;
                    }
                    else
                    {
                        samples_last_block = wave_header.head.validBitsPerSample;
                    }

                    if (fact_samples)
                    {
                        if (fact_samples < num_samples && fact_samples > num_samples - samples_last_block)
                        {
                            num_samples = fact_samples;
                        }
                        else if (
                            wave_header.head.numChannels == 2 && (fact_samples >>= 1) < num_samples &&
                            fact_samples > num_samples - samples_last_block)
                        {
                            num_samples = fact_samples;
                        }
                    }
                }

                if (!num_samples)
                {
                    return false;
                }

                num_channels = wave_header.head.numChannels;
                sample_rate = wave_header.head.sampleRate;
                break;
            }
            else
            {
                const auto bytes_to_eat = static_cast<AmInt64>((chunk_header.chunkSize + 1) & ~1L);
                file->Seek(bytes_to_eat, eFileSeekOrigin_Current);
            }
        }

        format.SetAll(sample_rate, num_channels, bits_per_sample, num_samples, num_channels * sizeof(AmInt16), eAudioSampleFormat_Int16);

        blockSize = wave_header.head.blockAlign;

        return true;
    }

    /**
     * @brief Writes the RIFF/WAVE/IMA-ADPCM header to @p file based on @p format and @p samplesPerBlock.
     *
     * @pre @p file must be open for writing and positioned at byte 0.
     * @pre @p format must have valid channel count (1 or 2), sample rate, and frame count set.
     * @post On success, the complete ADPCMHeader is written and the cursor sits at the first data byte position.
     *
     * @return @c true if the header was written successfully; @c false if the output would exceed RIFF 32-bit size limits.
     */
    bool WriteHeader(std::shared_ptr<File> file, SoundFormat& format, AmUInt32 samplesPerBlock)
    {
        ADPCMHeader header{};

        // numChannels XOR 3 yields 2 for mono, 1 for stereo — divisor in IMA samplesPerBlock formula.
        const AmInt32 blockSize = (samplesPerBlock - 1) / (format.GetNumChannels() ^ 3) + (format.GetNumChannels() * 4);
        const AmSize numBlocks = format.GetFramesCount() / samplesPerBlock;
        const AmInt32 leftOverSamples = format.GetFramesCount() % samplesPerBlock;
        AmSize totalDataBytes = numBlocks * blockSize;

        if (leftOverSamples)
        {
            const AmInt32 lastBlockSamples = ((leftOverSamples + 6) & ~7) + 1;
            const AmInt32 lastBlockSize = (lastBlockSamples - 1) / (format.GetNumChannels() ^ 3) + (format.GetNumChannels() * 4);
            totalDataBytes += lastBlockSize;
        }

        const AmSize chunkSize = sizeof(RIFFHeader) + sizeof(WAVEHeader) + sizeof(DATAHeader) + totalDataBytes;
        if (chunkSize > std::numeric_limits<AmUInt32>::max() || totalDataBytes > std::numeric_limits<AmUInt32>::max())
        {
            amLogError("AMS codec: file too large for RIFF/WAV 32-bit size fields");
            return false;
        }

        // ========== RIFF HEADER
        header.riff.chunkSize = sizeof(RIFFHeader) + sizeof(WAVEHeader) + sizeof(DATAHeader) + totalDataBytes;

        // ========== FORMAT HEADER
        header.fmt.chunkSize = sizeof(WAVEHeader);

        // ========== WAVE HEADER
        header.wave.audioFormat = WAVE_FORMAT_IMA_ADPCM;
        header.wave.numChannels = format.GetNumChannels();
        header.wave.sampleRate = format.GetSampleRate();
        header.wave.byteRate = format.GetSampleRate() * blockSize / samplesPerBlock;
        header.wave.blockAlign = blockSize;
        header.wave.bitsPerSample = 4; // <- 4 for ADPCM
        header.wave.extendedSize = 2;
        // IMA-ADPCM repurposes validBitsPerSample to store samplesPerBlock.
        header.wave.validBitsPerSample = samplesPerBlock;

        // ========== FACT HEADER
        header.fact.totalSamples = format.GetFramesCount();
        header.fact.chunkSize = 4;

        // ========== DATA HEADER
        header.data.chunkSize = totalDataBytes;

        // write the RIFF chunks up to just before the data starts

        native_to_little_endian(&header.riff, HEADER_FMT_CHUNK);
        native_to_little_endian(&header.fmt, HEADER_FMT_CHUNK);
        native_to_little_endian(&header.wave, HEADER_FMT_WAVE);
        native_to_little_endian(&header.fact, HEADER_FMT_FACT);
        native_to_little_endian(&header.data, HEADER_FMT_CHUNK);

        return file->Write(reinterpret_cast<AmConstUInt8Buffer>(&header), sizeof(header));
    }

    /**
     * @brief Encodes PCM audio from @p in into IMA-ADPCM blocks and writes them to @p file.
     *
     * @pre @p in must have at least @p length frames and @p format.GetNumChannels() channels.
     * @pre @p file must be open for writing and positioned at the start of the data chunk.
     * @post On success, all encoded blocks are written to @p file.
     *
     * @return The number of PCM frames successfully encoded and written, or 0 on allocation or write failure.
     */
    AmUInt64 Encode(
        std::shared_ptr<File> file,
        SoundFormat& format,
        AudioBuffer* in,
        AmUInt64 length,
        AmUInt32 samplesPerBlock,
        int lookAhead,
        NoiseShapingMode noiseShaping)
    {
        const AmUInt32 numChannels = format.GetNumChannels();
        AmUInt32 blockSize = (samplesPerBlock - 1) / (numChannels ^ 3) + (numChannels * 4);

        ScopedMemoryAllocation adpcm_block(eMemoryPoolKind_Codec, blockSize, __FILE__, __LINE__);

        std::shared_ptr<Context> ctx = nullptr;

        if (!adpcm_block.Address())
            return 0;

        AmUInt64 maxSamplesNeeded = length;
        const AmUInt64 lastBlockSamples = length % samplesPerBlock;
        if (lastBlockSamples > 0)
        {
            const AmUInt32 lastBlockAdpcmSamples = ((lastBlockSamples + 6) & ~7) + 1;
            maxSamplesNeeded += (lastBlockAdpcmSamples - lastBlockSamples);
        }

        ScopedMemoryAllocation input16(eMemoryPoolKind_Codec, maxSamplesNeeded * numChannels * sizeof(AmInt16), __FILE__, __LINE__);
        auto* input16_buffer = input16.PointerOf<AmInt16>();

        if (!input16.Address())
            return 0;

        if (numChannels == 1)
        {
            const auto& ch0 = in->GetChannel(0);
            for (AmUInt64 i = 0; i < length; i++)
                input16_buffer[i] = AmReal32ToInt16(ch0[i], true);
        }
        else
        {
            const auto& ch0 = in->GetChannel(0);
            const auto& ch1 = in->GetChannel(1);
            for (AmUInt64 i = 0; i < length; i++)
            {
                input16_buffer[i * 2]     = AmReal32ToInt16(ch0[i], true);
                input16_buffer[i * 2 + 1] = AmReal32ToInt16(ch1[i], true);
            }
        }

        AmUInt64 offset = 0;
        while (length)
        {
            AmUInt32 this_block_adpcm_samples = samplesPerBlock;
            AmUInt32 this_block_pcm_samples = samplesPerBlock;
            AmSize num_bytes = 0;

            if (this_block_pcm_samples > length)
            {
                this_block_adpcm_samples = ((length + 6) & ~7) + 1;
                blockSize = (this_block_adpcm_samples - 1) / (numChannels ^ 3) + (numChannels * 4);
                this_block_pcm_samples = static_cast<AmUInt32>(length);
            }

            AmInt16Buffer pcm_block = input16_buffer + offset * numChannels;

            // TODO: big-endian encoder path needs audit if a BE platform is supported

            // if this is the last block, and it's not full, duplicate the last sample(s) so we don't
            // create problems for the lookAhead

            if (this_block_adpcm_samples > this_block_pcm_samples)
            {
                AmInt16* dst = pcm_block + this_block_pcm_samples * numChannels;
                AmInt16* src = dst - numChannels;
                int dups = static_cast<int>((this_block_adpcm_samples - this_block_pcm_samples) * numChannels);

                while (dups--)
                {
                    *dst++ = *src++;
                }
            }

            // if this is the first block, compute a decaying average (in reverse) so that we can let the
            // encoder know what kind of initial deltas to expect (helps to initialize index)

            if (ctx == nullptr)
            {
                AmInt32 average_deltas[2];

                average_deltas[0] = average_deltas[1] = 0;

                for (AmUInt32 i = this_block_adpcm_samples * numChannels; i -= numChannels;)
                {
                    average_deltas[0] -= average_deltas[0] >> 3;
                    average_deltas[0] += std::abs(static_cast<AmInt32>(pcm_block[i]) - pcm_block[i - numChannels]);

                    if (numChannels == 2)
                    {
                        average_deltas[1] -= average_deltas[1] >> 3;
                        average_deltas[1] += std::abs(static_cast<AmInt32>(pcm_block[i - 1]) - pcm_block[i + 1]);
                    }
                }

                average_deltas[0] >>= 3;
                average_deltas[1] >>= 3;

                ctx = CreateContext(numChannels, lookAhead, noiseShaping, average_deltas);
            }

            Compress(ctx.get(), adpcm_block.PointerOf<AmUInt8>(), num_bytes, pcm_block, this_block_adpcm_samples);

            if (num_bytes != blockSize)
                return 0;

            if (file->Write(adpcm_block.PointerOf<AmUInt8>(), blockSize) != blockSize)
                return 0;

            length -= this_block_pcm_samples;
            offset += this_block_pcm_samples;
        }

        return offset;
    }
    } // anonymous namespace

    AMSCodec::AMSCodec()
        : Codec("ams")
    {}

    bool AMSCodec::AMSDecoder::Open(std::shared_ptr<File> file)
    {
        _file = file;

        if (!ReadHeader(_file, m_format, _blockSize))
        {
            amLogError("The AMS codec cannot handle the file: '" AM_OS_CHAR_FMT "'", file->GetPath().c_str());
            return false;
        }

        const AmUInt32 numChannels = m_format.GetNumChannels();
        _samplesPerBlock = (_blockSize - numChannels * 4) * (numChannels ^ 3) + 1;

        _adpcmBlockBuffer = ScopedMemoryAllocation(eMemoryPoolKind_Codec, _blockSize, __FILE__, __LINE__);
        _pcmBlockBuffer =
            ScopedMemoryAllocation(eMemoryPoolKind_Codec, _samplesPerBlock * numChannels * sizeof(AmInt16), __FILE__, __LINE__);

        if (!_adpcmBlockBuffer.Address() || !_pcmBlockBuffer.Address())
        {
            amLogError("The AMS codec failed to allocate decode buffers for: '" AM_OS_CHAR_FMT "'", file->GetPath().c_str());
            return false;
        }

        _initialized = true;

        return true;
    }

    bool AMSCodec::AMSDecoder::Close()
    {
        if (_initialized)
        {
            _file.reset();

            _adpcmBlockBuffer = ScopedMemoryAllocation();
            _pcmBlockBuffer = ScopedMemoryAllocation();

            m_format = SoundFormat();
            _samplesPerBlock = 0;
            _initialized = false;
        }

        // true because it is already closed
        return true;
    }

    AmUInt64 AMSCodec::AMSDecoder::Load(AudioBuffer* out)
    {
        if (!_initialized)
            return 0;

        return Stream(out, 0, 0, m_format.GetFramesCount());
    }

    AmUInt64 AMSCodec::AMSDecoder::Stream(AudioBuffer* out, AmUInt64 bufferOffset, AmUInt64 seekOffset, AmUInt64 length)
    {
        if (!_initialized)
            return 0;

        if (!Seek(seekOffset))
            return 0;

        const AmUInt32 numChannels = m_format.GetNumChannels();
        auto* adpcm_block = _adpcmBlockBuffer.PointerOf<AmUInt8>();
        auto* pcm_block = _pcmBlockBuffer.PointerOf<AmInt16>();

        // Cache channel views once — avoids repeated GetChannel() lookups in the inner loop.
        AmReal32* ch0 = out->GetChannel(0).begin();
        AmReal32* ch1 = numChannels == 2 ? out->GetChannel(1).begin() : nullptr;

        // The offset within the first block to start reading from.
        // Seek() has positioned the file at the start of the block containing seekOffset.
        AmUInt64 startSampleInBlock = seekOffset % _samplesPerBlock;
        AmUInt64 samplesDecoded = 0;

        while (samplesDecoded < length)
        {
            const AmSize bytesRead = _file->Read(adpcm_block, _blockSize);
            if (bytesRead == 0)
                break; // EOF

            const AmInt32 decompressedSamples = Decompress(pcm_block, adpcm_block, bytesRead, numChannels);
            if (decompressedSamples <= 0)
                break;

            if (startSampleInBlock >= static_cast<AmUInt64>(decompressedSamples))
                break;

            const AmUInt64 availableFromBlock = static_cast<AmUInt64>(decompressedSamples) - startSampleInBlock;
            const AmUInt64 samplesToTake = std::min(availableFromBlock, length - samplesDecoded);

            // Deinterleave Int16 PCM into per-channel Float32.
            if (numChannels == 1)
            {
                for (AmUInt64 i = 0; i < samplesToTake; i++)
                    ch0[bufferOffset + samplesDecoded + i] = AmInt16ToReal32(pcm_block[startSampleInBlock + i]);
            }
            else
            {
                for (AmUInt64 i = 0; i < samplesToTake; i++)
                {
                    const AmUInt64 base = (startSampleInBlock + i) * 2;
                    ch0[bufferOffset + samplesDecoded + i] = AmInt16ToReal32(pcm_block[base]);
                    ch1[bufferOffset + samplesDecoded + i] = AmInt16ToReal32(pcm_block[base + 1]);
                }
            }

            samplesDecoded += samplesToTake;

            startSampleInBlock = 0;
        }

        return samplesDecoded;
    }

    bool AMSCodec::AMSDecoder::Seek(AmUInt64 offset)
    {
        if (!_initialized || _samplesPerBlock == 0)
            return false;

        const AmUInt32 steps = static_cast<AmUInt32>(offset / _samplesPerBlock);
        const AmUInt64 fileOffset = static_cast<AmUInt64>(steps) * _blockSize;
        _file->Seek(static_cast<AmInt64>(sizeof(ADPCMHeader) + fileOffset), eFileSeekOrigin_Start);

        return _file->Position() == sizeof(ADPCMHeader) + fileOffset;
    }

    bool AMSCodec::AMSEncoder::Open(std::shared_ptr<File> file)
    {
        _file = file;

        if (!WriteHeader(_file, m_format, _samplesPerBlock))
        {
            amLogError("The AMS codec was unable to write the file: '" AM_OS_CHAR_FMT "'", file->GetPath().c_str());
            return false;
        }

        _initialized = true;
        return true;
    }

    bool AMSCodec::AMSEncoder::Close()
    {
        if (_initialized)
        {
            _file.reset();

            m_format = SoundFormat();
            _initialized = false;
        }

        return true;
    }

    AmUInt64 AMSCodec::AMSEncoder::Write(AudioBuffer* in, AmUInt64 offset, AmUInt64 length)
    {
        if (!_initialized)
            return 0;

        AMPLITUDE_ASSERT(offset == 0 && "AMSEncoder::Write currently only supports offset == 0; partial writes are not implemented");
        _file->Seek(static_cast<AmInt64>(sizeof(ADPCMHeader)), eFileSeekOrigin_Start);
        return Encode(_file, m_format, in, length, _samplesPerBlock, _lookAhead, _noiseShaping);
    }

    void AMSCodec::AMSEncoder::SetEncodingParams(
        AmUInt32 samplesPerBlock, AmUInt32 lookAhead, NoiseShapingMode noiseShaping)
    {
        // _blockSize is intentionally not stored here; WriteHeader recomputes it from _samplesPerBlock.
        _samplesPerBlock = samplesPerBlock;
        _lookAhead = lookAhead;
        _noiseShaping = noiseShaping;
    }

    std::shared_ptr<Codec::Decoder> AMSCodec::CreateDecoder()
    {
        return ampoolshared(eMemoryPoolKind_Codec, AMSDecoder, this);
    }

    std::shared_ptr<Codec::Encoder> AMSCodec::CreateEncoder()
    {
        return ampoolshared(eMemoryPoolKind_Codec, AMSEncoder, this);
    }

    bool AMSCodec::CanHandleFile(std::shared_ptr<File> file) const
    {
        if (!file)
            return false;

        const auto pos = file->Position();
        file->Seek(0, eFileSeekOrigin_Start);

        SoundFormat tempFormat;
        AmUInt16 tempBlockSize = 0;
        const bool result = ReadHeader(file, tempFormat, tempBlockSize);

        file->Seek(static_cast<AmInt64>(pos), eFileSeekOrigin_Start);

        return result;
    }
} // namespace SparkyStudios::Audio::Amplitude
