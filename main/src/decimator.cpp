#include "decimator.h"

#include "design_iir.h"
#include "iir_filter.h"

Decimator::~Decimator() {
    if (antialias_filter_) {
        delete antialias_filter_->get_stage(0);
        delete antialias_filter_;
    }
}

void Decimator::init(const DecimatorParams &params) {
    idx_ = 0;
    factor_ = params.decimation_factor;

    antialias_filter_ = new IIRFilter(1);
    antialias_filter_->set_stage(
        0, design_butterlp(0.5 * params.sample_rate / params.decimation_factor,
                           params.sample_rate));
}

float Decimator::update(float sample, bool &ready) {
    float y = antialias_filter_->filter(sample);

    ++idx_;
    if (idx_ >= factor_) {
        ready = true;
        idx_ = 0;
    }

    return y;
}
