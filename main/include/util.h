#pragma once

#include <cstddef>

#include "memory.h"

struct TempoEstimate {
    float rate;
    float phase;
};

#ifdef __cplusplus
extern "C" {
#endif

struct Buffer {
    Buffer() : data(NULL), curr_idx(0), num_samples(0) {}
    Buffer(size_t size) { init(size); }

    ~Buffer() {
        if (data) {
            delete[] data;
        }
    }

    void init(size_t size) {
        data = new float[size];
        curr_idx = 0;
        num_samples = size;
    }

    inline float &operator[](size_t idx) { return data[idx]; }

    inline float operator[](size_t idx) const { return data[idx]; }

    inline void push(float val) {
        data[curr_idx] = val;
        ++curr_idx;
    }

    inline void reset() { curr_idx = 0; }

    inline bool empty() const { return curr_idx == 0; }
    inline bool full() const { return curr_idx == num_samples; }
    inline int count() const { return curr_idx; }
    inline int size() const { return num_samples; }

    float *data;
    size_t curr_idx;
    size_t num_samples;
};

#ifdef __cplusplus
}
#endif
