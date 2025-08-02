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
    flags = { { value = "-msse2", tools = { "gcc", "clang", "icc" } }, { value = "/arch:SSE2", tools = { "cl", "clang_cl", "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_SSE2", "__SSE2__" },
    suffix = "-x86_sse2",
    test = [[#include <emmintrin.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        __m128i align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m128i one = _mm_load_si128((__m128i*)p);
    one = _mm_or_si128(one, one);
    _mm_store_si128((__m128i*)p, one);

    p = prevent_optimization(p);
}]]
  },
  X86_SSE3 = {
    flags = { { value = "-msse3", tools = { "gcc", "clang", "icc" } }, { value = "/arch:SSE2", tools = { "cl", "clang_cl", "icl" } }, { value = "/arch:AVX", tools = { "cl", "clang_cl", "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_SSE3", "__SSE3__" },
    suffix = "-x86_sse3",
    test = [[#include <pmmintrin.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        __m128 align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m128 one = _mm_load_ps((float*)p);
    one = _mm_hadd_ps(one, one);
    _mm_store_ps((float*)p, one);

    p = prevent_optimization(p);
}]]
  },
  X86_SSSE3 = {
    flags = { { value = "-mssse3", tools = { "gcc", "clang", "icc" } }, { value = "/arch:SSE2", tools = { "cl", "clang_cl", "icl" } }, { value = "/arch:AVX", tools = { "cl", "clang_cl", "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_SSSE3", "__SSSE3__" },
    suffix = "-x86_ssse3",
    test = [[#include <tmmintrin.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        __m128i align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m128i one = _mm_load_si128((__m128i*)p);
    one = _mm_abs_epi8(one);
    _mm_store_si128((__m128i*)p, one);

    p = prevent_optimization(p);
}]]
  },
  X86_SSE4_1 = {
    flags = { { value = "-msse4.1", tools = { "gcc", "clang", "icc" } }, { value = "/arch:SSE2", tools = { "cl", "clang_cl", "icl" } }, { value = "/arch:AVX", tools = { "cl", "clang_cl", "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_SSE4_1", "__SSE4_1__" },
    suffix = "-x86_sse4_1",
    test = [[#include <smmintrin.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        __m128i align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m128i one = _mm_load_si128((__m128i*)p);
    one = _mm_cvtepi16_epi32(one);
    _mm_store_si128((__m128i*)p, one);

    p = prevent_optimization(p);
}]]
  },
  X86_POPCNT = {
    flags = { { value = "-mssse3", tools = { "gcc", "clang", "icc" } }, { value = "-mpopcnt", tools = { "gcc", "clang", "icc" } }, { value = "/arch:AVX", tools = { "cl", "clang_cl", "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_POPCNT_INSN" },
    suffix = "-x86_popcnt",
    test = [[#include <nmmintrin.h>
#include <iostream>

unsigned* prevent_optimization(unsigned* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    unsigned* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    unsigned data;
    unsigned* p = &data;
    p = prevent_optimization(p);

    *p = _mm_popcnt_u32(*p);

    p = prevent_optimization(p);
}]]
  },
  X86_AVX = {
    flags = { { value = "-mavx", tools = { "gcc", "clang", "icc" } }, { value = "/arch:AVX", tools = { "cl", "clang_cl", "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_AVX", "__AVX__" },
    suffix = "-x86_avx",
    test = [[#include <immintrin.h>
#include <iostream>

#if (__clang_major__ == 3) && (__clang_minor__ == 6)
#error Not supported.
#endif
#if (__GNUC__ == 4) && (__GNUC_MINOR__ == 4) && !defined(__INTEL_COMPILER) && !defined(__clang__)
#error Not supported.
#endif

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[32];
        __m256 align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m256 one = _mm256_load_ps((float*)p);
    one = _mm256_add_ps(one, one);
    _mm256_store_ps((float*)p, one);

    p = prevent_optimization(p);
}]]
  },
  X86_AVX2 = {
    flags = { { value = "-mavx2", tools = { "gcc", "clang", "icc" } }, { value = "-xCORE-AVX2", tools = { "icc" } }, { value = "/arch:AVX2", tools = { "cl", "clang_cl", "icl" } }, { value = "/arch:CORE-AVX2", tools = { "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_AVX2", "__AVX2__" },
    suffix = "-x86_avx2",
    test = [[#include <immintrin.h>
#include <iostream>

#if (__clang_major__ == 3) && (__clang_minor__ == 6)
#error Not supported.
#endif

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[32];
        __m256 align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m256i one = _mm256_load_si256((__m256i*)p);
    one = _mm256_or_si256(one, one);
    _mm256_store_si256((__m256i*)p, one);

    p = prevent_optimization(p);
}]]
  },
  ARM_FMA3 = {
    flags = { { value = "-mfma", tools = { "gcc", "clang", "icc" } }, { value = "-xCORE-AVX2", tools = { "icc" } }, { value = "/arch:AVX2", tools = { "cl", "clang_cl", "icl" } }, { value = "/arch:CORE-AVX2", tools = { "icl" } } },
    define = { "AM_BUILDSYSTEM_ARCH_X86_FMA3", "__FMA__" },
    suffix = "-x86_fma3",
    test = [[#include <immintrin.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        __m128 align;
    };
    char* p = data;
    p = prevent_optimization(p);

    __m128 one = _mm_load_ps((float*)p);
    one = _mm_fmadd_ps(one, one, one);
    _mm_store_ps((float*)p, one);

    p = prevent_optimization(p);
}]]
  },
  ARM_NEON = {
    flags = { { value = "-mfpu=neon", tools = { "gcc", "clang", "icc" } } },
    define = { "AM_BUILDSYSTEM_ARCH_ARM_NEON" },
    suffix = "-arm_neon",
    test = [[#if defined(__clang_major__)
#if (__clang_major__ < 3) || ((__clang_major__ == 3) && (__clang_minor__ <= 3))
#error NEON is not supported on clang 3.3 and earlier.
#endif
#endif

#include <arm_neon.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        uint32x4_t align;
    };
    char* p = data;
    p = prevent_optimization(p);

    uint32x4_t one = vld1q_u32((uint32_t*)p);
    one = vaddq_u32(one, one);
    vst1q_u32((uint32_t*)p, one);

    p = prevent_optimization(p);
}]]
  },
  ARM64_NEON = {
    flags = { { value = "-mcpu=generic+simd", tools = { "gcc", "clang", "icc" } } },
    define = { "AM_BUILDSYSTEM_ARCH_ARM_NEON" },
    suffix = "-arm64_neon",
    test = [[#include <arm_neon.h>
#include <iostream>

char* prevent_optimization(char* ptr)
{
    volatile bool never = false;
    if (never) {
        while (*ptr++)
            std::cout << *ptr;
    }
    char* volatile* volatile opaque;
    opaque = &ptr;
    return *opaque;
}

int main()
{
    union {
        char data[16];
        uint32x4_t align;
    };
    char* p = data;
    p = prevent_optimization(p);

    uint32x4_t one = vld1q_u32((uint32_t*)p);
    one = vaddq_u32(one, one);

    // GCC 4.8 misses a subset of functions
    one = vdupq_laneq_u32(one, 1);

    vst1q_u32((uint32_t*)p, one);
    p = prevent_optimization(p);
}]]
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
  local ok = check_cxsnippets(info.test, { sourcekind = "cxx" })
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
