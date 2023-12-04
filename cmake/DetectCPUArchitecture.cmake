# Copyright (c) 2021-present Sparky Studios. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Inspired from libsimdpp - Copyright (C) 2012-2013  Povilas Kanapickas <povilas@radix.lt>

include(CheckCXXSourceRuns)
include(CheckCXXSourceCompiles)

# ------------------------------------------------------------------------------
# Compiler checks (internal)
set(AM_BUILDSYSTEM_GCC 0)
set(AM_BUILDSYSTEM_CLANG 0)
set(AM_BUILDSYSTEM_MSVC 0)
set(AM_BUILDSYSTEM_INTEL 0)
set(AM_BUILDSYSTEM_MSVC_INTEL 0)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(AM_BUILDSYSTEM_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Apple")
    set(AM_BUILDSYSTEM_CLANG 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(AM_BUILDSYSTEM_GCC 1)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    if (MSVC)
        set(AM_BUILDSYSTEM_MSVC_INTEL 1)
    else ()
        set(AM_BUILDSYSTEM_INTEL 1)
    endif ()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(AM_BUILDSYSTEM_MSVC 1)
else ()
    message(FATAL_ERROR "Compiler '${CMAKE_CXX_COMPILER_ID}' not recognized")
endif ()

# ------------------------------------------------------------------------------
# Architecture descriptions (internal)
#
# Each architecture has the following information specific to it:
#  - AM_BUILDSYSTEM_${ARCH}_TEST_CODE: source code snippet that uses functionality
#       from that arch. Used for @c check_cxx_source_runs macro.
#       We are taking extra care to confuse the compiler so that it does not
#       optimize things out. Nowadays compilers have good sense of when things
#       don't have side effects and will see through simple obfuscation
#       patterns.
#  - AM_BUILDSYSTEM_${ARCH}_CXX_FLAGS: compiler flags that are needed for compilation.
#  - AM_BUILDSYSTEM_${ARCH}_DEFINE: defines the macro that is needed to enable the
#       specific instruction set within the library.
#  - AM_BUILDSYSTEM_${ARCH}_SUFFIX: defines a suffix to append to the filename of the
#       source file specific to this architecture.
#
# Three lists are created:
#
#  - AM_BUILDSYSTEM_ARCHS_PRI - primary architectures.
#  - AM_BUILDSYSTEM_ARCHS_SEC - secondary architectures. Effectively equivalent to one
#       of the primary architectures, just different instructions are generated
#       in specific scenarios.
#  - AM_BUILDSYSTEM_ARCHS - all architectures
#

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_SSE2")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_SSE2_CXX_FLAGS "-msse2")
elseif (AM_BUILDSYSTEM_MSVC OR AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_SSE2_CXX_FLAGS "/arch:SSE2")
endif ()

set(AM_BUILDSYSTEM_X86_SSE2_DEFINE "AM_BUILDSYSTEM_ARCH_X86_SSE2")
set(AM_BUILDSYSTEM_X86_SSE2_SUFFIX "-x86_sse2")
set(AM_BUILDSYSTEM_X86_SSE2_TEST_CODE
        "#include <emmintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_SSE3")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_SSE3_CXX_FLAGS "-msse3")
elseif (AM_BUILDSYSTEM_MSVC)
    set(AM_BUILDSYSTEM_X86_SSE3_CXX_FLAGS "/arch:SSE2")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_SSE3_CXX_FLAGS "/arch:SSE3")
endif ()

set(AM_BUILDSYSTEM_X86_SSE3_DEFINE "AM_BUILDSYSTEM_ARCH_X86_SSE3")
set(AM_BUILDSYSTEM_X86_SSE3_SUFFIX "-x86_sse3")
set(AM_BUILDSYSTEM_X86_SSE3_TEST_CODE
        "#include <pmmintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_SSSE3")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_SSSE3_CXX_FLAGS "-mssse3")
elseif (AM_BUILDSYSTEM_MSVC)
    set(AM_BUILDSYSTEM_X86_SSSE3_CXX_FLAGS "/arch:SSE2")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_SSSE3_CXX_FLAGS "/arch:SSSE3")
endif ()

set(AM_BUILDSYSTEM_X86_SSSE3_DEFINE "AM_BUILDSYSTEM_ARCH_X86_SSSE3")
set(AM_BUILDSYSTEM_X86_SSSE3_SUFFIX "-x86_ssse3")
set(AM_BUILDSYSTEM_X86_SSSE3_TEST_CODE
        "#include <tmmintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_SSE4_1")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_SSE4_1_CXX_FLAGS "-msse4.1")
elseif (AM_BUILDSYSTEM_MSVC)
    set(AM_BUILDSYSTEM_X86_SSE4_1_CXX_FLAGS "/arch:SSE2")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_SSE4_1_CXX_FLAGS "/arch:SSE4.1")
endif ()

set(AM_BUILDSYSTEM_X86_SSE4_1_DEFINE "AM_BUILDSYSTEM_ARCH_X86_SSE4_1")
set(AM_BUILDSYSTEM_X86_SSE4_1_SUFFIX "-x86_sse4_1")
set(AM_BUILDSYSTEM_X86_SSE4_1_TEST_CODE
        "#include <smmintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_POPCNT_INSN")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_POPCNT_INSN_CXX_FLAGS "-mssse3 -mpopcnt")
elseif (AM_BUILDSYSTEM_MSVC)
    set(AM_BUILDSYSTEM_X86_POPCNT_INSN_CXX_FLAGS "/arch:SSE4.2")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_POPCNT_INSN_CXX_FLAGS "/arch:SSE4.2")
endif ()

set(AM_BUILDSYSTEM_X86_POPCNT_INSN_DEFINE "AM_BUILDSYSTEM_ARCH_X86_POPCNT_INSN")
set(AM_BUILDSYSTEM_X86_POPCNT_INSN_SUFFIX "-x86_popcnt")
set(AM_BUILDSYSTEM_X86_POPCNT_INSN_TEST_CODE
        "#include <nmmintrin.h>
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
    }"
)
###

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_AVX")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX_CXX_FLAGS "-mavx")
elseif (AM_BUILDSYSTEM_MSVC OR AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX_CXX_FLAGS "/arch:AVX")
endif ()

set(AM_BUILDSYSTEM_X86_AVX_DEFINE "AM_BUILDSYSTEM_ARCH_X86_AVX")
set(AM_BUILDSYSTEM_X86_AVX_SUFFIX "-x86_avx")
set(AM_BUILDSYSTEM_X86_AVX_TEST_CODE
        "#include <immintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_AVX2")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_X86_AVX2_CXX_FLAGS "-mavx2")
elseif (AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX2_CXX_FLAGS "-xCORE-AVX2")
elseif (AM_BUILDSYSTEM_MSVC)
    set(AM_BUILDSYSTEM_X86_AVX2_CXX_FLAGS "/arch:AVX")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX2_CXX_FLAGS "/arch:CORE-AVX2")
endif ()

set(AM_BUILDSYSTEM_X86_AVX2_DEFINE "AM_BUILDSYSTEM_ARCH_X86_AVX2")
set(AM_BUILDSYSTEM_X86_AVX2_SUFFIX "-x86_avx2")
set(AM_BUILDSYSTEM_X86_AVX2_TEST_CODE
        "#include <immintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_FMA3")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_X86_FMA3_CXX_FLAGS "-mfma")
elseif (AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_FMA3_CXX_FLAGS "-xCORE-AVX2")
elseif (AM_BUILDSYSTEM_MSVC)
    set(AM_BUILDSYSTEM_X86_FMA3_CXX_FLAGS "/arch:AVX")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_FMA3_CXX_FLAGS "/arch:CORE-AVX2")
endif ()

set(AM_BUILDSYSTEM_X86_FMA3_DEFINE "AM_BUILDSYSTEM_ARCH_X86_FMA3")
set(AM_BUILDSYSTEM_X86_FMA3_SUFFIX "-x86_fma3")
set(AM_BUILDSYSTEM_X86_FMA3_TEST_CODE
        "#include <immintrin.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_FMA4")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    # intel does not support FMA4
    set(AM_BUILDSYSTEM_X86_FMA4_CXX_FLAGS "-mfma4")
elseif (AM_BUILDSYSTEM_MSVC OR AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_FMA4_CXX_FLAGS "/arch:AVX")
endif ()

set(AM_BUILDSYSTEM_X86_FMA4_DEFINE "AM_BUILDSYSTEM_ARCH_X86_FMA4")
set(AM_BUILDSYSTEM_X86_FMA4_SUFFIX "-x86_fma4")
set(AM_BUILDSYSTEM_X86_FMA4_TEST_CODE
        "#include <x86intrin.h>
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
        one = _mm_macc_ps(one, one, one);
        _mm_store_ps((float*)p, one);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_XOP")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_X86_XOP_CXX_FLAGS "-mxop")
endif ()

set(AM_BUILDSYSTEM_X86_XOP_DEFINE "AM_BUILDSYSTEM_ARCH_X86_XOP")
set(AM_BUILDSYSTEM_X86_XOP_SUFFIX "-x86_xop")
set(AM_BUILDSYSTEM_X86_XOP_TEST_CODE
        "#include <x86intrin.h>
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
        one = _mm_cmov_si128(one, one, one);
        one = _mm_comeq_epi64(one, one);
        _mm_store_si128((__m128i*)p, one);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_AVX512F")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_X86_AVX512F_CXX_FLAGS "-mavx512f -O1")
elseif (AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX512F_CXX_FLAGS "-xCOMMON-AVX512")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX512F_CXX_FLAGS "/arch:COMMON-AVX512")
else ()
    #unsupported on MSVC
endif ()

set(AM_BUILDSYSTEM_X86_AVX512F_DEFINE "AM_BUILDSYSTEM_ARCH_X86_AVX512F")
set(AM_BUILDSYSTEM_X86_AVX512F_SUFFIX "-x86_avx512f")
set(AM_BUILDSYSTEM_X86_AVX512F_TEST_CODE
        "#include <immintrin.h>
    #include <iostream>

    #if defined(__GNUC__) && (__GNUC__ < 6) && !defined(__INTEL_COMPILER) && !defined(__clang__)
    #error GCC 5.x and older are not supported on AVX512F.
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
            char data[64];
            __m512 align;
        };
        char* p = data;
        p = prevent_optimization(p);
        __m512 f = _mm512_load_ps((float*)p);
        p = prevent_optimization(p);
        __m512i i = _mm512_load_epi32((__m512i*)p);
        p = prevent_optimization(p);

        f = _mm512_add_ps(f, f);

        // MSVC 2017 miss this
        i = _mm512_or_epi32(i, i);
        f = _mm512_ceil_ps(f);

        // ICE on various versions of Clang trying to select palignr
        __m512i i2 = _mm512_load_epi32((__m512i*)p);
        __m512i ap = _mm512_alignr_epi32(i, i, 2);
        i = _mm512_mask_alignr_epi32(ap, 0xcccc, i2, i2, 14);

        p = prevent_optimization(p);
        _mm512_store_ps((float*)p, f);
        p = prevent_optimization(p);
        _mm512_store_epi32((void*)p, i);
        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_AVX512BW")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_X86_AVX512BW_CXX_FLAGS "-mavx512bw")
    #unsupported on MSVC
elseif (AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX512BW_CXX_FLAGS "-xCORE-AVX512")
elseif (AM_BUILDSYSTEM_MSVC_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX512BW_CXX_FLAGS "/arch:CORE-AVX512")
endif ()

set(AM_BUILDSYSTEM_X86_AVX512BW_DEFINE "AM_BUILDSYSTEM_ARCH_X86_AVX512BW")
set(AM_BUILDSYSTEM_X86_AVX512BW_SUFFIX "-x86_avx512bw")
set(AM_BUILDSYSTEM_X86_AVX512BW_TEST_CODE
        "#include <immintrin.h>
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
            char data[64];
            __m512i align;
        };
        char* p = data;
        p = prevent_optimization(p);

        __m512i i = _mm512_load_si512((void*)p);
        i = _mm512_add_epi16(i, i); // only in AVX-512BW
        _mm512_store_si512((void*)p, i);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_AVX512DQ")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX512DQ_CXX_FLAGS "-mavx512dq")
    #unsupported on MSVC
endif ()

set(AM_BUILDSYSTEM_X86_AVX512DQ_DEFINE "AM_BUILDSYSTEM_ARCH_X86_AVX512DQ")
set(AM_BUILDSYSTEM_X86_AVX512DQ_SUFFIX "-x86_avx512dq")
set(AM_BUILDSYSTEM_X86_AVX512DQ_TEST_CODE
        "#include <immintrin.h>
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
            char data[64];
            __m512 align;
        };
        char* p = data;
        p = prevent_optimization(p);

        __m512 f = _mm512_load_ps((float*)p);
        f = _mm512_and_ps(f, f); // only in AVX512-DQ
        _mm512_store_ps((float*)p, f);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "X86_AVX512VL")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC OR AM_BUILDSYSTEM_INTEL)
    set(AM_BUILDSYSTEM_X86_AVX512VL_CXX_FLAGS "-mavx512vl")
    #unsupported on MSVC
endif ()

set(AM_BUILDSYSTEM_X86_AVX512VL_DEFINE "AM_BUILDSYSTEM_ARCH_X86_AVX512VL")
set(AM_BUILDSYSTEM_X86_AVX512VL_SUFFIX "-x86_avx512vl")
set(AM_BUILDSYSTEM_X86_AVX512VL_TEST_CODE
        "#if !defined(__APPLE__) && (__clang_major__ == 3)
    #error AVX512VL is not supported on clang 3.9 and earlier.
    #endif

    #include <immintrin.h>
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

        __m128 f = _mm_load_ps((float*)p);
        f = _mm_rcp14_ps(f); // only in AVX512-VL
        _mm_store_ps((float*)p, f);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "ARM_NEON")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_ARM_NEON_CXX_FLAGS "-mfpu=neon")
endif ()

set(AM_BUILDSYSTEM_ARM_NEON_DEFINE "AM_BUILDSYSTEM_ARCH_ARM_NEON")
set(AM_BUILDSYSTEM_ARM_NEON_SUFFIX "-arm_neon")
set(AM_BUILDSYSTEM_ARM_NEON_TEST_CODE
        "#if defined(__clang_major__)
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_SEC "ARM_NEON_FLT_SP")

if (AM_BUILDSYSTEM_CLANG OR AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_ARM_NEON_FLT_SP_CXX_FLAGS "-mfpu=neon")
endif ()

set(AM_BUILDSYSTEM_ARM_NEON_FLT_SP_DEFINE "AM_BUILDSYSTEM_ARCH_ARM_NEON_FLT_SP")
set(AM_BUILDSYSTEM_ARM_NEON_FLT_SP_SUFFIX "-arm_neon_flt_sp")

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "ARM64_NEON")

if (AM_BUILDSYSTEM_CLANG)
    set(AM_BUILDSYSTEM_ARM64_NEON_CXX_FLAGS "-arch arm64")
elseif (AM_BUILDSYSTEM_GCC)
    set(AM_BUILDSYSTEM_ARM64_NEON_CXX_FLAGS "-mcpu=generic+simd")
endif ()

set(AM_BUILDSYSTEM_ARM64_NEON_DEFINE "AM_BUILDSYSTEM_ARCH_ARM_NEON")
set(AM_BUILDSYSTEM_ARM64_NEON_SUFFIX "-arm64_neon")
set(AM_BUILDSYSTEM_ARM64_NEON_TEST_CODE
        "#include <arm_neon.h>
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
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "MIPS_MSA")

set(AM_BUILDSYSTEM_MIPS_MSA_CXX_FLAGS "-mips64r5 -mmsa -mhard-float -mfp64 -mnan=legacy")
set(AM_BUILDSYSTEM_MIPS_MSA_DEFINE "AM_BUILDSYSTEM_ARCH_MIPS_MSA")
set(AM_BUILDSYSTEM_MIPS_MSA_SUFFIX "-mips_msa")
set(AM_BUILDSYSTEM_MIPS_MSA_TEST_CODE
        "#include <msa.h>
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
            v4i32 align;
        };
        char* p = data;
        p = prevent_optimization(p);

        v16i8 v = __msa_ld_b(p, 0);
        v = __msa_add_a_b(v, v);
        __msa_st_b(v, p, 0);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "POWER_ALTIVEC")

set(AM_BUILDSYSTEM_POWER_ALTIVEC_CXX_FLAGS "-maltivec")
set(AM_BUILDSYSTEM_POWER_ALTIVEC_DEFINE "AM_BUILDSYSTEM_ARCH_POWER_ALTIVEC")
set(AM_BUILDSYSTEM_POWER_ALTIVEC_SUFFIX "-power_altivec")
set(AM_BUILDSYSTEM_POWER_ALTIVEC_TEST_CODE
        "#include <altivec.h>
    #include <iostream>

    #if defined(__GNUC__) && (__GNUC__ < 6) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #if !defined(__INTEL_COMPILER) && !defined(__clang__)
    #error GCC 5.0 and older are not supported on PPC little-endian.
    #endif
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
            char data[16];
            vector unsigned char align;
        };
        char* p = data;
        p = prevent_optimization(p);

        vector unsigned char v = vec_ld(0, (unsigned char*)p);
        v = vec_add(v, v);
        vec_st(v, 0, (unsigned char*)p);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "POWER_VSX_206")

set(AM_BUILDSYSTEM_POWER_VSX_206_CXX_FLAGS "-mvsx")
set(AM_BUILDSYSTEM_POWER_VSX_206_DEFINE "AM_BUILDSYSTEM_ARCH_POWER_VSX_206")
set(AM_BUILDSYSTEM_POWER_VSX_206_SUFFIX "-power_vsx_2.06")
set(AM_BUILDSYSTEM_POWER_VSX_206_TEST_CODE
        "#include <altivec.h>
    #include <iostream>

    #if defined(__GNUC__) && (__GNUC__ < 6) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #if !defined(__INTEL_COMPILER) && !defined(__clang__)
    #error GCC 5.0 and older are not supported on PPC little-endian.
    #endif
    #endif

    #if defined(__GNUC__) && (__GNUC__ < 6) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #if !defined(__INTEL_COMPILER) && !defined(__clang__)
    // Internal compiler errors or wrong behaviour on various SIMD memory operations
    #error GCC 5.x and older not supported on VSX big-endian.
    #endif
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
            char data[16];
            vector unsigned char align;
        };
        char* p = data;
        p = prevent_optimization(p);

        vector unsigned char v = vec_vsx_ld(0, (unsigned char*)p);
        v = vec_add(v, v);
        vec_vsx_st(v, 0, (unsigned char*)p);

        p = prevent_optimization(p);
    }"
)

list(APPEND AM_BUILDSYSTEM_ARCHS_PRI "POWER_VSX_207")

set(AM_BUILDSYSTEM_POWER_VSX_207_CXX_FLAGS "-mvsx -mcpu=power8")
set(AM_BUILDSYSTEM_POWER_VSX_207_DEFINE "AM_BUILDSYSTEM_ARCH_POWER_VSX_207")
set(AM_BUILDSYSTEM_POWER_VSX_207_SUFFIX "-power_vsx_2.07")
set(AM_BUILDSYSTEM_POWER_VSX_207_TEST_CODE
        "#include <altivec.h>
    #include <iostream>

    #if defined(__GNUC__) && (__GNUC__ < 6) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    #if !defined(__INTEL_COMPILER) && !defined(__clang__)
    #error GCC 5.0 and older are not supported on PPC little-endian.
    #endif
    #endif

    #if defined(__GNUC__) && (__GNUC__ < 6) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    #if !defined(__INTEL_COMPILER) && !defined(__clang__)
    #error GCC 5.x and older not supported on VSX big-endian.
    #endif
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
            char data[16];
            vector unsigned char align;
        };
        char* p = data;
        p = prevent_optimization(p);

        vector unsigned char v = vec_vsx_ld(0, (unsigned char*)p);
        v = vec_vpopcnt(v);
        vec_vsx_st(v, 0, (unsigned char*)p);

        p = prevent_optimization(p);
    }"
)

set(AM_BUILDSYSTEM_ARCHS "${AM_BUILDSYSTEM_ARCHS_PRI};${AM_BUILDSYSTEM_ARCHS_SEC}")

# ------------------------------------------------------------------------------
# Given one arch, returns compilation flags and an unique identifier (internal)
# If the given architecture does not exist, sets both result variables to ""
#
# Arguments:
#
# - CXX_FLAGS_VAR: the name of a variable to store the compilation flags to
#
# - DEFINES_LIST_VAR: the name of a variable to store comma delimited list of
# preprocessor defines for the current architecture.
#
# - UNIQUE_ID_VAR: the name of a variable to store the unique identifier to
#
# - ARCH: an architecture
#
function(am_buildsystem_get_arch_info CXX_FLAGS_VAR DEFINES_LIST_VAR UNIQUE_ID_VAR ARCH)
    set(UNIQUE_ID "")
    set(CXX_FLAGS "")
    set(DISPATCH_FLAGS "")
    set(DEFINES_LIST "")

    string(REPLACE "," ";" ARCH_IDS "${ARCH}")
    list(SORT ARCH_IDS)
    foreach (ID ${ARCH_IDS})
        if (${ID} STREQUAL "NONE_NULL")
            set(UNIQUE_ID "${UNIQUE_ID}-null")
        else ()
            list(FIND AM_BUILDSYSTEM_ARCHS "${ID}" FOUND)
            if (NOT ${FOUND} EQUAL -1)
                list(APPEND DEFINES_LIST "${AM_BUILDSYSTEM_${ID}_DEFINE}")
                set(CXX_FLAGS "${CXX_FLAGS} ${AM_BUILDSYSTEM_${ID}_CXX_FLAGS} -D${AM_BUILDSYSTEM_${ID}_DEFINE}")
                set(UNIQUE_ID "${UNIQUE_ID}${AM_BUILDSYSTEM_${ID}_SUFFIX}")
            endif ()
        endif ()
    endforeach ()

    string(REPLACE ";" "," DEFINES_LIST "${DEFINES_LIST}")

    set(${CXX_FLAGS_VAR} "${CXX_FLAGS}" PARENT_SCOPE)
    set(${UNIQUE_ID_VAR} "${UNIQUE_ID}" PARENT_SCOPE)
    set(${DEFINES_LIST_VAR} "${DEFINES_LIST}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# Given a list of archs, return all possible permutations of them (internal)
#
# Arguments:
#
# - ALL_ARCHS_VAL: the name of the variable to store the permutation to
#
# - ARCH...: a list of supported architectures
#
function(am_buildsystem_get_arch_perm ALL_ARCHS_VAR)
    list(APPEND ARCHS ${ARGV})
    list(REMOVE_AT ARCHS 0)

    foreach (ARCH ${ARCHS})
        set(ARCH_SUPPORTED_${ARCH} "1")
    endforeach ()

    set(ALL_ARCHS "NONE_NULL")
    if (DEFINED ARCH_SUPPORTED_X86_SSE2)
        # all x86_64 processors
        list(APPEND ALL_ARCHS "X86_SSE2")
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_SSE3)
        # Since Prescott, Merom (Core architecture)
        # Since later K8 steppings, fully supported since K10
        list(APPEND ALL_ARCHS "X86_SSE3")
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_SSSE3)
        # Since Merom (Core architecture)
        # Since Bobcat and Bulldozer
        list(APPEND ALL_ARCHS "X86_SSSE3")
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_SSE4_1)
        # Since Penryl (45 nm Merom shrink)
        # Since Bulldozer
        list(APPEND ALL_ARCHS "X86_SSE4_1")
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_POPCNT_INSN)
        # Since Nehalem and K10.

        # NOTE: These two architectures are the only that support popcnt and
        # don't support AVX. There's no full overlap of the instruction set
        # support in these architectures, thus these two separate configs were
        # omitted from the default instruction set matrix.
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_AVX)
        # Since Sandy Bridge, Bulldozer, Jaguar
        list(APPEND ALL_ARCHS "X86_AVX,X86_POPCNT_INSN")

        if (DEFINED ARCH_SUPPORTED_X86_FMA3)
            # Since Haswell, Piledriver (later Bulldozer variant)
            # All CPUs in this range support popcnt
        endif ()
        if (DEFINED ARCH_SUPPORTED_X86_FMA4)
            # Since Bulldozer until Zen. Jaguar does not support FMA4 nor FMA3
            # All CPUs in this range support popcnt
            list(APPEND ALL_ARCHS "X86_AVX,X86_FMA4,X86_POPCNT_INSN")
        endif ()
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_AVX2)
        # Since Haswell and Zen
        # All Intel and AMD CPUs that support AVX2 also support FMA3 and POPCNT,
        # thus separate X86_AVX2 config is not needed.
        if (DEFINED ARCH_SUPPORTED_X86_FMA3)
            list(APPEND ALL_ARCHS "X86_AVX2,X86_FMA3,X86_POPCNT_INSN")
        endif ()
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_FMA3)
        # Since Haswell, Piledriver (later Bulldozer variant)
        # All Intel and AMD CPUs that support FMA3 also support AVX, thus
        # separate X86_FMA3 config is not needed
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_FMA4)
        # Since Bulldozer until Zen
        # All AMD CPUs that support FMA4 also support AVX, thus
        # separate X86_FMA4 config is not needed
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_AVX512F)
        # Since Knights Landing, Skylake-X
        # All Intel CPUs that support AVX512F also support FMA3 and POPCNT,
        # thus separate X86_512F config is not needed.
        list(APPEND ALL_ARCHS "X86_AVX512F,X86_FMA3,X86_POPCNT_INSN")

        if (DEFINED ARCH_SUPPORTED_X86_AVX512BW)
            if (DEFINED ARCH_SUPPORTED_X86_AVX512DQ)
                if (DEFINED ARCH_SUPPORTED_X86_AVX512VL)
                    # All Intel processors that support AVX512BW also support
                    # AVX512DQ and AVX512VL
                    list(APPEND ALL_ARCHS "X86_AVX512F,X86_FMA3,X86_POPCNT_INSN,X86_AVX512BW,X86_AVX512DQ,X86_AVX512VL")
                endif ()
            endif ()
        endif ()
    endif ()
    if (DEFINED ARCH_SUPPORTED_X86_XOP)
        list(APPEND ALL_ARCHS "X86_XOP")
        if (DEFINED ARCH_SUPPORTED_X86_AVX)
            list(APPEND ALL_ARCHS "X86_AVX,X86_XOP")
        endif ()
    endif ()
    if (DEFINED ARCH_SUPPORTED_ARM_NEON)
        list(APPEND ALL_ARCHS "ARM_NEON")
        list(APPEND ALL_ARCHS "ARM_NEON_FLT_SP")
    endif ()
    if (DEFINED ARCH_SUPPORTED_ARM64_NEON)
        list(APPEND ALL_ARCHS "ARM64_NEON")
    endif ()
    if (DEFINED ARCH_SUPPORTED_MIPS_MSA)
        list(APPEND ALL_ARCHS "MIPS_MSA")
    endif ()
    if (DEFINED ARCH_SUPPORTED_POWER_ALTIVEC)
        list(APPEND ALL_ARCHS "POWER_ALTIVEC")
    endif ()
    if (DEFINED ARCH_SUPPORTED_POWER_VSX_206)
        list(APPEND ALL_ARCHS "POWER_VSX_206")
    endif ()
    if (DEFINED ARCH_SUPPORTED_POWER_VSX_207)
        list(APPEND ALL_ARCHS "POWER_VSX_207")
    endif ()
    set(${ALL_ARCHS_VAR} "${ALL_ARCHS}" PARENT_SCOPE)
endfunction()

# ------------------------------------------------------------------------------
# am_buildsystem_get_runnable_archs(ARCH_LIST_VAR)
#
# Returns a list of architectures that are supported by the current build
# system and the processor.
#
# Arguments:
#
# * ARCH_LIST_VAR: the name of the variable to put the architecture list to
function(am_buildsystem_get_runnable_archs ARCH_LIST_VAR)
    foreach (ARCH ${AM_BUILDSYSTEM_ARCHS_PRI})
        set(CMAKE_REQUIRED_FLAGS "${AM_BUILDSYSTEM_${ARCH}_CXX_FLAGS}")
        check_cxx_source_runs("${AM_BUILDSYSTEM_${ARCH}_TEST_CODE}" CAN_RUN_${ARCH})
        if (CAN_RUN_${ARCH})
            list(APPEND ARCHS ${ARCH})
        endif ()
    endforeach ()

    am_buildsystem_get_arch_perm(ALL_ARCHS "${ARCHS}")
    set(${ARCH_LIST_VAR} "${ALL_ARCHS}" PARENT_SCOPE)
endfunction()
