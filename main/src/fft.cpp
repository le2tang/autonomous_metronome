#include "fft.h"

#include <math.h>

#include "memory.h"

namespace fft {

FFT::~FFT() {
    if (twiddle_) {
        delete[] twiddle_;
    }

    if (twiddle_inv_) {
        delete[] twiddle_inv_;
    }
}

void FFT::twiddle_init(int max_fft_size) {
    int num_twiddle = 0;
    int i = max_fft_size;
    while (i > 1) {
        i >>= 1;
        ++num_twiddle;
    }

    twiddle_ = new float[2 * num_twiddle];

    for (int i = 0; i < num_twiddle; ++i) {
        twiddle_[2 * i] = cos(-2 * M_PI / (2 << i));
        twiddle_[2 * i + 1] = sin(-2 * M_PI / (2 << i));
    }
}

void FFT::twiddle_inv_init(int max_fft_size) {
    int num_twiddle = 1;
    int i = max_fft_size;
    while (i > 1) {
        i >>= 1;
        ++num_twiddle;
    }

    twiddle_inv_ = new float[2 * num_twiddle];

    for (int i = 0; i < num_twiddle; ++i) {
        twiddle_inv_[2 * i] = cos(2 * M_PI / (2 << i));
        twiddle_inv_[2 * i + 1] = sin(2 * M_PI / (2 << i));
    }
}

void FFT::fft(float *data, int fft_size) { fft_(data, fft_size, false); }

void FFT::ifft(float *data, int fft_size) { fft_(data, fft_size, true); }

void FFT::fft_(float *data, int fft_size, bool invert) {
    float *twiddle = (invert) ? twiddle_inv_ : twiddle_;

    bit_reverse_(data, fft_size);
    for (int s = 1; (1 << s) <= fft_size; ++s) {
        int m = 1 << s;
        for (int k = 0; k < fft_size; k += m) {
            float omega_real = 1;
            float omega_imag = 0;
            for (int j = 0; j < (m >> 1); ++j) {
                float temp_real = omega_real * data[2 * k + 2 * j + m] -
                                  omega_imag * data[2 * k + 2 * j + m + 1];
                float temp_imag = omega_real * data[2 * k + 2 * j + m + 1] +
                                  omega_imag * data[2 * k + 2 * j + m];

                data[2 * k + 2 * j + m] = data[2 * k + 2 * j] - temp_real;
                data[2 * k + 2 * j + m + 1] =
                    data[2 * k + 2 * j + 1] - temp_imag;

                data[2 * k + 2 * j] += temp_real;
                data[2 * k + 2 * j + 1] += temp_imag;

                float temp_omega_real = omega_real * twiddle[2 * (s - 1)] -
                                        omega_imag * twiddle[2 * (s - 1) + 1];
                omega_imag = omega_real * twiddle[2 * (s - 1) + 1] +
                             omega_imag * twiddle[2 * (s - 1)];
                omega_real = temp_omega_real;
            }
        }
    }

    if (invert) {
        for (int i = 0; i < (fft_size << 1); ++i) {
            data[i] /= fft_size;
        }
    }
}

void FFT::bit_reverse_(float *data, int fft_size) {
    // Reorder data
    for (int i = 1, irev = 0; i < fft_size; ++i) {
        int bit = fft_size >> 1;
        for (; irev & bit; bit >>= 1) {
            irev ^= bit;
        }
        irev ^= bit;

        if (i < irev) {
            float temp = data[2 * i];
            data[2 * i] = data[2 * irev];
            data[2 * irev] = temp;

            temp = data[2 * i + 1];
            data[2 * i + 1] = data[2 * irev + 1];
            data[2 * irev + 1] = temp;
        }
    }
}

} // namespace fft
