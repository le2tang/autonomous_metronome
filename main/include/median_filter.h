#pragma once

#include "binary_search_tree.h"

class MedianFilter {
  public:
    MedianFilter() : size_(0) {}
    ~MedianFilter() {
        if (window_) {
            for (int idx = 0; idx < size_; ++idx) {
                delete window_[idx];
            }
            delete[] window_;
        }
    }

    void init(int size) {
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

    float update(float val) {
        BinarySearchTreeNode<float> *node = window_[idx_];

        if (!lower_.remove(node->key)) {
            upper_.remove(node->key);
        }

        BinarySearchTreeNode<float> *new_node =
            new BinarySearchTreeNode<float>(val);
        if (lower_.size() <= upper_.size()) {
            // Upper full
            if (val > upper_.min()->key) {
                // In upper
                float min_val = upper_.min()->key;
                upper_.remove(min_val);
                BinarySearchTreeNode<float> *min_node =
                    new BinarySearchTreeNode<float>(min_val);
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
                float max_val = lower_.max()->key;
                BinarySearchTreeNode<float> *max_node =
                    new BinarySearchTreeNode<float>(max_val);
                lower_.remove(max_val);
                upper_.insert(max_node);

                lower_.insert(new_node);
            }
        }

        window_[idx_] = new_node;
        ++idx_;
        if (idx_ == size_) {
            idx_ = 0;
        }

        return lower_.max()->key;
    }

    float median() { return lower_.max()->key; }

  private:
    BinarySearchTree<float, FloatEqual> lower_;
    BinarySearchTree<float, FloatEqual> upper_;

    BinarySearchTreeNode<float> **window_;

    int idx_;
    int size_;
};
