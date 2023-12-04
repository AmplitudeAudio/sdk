// ==================================================================================
// Copyright (c) 2017 HiFi-LoFi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is furnished
// to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
// ==================================================================================

#ifndef _AUDIOFFT_H
#define _AUDIOFFT_H

/**
 * AudioFFT provides real-to-complex/complex-to-real FFT routines.
 *
 * Features:
 *
 * - Real-complex FFT and complex-real inverse FFT for power-of-2-sized real data.
 *
 * - Uniform interface to different FFT implementations (currently Ooura, FFTW3 and Apple Accelerate).
 *
 * - Complex data is handled in "split-complex" format, i.e. there are separate
 *   arrays for the real and imaginary parts which can be useful for SIMD optimizations
 *   (split-complex arrays have to be of length (size/2+1) representing bins from DC
 *   to Nyquist frequency).
 *
 * - Output is "ready to use" (all scaling etc. is already handled internally).
 *
 * - No allocations/deallocations after the initialization which makes it usable
 *   for real-time audio applications (that's what I wrote it for and using it).
 */

#include <cstddef>
#include <memory>

namespace SparkyStudios::Audio::Amplitude
{
    namespace detail
    {
        class AudioFFTImpl;
    }

    // =============================================================

    /**
     * @class AudioFFT
     * @brief Performs 1D FFTs
     */
    class AudioFFT
    {
    public:
        /**
         * @brief Constructor
         */
        AudioFFT();

        AudioFFT(const AudioFFT&) = delete;
        AudioFFT& operator=(const AudioFFT&) = delete;

        /**
         * @brief Destructor
         */
        ~AudioFFT();

        /**
         * @brief Initializes the FFT object
         * @param size Size of the real input (must be power 2)
         */
        void init(size_t size);

        /**
         * @brief Performs the forward FFT
         * @param data The real input data (has to be of the length as specified in init())
         * @param re The real part of the complex output (has to be of length as returned by ComplexSize())
         * @param im The imaginary part of the complex output (has to be of length as returned by ComplexSize())
         */
        void fft(const float* data, float* re, float* im);

        /**
         * @brief Performs the inverse FFT
         * @param data The real output data (has to be of the length as specified in init())
         * @param re The real part of the complex input (has to be of length as returned by ComplexSize())
         * @param im The imaginary part of the complex input (has to be of length as returned by ComplexSize())
         */
        void ifft(float* data, const float* re, const float* im);

        /**
         * @brief Gets the size of the buffer.
         * @return The buffer size.
         */
        [[nodiscard]] size_t size() const;

        /**
         * @brief Calculates the necessary size of the real/imaginary complex arrays
         * @param size The size of the real data
         * @return The size of the real/imaginary complex arrays
         */
        static size_t ComplexSize(size_t size);

    private:
        std::unique_ptr<detail::AudioFFTImpl> _impl;
        size_t _size;
    };
} // namespace SparkyStudios::Audio::Amplitude

#endif // Header guard
