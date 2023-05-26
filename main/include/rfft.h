#pragma once

#include "fft.h"

namespace fft {

class RFFTParams {
  public:
    RFFTParams(int rfft_size);
    ~RFFTParams();

    inline int rfft_size() const { return rfft_size_; }
    const float *a_coeffs() const { return a_coeffs_; }
    const float *b_coeffs() const { return b_coeffs_; }

  private:
    int rfft_size_;
    float *a_coeffs_;
    float *b_coeffs_;
};

class RFFT : public FFT {
  public:
    ~RFFT();

    void twiddle_init(int max_rfft_size);
    void twiddle_inv_init(int max_rfft_size);

    void rfft(float *data, const RFFTParams &params);
    void irfft(float *data, const RFFTParams &params);

  private:
    void split_(float &split_real, float &split_imag, float *data, float a_real,
                float a_imag, float b_real, float b_imag, int low, int high);

    void split_inv_(float &split_real, float &split_imag, float *data,
                    float a_real, float a_imag, float b_real, float b_imag,
                    int low, int high);
};

} // namespace fft
