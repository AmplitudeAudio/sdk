// Copyright (c) 2026-present Sparky Studios. All rights reserved.
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

#include <Fidelity/AssetGenerator.h>

#include <charconv>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include <Fidelity/Wav.h>

namespace SparkyStudios::Audio::Amplitude::Fidelity
{
    namespace
    {
        constexpr std::uint64_t kFirstSoundId = 910000;
        constexpr std::uint64_t kSoundBankId = 910;
        constexpr std::uint64_t kGlideRtpcId = 9101;
        constexpr std::uint64_t kSpreadRtpcId = 9102;

        bool WriteIfChanged(const std::filesystem::path& path, std::string_view content)
        {
            {
                std::ifstream existing(path, std::ios::binary);
                if (existing)
                {
                    const std::string current((std::istreambuf_iterator<char>(existing)), std::istreambuf_iterator<char>());
                    if (current == content)
                        return true;
                }
            }

            std::ofstream file(path, std::ios::binary | std::ios::trunc);
            file.write(content.data(), static_cast<std::streamsize>(content.size()));
            if (!file.good())
            {
                std::cerr << "Cannot write " << path.string() << "\n";
                return false;
            }

            return true;
        }

        /// The shortest decimal that reads back as @p value. An ostream's default precision is six significant digits,
        /// which would rewrite the number the engine then loads.
        std::string JsonNumber(double value)
        {
            char buffer[64];
            const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), value);
            return error == std::errc{} ? std::string(buffer, end) : "0";
        }

        std::string CurveJson(double y0, double y1)
        {
            std::ostringstream json;
            json << "{ \"parts\": [ { \"start\": { \"x\": 0, \"y\": " << JsonNumber(y0)
                 << " }, \"end\": { \"x\": 1, \"y\": " << JsonNumber(y1) << " }, \"fader\": \"Linear\" } ] }";
            return json.str();
        }

        const char* Pipeline(const ConfigVariant& variant)
        {
            if (variant.ambisonic)
                return "fidelity.ambisonic-surround.ampipeline";

            return variant.channels == 2 ? "fidelity.stereo.ampipeline" : "fidelity.surround.ampipeline";
        }

        std::string ConfigJson(const ConfigVariant& variant)
        {
            std::ostringstream json;
            json << "{\n"
                 << "  \"driver\": \"offline\",\n"
                 << "  \"output\": { \"frequency\": " << variant.outputRate << ", \"buffer_size\": " << variant.channels * variant.blockSize
                 << ", \"format\": \"Float32\", \"channels\": \""
                 << (variant.channels == 8 ? "Surround_7_1" : variant.channels == 6 ? "Surround_5_1" : "Stereo") << "\" },\n"
                 << "  \"mixer\": { \"panning_mode\": \"" << (variant.ambisonic ? "BinauralHighQuality" : "Stereo")
                 << "\", \"active_channels\": 50, \"virtual_channels\": 100, "
                 << "\"pipeline\": \"" << Pipeline(variant) << "\" },\n"
                 << (variant.ambisonic ? "  \"hrtf\": { \"amir_file\": \"data/sadie_h12.amir\", \"hrir_sampling\": \"Bilinear\" },\n" : "")
                 << "  \"game\": {\n"
                 << "    \"listener_fetch_mode\": \"Nearest\", \"track_environments\": false, \"listeners\": 100, \"entities\": 4096,\n"
                 << "    \"environments\": 512, \"rooms\": 1024, \"doppler_factor\": 1.0, \"sound_speed\": 343,\n"
                 << "    \"obstruction\": { \"lpf_curve\": " << CurveJson(0, 1) << ", \"gain_curve\": " << CurveJson(1, 1) << " },\n"
                 << "    \"occlusion\": { \"lpf_curve\": " << CurveJson(0, 1) << ", \"gain_curve\": " << CurveJson(1, 0) << " }\n"
                 << "  },\n"
                 << "  \"buses_file\": \"fidelity.buses.ambus\"\n"
                 << "}\n";
            return json.str();
        }

        std::string SoundJson(std::uint64_t id, const StimulusSpec& spec, bool streamed)
        {
            const bool loop = spec.kind == StimulusKind::LoopSine;
            std::ostringstream json;
            json << "{\"id\":" << id << ",\"name\":\"" << SoundName(spec, streamed) << "\",\"effect\":0,"
                 << "\"gain\":{\"kind\":\"Static\",\"value\":1},\"pitch\":{\"kind\":\"Static\",\"value\":" << JsonNumber(spec.pitch)
                 << "},\"bus\":1,"
                 << "\"priority\":{\"kind\":\"Static\",\"value\":1},\"spatialization\":0,\"attenuation\":0,\"scope\":0,"
                 << "\"fader\":\"Linear\",\"stream\":" << (streamed ? "true" : "false")
                 << ",\"loop\":{\"enabled\":" << (loop ? "true" : "false") << ",\"loop_count\":" << (loop ? 1000 : 0) << "},"
                 << "\"near_field_gain\":{\"kind\":\"Static\",\"value\":0},\"path\":\"fidelity/" << spec.name << ".wav\"}\n";
            return json.str();
        }

        /// The glide sound's pitch curve. The engine normalises a curve's x over the RTPC's own value range, so the
        /// identity over [0.25, 4] is a straight run from 0 to 1 in x.
        std::string GlideSoundJson(std::uint64_t id)
        {
            std::ostringstream json;
            json << "{\"id\":" << id << ",\"name\":\"" << kGlideSoundName << "\",\"effect\":0,"
                 << "\"gain\":{\"kind\":\"Static\",\"value\":1},"
                 << "\"pitch\":{\"kind\":\"RTPC\",\"value\":1,\"rtpc\":{\"id\":" << kGlideRtpcId << ",\"curve\":{\"parts\":[{"
                 << "\"start\":{\"x\":0,\"y\":0.25},\"end\":{\"x\":1,\"y\":4},\"fader\":\"Linear\"}]}}},"
                 << "\"bus\":1,\"priority\":{\"kind\":\"Static\",\"value\":1},\"spatialization\":0,\"attenuation\":0,\"scope\":0,"
                 << "\"fader\":\"Linear\",\"stream\":false,\"loop\":{\"enabled\":true,\"loop_count\":0},"
                 << "\"near_field_gain\":{\"kind\":\"Static\",\"value\":0},\"path\":\"fidelity/loop_sine_48000.wav\"}\n";
            return json.str();
        }

        /// The 997 Hz loop, positional, no attenuation: the pan scenarios place it with a channel location.
        std::string PositionalSineJson(std::uint64_t id)
        {
            std::ostringstream json;
            json << "{\"id\":" << id << ",\"name\":\"" << kPositionalSineName << "\",\"effect\":0,"
                 << "\"gain\":{\"kind\":\"Static\",\"value\":1},\"pitch\":{\"kind\":\"Static\",\"value\":1},"
                 << "\"bus\":1,\"priority\":{\"kind\":\"Static\",\"value\":1},\"spatialization\":1,\"attenuation\":0,\"scope\":0,"
                 << "\"fader\":\"Linear\",\"stream\":false,\"loop\":{\"enabled\":true,\"loop_count\":0},"
                 << "\"near_field_gain\":{\"kind\":\"Static\",\"value\":0},\"path\":\"fidelity/loop_sine_48000.wav\"}\n";
            return json.str();
        }

        /// The white noise (pink_48000) loop, HRTF-spatialized, no attenuation: the ambisonic scenarios place it with a location.
        std::string HrtfNoiseJson(std::uint64_t id)
        {
            std::ostringstream json;
            json << "{\"id\":" << id << ",\"name\":\"" << kHrtfNoiseName << "\",\"effect\":0,"
                 << "\"gain\":{\"kind\":\"Static\",\"value\":1},\"pitch\":{\"kind\":\"Static\",\"value\":1},"
                 << "\"bus\":1,\"priority\":{\"kind\":\"Static\",\"value\":1},\"spatialization\":3,\"attenuation\":0,\"scope\":0,"
                 << "\"fader\":\"Linear\",\"stream\":false,\"loop\":{\"enabled\":true,\"loop_count\":0},"
                 << "\"near_field_gain\":{\"kind\":\"Static\",\"value\":0},\"path\":\"fidelity/pink_48000.wav\"}\n";
            return json.str();
        }

        /// Independent stereo pink noise, positional, its spread following the fidelity_spread RTPC (identity over 0 - 360).
        std::string SpreadSweepJson(std::uint64_t id)
        {
            std::ostringstream json;
            json << "{\"id\":" << id << ",\"name\":\"" << kSpreadSweepSoundName << "\",\"effect\":0,"
                 << "\"gain\":{\"kind\":\"Static\",\"value\":1},\"pitch\":{\"kind\":\"Static\",\"value\":1},"
                 << "\"bus\":1,\"priority\":{\"kind\":\"Static\",\"value\":1},\"spatialization\":1,\"attenuation\":0,\"scope\":0,"
                 << "\"fader\":\"Linear\",\"stream\":false,\"loop\":{\"enabled\":false,\"loop_count\":0},"
                 << "\"near_field_gain\":{\"kind\":\"Static\",\"value\":0},\"path\":\"fidelity/pink_stereo_independent_48000.wav\","
                 << "\"spread\":{\"kind\":\"RTPC\",\"value\":0,\"rtpc\":{\"id\":" << kSpreadRtpcId << ",\"curve\":{\"parts\":[{"
                 << "\"start\":{\"x\":0,\"y\":0},\"end\":{\"x\":1,\"y\":360},\"fader\":\"Linear\"}]}}}}\n";
            return json.str();
        }
    } // namespace

    std::vector<ConfigVariant> SurroundConfigVariants()
    {
        return { { 1024, 48000, 6 }, { 1024, 48000, 8 } };
    }

    std::vector<ConfigVariant> AmbisonicConfigVariants()
    {
        return { { 1024, 48000, 6, true } };
    }

    std::string ConfigKind(const ConfigVariant& variant)
    {
        if (variant.ambisonic)
            return "ambisonic51";

        return variant.channels == 8 ? "surround71" : variant.channels == 6 ? "surround51" : "isolated";
    }

    std::vector<ConfigVariant> IsolatedConfigVariants()
    {
        std::vector<ConfigVariant> variants;
        for (const std::uint32_t rate : { 48000u, 44100u })
            for (const std::uint32_t block : { 256u, 1024u, 4096u })
                variants.push_back({ block, rate });

        return variants;
    }

    std::string ConfigName(std::string_view kind, std::uint32_t blockSize, std::uint32_t outputRate, std::string_view extension)
    {
        std::string name = "fidelity." + std::string(kind) + ".b" + std::to_string(blockSize);
        if (outputRate != 48000)
            name += ".r" + std::to_string(outputRate);

        return name + std::string(extension);
    }

    std::string SoundName(const StimulusSpec& spec, bool streamed)
    {
        return "fidelity." + spec.name + (streamed ? "_stream" : "");
    }

    bool GenerateAssets(const AssetPaths& paths)
    {
        std::error_code error;
        const std::filesystem::path sounds = paths.project / "sounds" / "fidelity";
        const std::filesystem::path banks = paths.project / "soundbanks";
        const std::filesystem::path rtpcs = paths.project / "rtpc";
        const std::filesystem::path data = paths.assets / "data" / "fidelity";
        for (const auto& directory : { sounds, banks, rtpcs, data })
        {
            std::filesystem::create_directories(directory, error);
            if (error)
            {
                std::cerr << "Cannot create " << directory.string() << ": " << error.message() << "\n";
                return false;
            }
        }

        bool ok = true;
        const std::string rtpc = "{\"id\":" + std::to_string(kGlideRtpcId) + ",\"name\":\"" + kGlideRtpcName +
            "\",\"min_value\":0.25,\"max_value\":4,\"default_value\":1,\"fade_settings\":{\"enabled\":false,"
            "\"fade_attack\":{\"duration\":0,\"fader\":\"Linear\"},\"fade_release\":{\"duration\":0,\"fader\":\"Linear\"}}}\n";
        ok = WriteIfChanged(rtpcs / "fidelity_pitch.json", rtpc) && ok;

        const std::string spreadRtpc = "{\"id\":" + std::to_string(kSpreadRtpcId) + ",\"name\":\"" + kSpreadRtpcName +
            "\",\"min_value\":0,\"max_value\":360,\"default_value\":0,\"fade_settings\":{\"enabled\":false,"
            "\"fade_attack\":{\"duration\":0,\"fader\":\"Linear\"},\"fade_release\":{\"duration\":0,\"fader\":\"Linear\"}}}\n";
        ok = WriteIfChanged(rtpcs / "fidelity_spread.json", spreadRtpc) && ok;

        std::vector<ConfigVariant> configs = IsolatedConfigVariants();
        for (const ConfigVariant& variant : SurroundConfigVariants())
            configs.push_back(variant);

        for (const ConfigVariant& variant : AmbisonicConfigVariants())
            configs.push_back(variant);

        for (const ConfigVariant& variant : configs)
            ok = WriteIfChanged(
                     paths.project / ConfigName(ConfigKind(variant), variant.blockSize, variant.outputRate, ".config.json"),
                     ConfigJson(variant)) &&
                ok;

        std::ostringstream bank;
        // The engine reads every list of a soundbank without a null check, so the empty ones are written explicitly.
        bank << "{\"id\":" << kSoundBankId << ",\"name\":\"fidelity\",\"switch_containers\":[],\"collections\":[],\"events\":[],"
             << "\"attenuators\":[],\"switches\":[],\"rtpc\":[\"fidelity_pitch.amrtpc\",\"fidelity_spread.amrtpc\"],\"effects\":[],\"sounds\":[";

        const std::vector<StimulusSpec>& catalog = StimulusCatalog();
        for (std::size_t i = 0; i < catalog.size(); ++i)
        {
            const StimulusSpec& spec = catalog[i];
            for (const bool streamed : { false, true })
            {
                const std::uint64_t id = kFirstSoundId + 2 * i + (streamed ? 1 : 0);
                const std::string file = spec.name + (streamed ? "_stream" : "");
                ok = WriteIfChanged(sounds / (file + ".json"), SoundJson(id, spec, streamed)) && ok;
                bank << (i == 0 && !streamed ? "" : ",") << "\"fidelity/" << file << ".amsound\"";
            }

            const std::vector<char> wav = EncodeWavFloat32(spec.sampleRate, spec.channels, RenderStimulusInterleaved(spec));
            ok = WriteIfChanged(data / (spec.name + ".wav"), std::string_view(wav.data(), wav.size())) && ok;
        }

        // The glide sound carries no catalog spec: its pitch comes from the RTPC at run time.
        ok = WriteIfChanged(sounds / "glide.json", GlideSoundJson(kFirstSoundId + 2 * catalog.size())) && ok;

        ok = WriteIfChanged(sounds / "positional_sine.json", PositionalSineJson(kFirstSoundId + 2 * catalog.size() + 1)) && ok;
        ok = WriteIfChanged(sounds / "spread_sweep.json", SpreadSweepJson(kFirstSoundId + 2 * catalog.size() + 2)) && ok;

        ok = WriteIfChanged(sounds / "hrtf_noise.json", HrtfNoiseJson(kFirstSoundId + 2 * catalog.size() + 3)) && ok;

        bank << ",\"fidelity/glide.amsound\",\"fidelity/positional_sine.amsound\",\"fidelity/spread_sweep.amsound\","
             << "\"fidelity/hrtf_noise.amsound\"]}\n";
        ok = WriteIfChanged(banks / "fidelity.json", bank.str()) && ok;
        return ok;
    }
} // namespace SparkyStudios::Audio::Amplitude::Fidelity
