#include "iir_filter.h"

#include <math.h>

#include "biquad.h"
#include "memory.h"

IIRFilter::IIRFilter(int num_stages)
    : num_stages_(num_stages), stages_(new Biquad *[num_stages]), gain_(1) {}

IIRFilter::~IIRFilter() {
    if (stages_) {
        delete[] stages_;
    }
}

void IIRFilter::set_stage(int idx, Biquad *stage) { stages_[idx] = stage; }

Biquad *IIRFilter::get_stage(int idx) { return stages_[idx]; }

const Biquad *IIRFilter::get_stage(int idx) const { return stages_[idx]; }

float IIRFilter::filter(float x) {
    float y = x;
    for (int idx = 0; idx < num_stages_; ++idx) {
        y = stages_[idx]->filter(y);
    }
    return gain_ * y;
}
