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

#pragma once

#ifndef SS_AMPLITUDE_AUDIO_UTILS_SCOPED_DENORMAL_FLUSH_H
#define SS_AMPLITUDE_AUDIO_UTILS_SCOPED_DENORMAL_FLUSH_H

#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
#include <immintrin.h>
#define AM_DENORMAL_FLUSH_SUPPORTED 1
#elif defined(__aarch64__) && defined(__GNUC__)
#include <cstdint>
#define AM_DENORMAL_FLUSH_SUPPORTED 1
#endif

namespace SparkyStudios::Audio::Amplitude
{
    /**
     * @brief RAII guard enabling flush-to-zero (FTZ) and denormals-are-zero (DAZ) on the current thread.
     *
     * Recursive DSP (IIR filters, envelopes, reverb tails) decays into subnormal floats on silence,
     * which can cost 10-100x per sample on affected CPUs. Audio processing threads should hold this
     * guard for the duration of processing. The previous floating-point state is restored on destruction.
     *
     * @internal
     */
    class ScopedDenormalFlush
    {
    public:
        ScopedDenormalFlush()
        {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
            _prevState = _mm_getcsr();
            _mm_setcsr(_prevState | 0x8040); // FTZ (bit 15) | DAZ (bit 6)
#elif defined(__aarch64__) && defined(__GNUC__)
            __asm__ volatile("mrs %0, fpcr" : "=r"(_prevState));
            __asm__ volatile("msr fpcr, %0" ::"r"(_prevState | (1ULL << 24))); // FZ
#endif
        }

        ~ScopedDenormalFlush()
        {
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
            _mm_setcsr(_prevState);
#elif defined(__aarch64__) && defined(__GNUC__)
            __asm__ volatile("msr fpcr, %0" ::"r"(_prevState));
#endif
        }

        ScopedDenormalFlush(const ScopedDenormalFlush&) = delete;
        ScopedDenormalFlush& operator=(const ScopedDenormalFlush&) = delete;

    private:
#if defined(__SSE__) || defined(_M_X64) || defined(_M_IX86)
        unsigned int _prevState = 0;
#elif defined(__aarch64__) && defined(__GNUC__)
        std::uint64_t _prevState = 0;
#endif
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // SS_AMPLITUDE_AUDIO_UTILS_SCOPED_DENORMAL_FLUSH_H
