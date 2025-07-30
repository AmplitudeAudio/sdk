-- Copyright (c) 2025-present Sparky Studios. All rights reserved.
--
-- Licensed under the Apache License, Version 2.0 (the "License");
-- You may not use this file except in compliance with the License.
-- You may obtain a copy of the License at
--
--     http://www.apache.org/licenses/LICENSE-2.0
--
-- Unless required by applicable law or agreed to in writing, software
-- distributed under the License is distributed on an "AS IS" BASIS,
-- WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
-- See the License for the specific language governing permissions and
-- limitations under the License.

local cpu_archs = {
    X86_SSE2 = {
        flags = {"-msse2", "/arch:SSE2"},
        define = "AM_BUILDSYSTEM_ARCH_X86_SSE2",
        suffix = "-x86_sse2",
        test = [[#include <emmintrin.h>
char test() { __m128i a; a = _mm_setzero_si128(); return 0; }]]
    },
    X86_SSE3 = {
        flags = {"-msse3", "/arch:SSE2", "/arch:SSE3"},
        define = "AM_BUILDSYSTEM_ARCH_X86_SSE3",
        suffix = "-x86_sse3",
        test = [[#include <pmmintrin.h>
char test() { __m128 a; a = _mm_setzero_ps(); return 0; }]]
    },
    X86_SSSE3 = {
        flags = {"-mssse3", "/arch:SSE2", "/arch:SSSE3"},
        define = "AM_BUILDSYSTEM_ARCH_X86_SSSE3",
        suffix = "-x86_ssse3",
        test = [[#include <tmmintrin.h>
char test() { __m128i a; a = _mm_setzero_si128(); return 0; }]]
    },
    X86_SSE4_1 = {
        flags = {"-msse4.1", "/arch:SSE2", "/arch:SSE4.1"},
        define = "AM_BUILDSYSTEM_ARCH_X86_SSE4_1",
        suffix = "-x86_sse4_1",
        test = [[#include <smmintrin.h>
char test() { __m128i a; a = _mm_setzero_si128(); return 0; }]]
    },
    X86_AVX = {
        flags = {"-mavx", "/arch:AVX"},
        define = "AM_BUILDSYSTEM_ARCH_X86_AVX",
        suffix = "-x86_avx",
        test = [[#include <immintrin.h>
char test() { __m256 a; a = _mm256_setzero_ps(); return 0; }]]
    },
    X86_AVX2 = {
        flags = {"-mavx2", "/arch:AVX", "/arch:CORE-AVX2"},
        define = "AM_BUILDSYSTEM_ARCH_X86_AVX2",
        suffix = "-x86_avx2",
        test = [[#include <immintrin.h>
char test() { __m256i a; a = _mm256_setzero_si256(); return 0; }]]
    },
    ARM_NEON = {
        flags = {"-mfpu=neon"},
        define = "AM_BUILDSYSTEM_ARCH_ARM_NEON",
        suffix = "-arm_neon",
        test = [[#include <arm_neon.h>
char test() { uint32x4_t a; a = vdupq_n_u32(0); return 0; }]]
    },
    ARM64_NEON = {
        flags = {"-arch arm64", "-mcpu=generic+simd"},
        define = "AM_BUILDSYSTEM_ARCH_ARM_NEON",
        suffix = "-arm64_neon",
        test = [[#include <arm_neon.h>
char test() { uint32x4_t a; a = vdupq_n_u32(0); return 0; }]]
    }
}

function am_get_arch_info(arch)
    local info = cpu_archs[arch]
    if info then
        return info.flags, info.define, info.suffix
    end
    return nil, nil, nil
end

import("lib.detect.check_cxsnippets")

local function is_arch_supported(arch)
    local info = cpu_archs[arch]
    if not info then return false end
    local ok = check_cxsnippets(info.test, {sourcekind = "cxx"})
    return ok
end

function am_get_supported_archs()
    local supported = {}
    for arch, _ in pairs(cpu_archs) do
        if is_arch_supported(arch) then
            table.insert(supported, arch)
        end
    end
    return supported
end

