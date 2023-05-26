#include "rfft.h"

#include <math.h>

namespace fft {

RFFTParams::RFFTParams(int rfft_size) { init(rfft_size); }

void RFFTParams::init(int rfft_size) {
    rfft_size_ = rfft_size;
    a_coeffs_ = new float[rfft_size];
    b_coeffs_ = new float[rfft_size];

    int fft_size = rfft_size >> 1;

    for (int i = 0; i < fft_size; ++i) {
        a_coeffs_[2 * i] = 0.5 * (1.0 - sin(i * M_PI / fft_size));
        a_coeffs_[2 * i + 1] = -0.5 * cos(i * M_PI / fft_size);

        b_coeffs_[2 * i] = 0.5 * (1.0 + sin(i * M_PI / fft_size));
        b_coeffs_[2 * i + 1] = 0.5 * cos(i * M_PI / fft_size);
    }
}

RFFTParams::~RFFTParams() {
    if (a_coeffs_) {
        delete[] a_coeffs_;
    }
    if (b_coeffs_) {
        delete[] b_coeffs_;
    }
}

RFFT::~RFFT() {
    if (twiddle_) {
        delete[] twiddle_;
    }

    if (twiddle_inv_) {
        delete[] twiddle_inv_;
    }
}

void RFFT::twiddle_init(int max_rfft_size) {
    FFT::twiddle_init(max_rfft_size >> 1);
}

void RFFT::twiddle_inv_init(int max_rfft_size) {
    FFT::twiddle_inv_init(max_rfft_size >> 1);
}

void RFFT::rfft(float *data, const RFFTParams &params) {
    int rfft_size = params.rfft_size();
    int fft_size = rfft_size >> 1;
    const float *a_coeffs = params.a_coeffs();
    const float *b_coeffs = params.b_coeffs();

    fft_(data, fft_size, false);

    float zero_freq_real = data[0];
    float zero_freq_imag = data[1];

    data[rfft_size] = zero_freq_real;
    data[rfft_size + 1] = zero_freq_imag;

    float split_real, split_imag;
    split_(split_real, split_imag, data, 0, 0, 1, 0, fft_size, fft_size);

    data[fft_size] = split_real;
    data[fft_size + 1] = split_imag;

    for (int i = 0; i < (fft_size >> 1); ++i) {
        int low = 2 * i;
        int high = 2 * fft_size - low;

        float low_split_real, low_split_imag;
        split_(low_split_real, low_split_imag, data, a_coeffs[low],
               a_coeffs[low + 1], b_coeffs[low], b_coeffs[low + 1], low, high);

        float high_split_real, high_split_imag;
        split_(high_split_real, high_split_imag, data, a_coeffs[high],
               a_coeffs[high + 1], b_coeffs[high], b_coeffs[high + 1], high,
               low);

        data[low] = low_split_real;
        data[low + 1] = low_split_imag;

        data[high] = high_split_real;
        data[high + 1] = high_split_imag;
    }

    data[rfft_size] = zero_freq_real - zero_freq_imag;
    data[rfft_size + 1] = 0;
}

void RFFT::irfft(float *data, const RFFTParams &params) {
    int rfft_size = params.rfft_size();
    int fft_size = rfft_size >> 1;
    const float *a_coeffs = params.a_coeffs();
    const float *b_coeffs = params.b_coeffs();

    float split_real, split_imag;
    split_inv_(split_real, split_imag, data, 0, 0, 1, 0, fft_size, fft_size);

    data[rfft_size] = split_real;
    data[rfft_size + 1] = split_imag;

    for (int i = 0; i < (fft_size >> 1); ++i) {
        int low = 2 * i;
        int high = 2 * fft_size - low;

        float low_split_real, low_split_imag;
        split_inv_(low_split_real, low_split_imag, data, a_coeffs[low],
                   a_coeffs[low + 1], b_coeffs[low], b_coeffs[low + 1], low,
                   high);

        float high_split_real, high_split_imag;
        split_inv_(high_split_real, high_split_imag, data, a_coeffs[high],
                   a_coeffs[high + 1], b_coeffs[high], b_coeffs[high + 1], high,
                   low);

        data[low] = low_split_real;
        data[low + 1] = low_split_imag;

        data[high] = high_split_real;
        data[high + 1] = high_split_imag;
    }

    fft_(data, fft_size, true);

    data[rfft_size] = 0;
    data[rfft_size + 1] = 0;
}

void RFFT::split_(float &split_real, float &split_imag, float *data,
                  float a_real, float a_imag, float b_real, float b_imag,
                  int low, int high) {
    split_real = data[low] * a_real - data[low + 1] * a_imag +
                 data[high] * b_real + data[high + 1] * b_imag;
    split_imag = data[low + 1] * a_real + data[low] * a_imag +
                 data[high] * b_imag - data[high + 1] * b_real;
}

void RFFT::split_inv_(float &split_real, float &split_imag, float *data,
                      float a_real, float a_imag, float b_real, float b_imag,
                      int low, int high) {
    split_real = data[low] * a_real + data[low + 1] * a_imag +
                 data[high] * b_real - data[high + 1] * b_imag;
    split_imag = data[low + 1] * a_real - data[low] * a_imag -
                 data[high] * b_imag - data[high + 1] * b_real;
}

} // namespace fft
