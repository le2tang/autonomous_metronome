#pragma once

namespace fft {

class FFT {
  public:
    FFT() : twiddle_(0), twiddle_inv_(0) {}
    ~FFT();

    void twiddle_init(int max_fft_size);
    void twiddle_inv_init(int max_fft_size);

    void fft(float *data, int fft_size);
    void ifft(float *data, int fft_size);

  protected:
    void fft_(float *data, int fft_size, bool invert);
    void bit_reverse_(float *data, int fft_size);

    float *twiddle_;
    float *twiddle_inv_;
};

} // namespace fft
