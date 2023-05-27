#pragma once

#include "fft.h"

namespace fft {

class RFFTParams {
  public:
    RFFTParams() {}
    RFFTParams(int rfft_size);
    ~RFFTParams();

    void init(int rfft_size);

    inline int rfft_size() const { return rfft_size_; }
    const float *a_coeffs() const { return a_coeffs_; }
    const float *b_coeffs() const { return b_coeffs_; }

  private:
    int rfft_size_;
    float *a_coeffs_;
    float *b_coeffs_;
};

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
