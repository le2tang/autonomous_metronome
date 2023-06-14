#include "median_filter.h"

MedianFilter::~MedianFilter() {
    if (window_) {
        for (int idx = 0; idx < size_; ++idx) {
            delete window_[idx];
        }
        delete[] window_;
    }
}

void MedianFilter::init(int size) {
    idx_ = 0;
    size_ = size;

    window_ = new BinarySearchTreeNode<float> *[size];
    for (int idx = 0; idx < size; ++idx) {
        window_[idx] = new BinarySearchTreeNode<float>(0);
        if (idx < (size / 2)) {
            lower_.insert(window_[idx]);
        } else {
            upper_.insert(window_[idx]);
        }
    }
}

void MedianFilter::update(float val) {
    BinarySearchTreeNode<float> *node = window_[idx_];

    if (!lower_.remove(node)) {
        upper_.remove(node);
    }

    BinarySearchTreeNode<float> *new_node =
        new BinarySearchTreeNode<float>(val);
    if (lower_.size() <= upper_.size()) {
        // Upper full
        if (val > upper_.min()->key) {
            // In upper
            BinarySearchTreeNode<float> *min_node = upper_.min();
            float min_val = min_node->key;
            upper_.remove(min_node);

            min_node = new BinarySearchTreeNode<float>(min_val);
            lower_.insert(min_node);

            upper_.insert(new_node);

        } else {
            // In lower
            lower_.insert(new_node);
        }
    } else {
        // Lower full
        if (val > upper_.min()->key) {
            // In upper
            upper_.insert(new_node);
        } else {
            // In lower
            BinarySearchTreeNode<float> *max_node = lower_.max();
            float max_val = max_node->key;
            lower_.remove(max_node);

            max_node = new BinarySearchTreeNode<float>(max_val);
            upper_.insert(max_node);

            lower_.insert(new_node);
        }
    }

    window_[idx_] = new_node;
    ++idx_;
    if (idx_ == size_) {
        idx_ = 0;
    }

    median_ = lower_.max()->key;
}
