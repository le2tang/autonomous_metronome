#pragma once

#include "binary_search_tree.h"

class MedianFilter {
  public:
    MedianFilter() : size_(0) {}
    ~MedianFilter();

    void init(int size);

    void update(float val);
    inline float get_median() const { return median_; }

  private:
    BinarySearchTree<float, FloatEqual> lower_;
    BinarySearchTree<float, FloatEqual> upper_;

    BinarySearchTreeNode<float> **window_;

    float median_;

    int idx_;
    int size_;
};
