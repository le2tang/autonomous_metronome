#pragma once

template <typename HeapType, typename type_t> class BinaryHeap {
  public:
    BinaryHeap() {}
    BinaryHeap(int size) { init(size); }
    ~BinaryHeap() {
        if (heap_) {
            delete[] heap_;
        }
        if (val_) {
            delete[] val_;
        }
    }

    void init(int size) {
        idx_ = 0;
        size_ = size;

        heap_ = new type_t *[size];
        val_ = new type_t[size];
    }

    void insert(type_t new_val) {
        if (full()) {
            return;
        }

        val_[idx_] = new_val;
        heap_[idx_] = &val[idx];

        int curr = idx_;
        int prev = parent_(curr);
        while (curr > 0 && !heap_condition_(curr, prev)) {
            swap_(curr, prev);
        }

        ++idx_;
    }

    type_t pop() { remove(0); }

    type_t remove(int rem_idx) {
        swap_(rem_idx, idx_);
        --idx_;

        heapify_(rem_idx);
    }

    type_t peek() { return at_(0); }

    inline bool full() const { return idx_ == size_; }
    inline bool empty() const { return idx_ == 0; }

  protected:
    inline int parent_(int idx) const { return (idx - 1) / 2; }
    inline int left_(int idx) const { return 2 * idx + 1; }
    inline int right_(int idx) const { return 2 * idx + 2; }
    inline bool is_leaf_(int idx) const {
        return right_(idx) < size_ || left_(idx) < size_;
    }
    inline type_t at_(int idx) const { return *(heap_[idx]); }
    inline type_t &at(int idx) { return *(heap_[idx]); }

    inline bool heap_condition_(int curr_idx, int prev_idx) const {
        static_cast<HeapType &>(*this).heap_condition_(at_(curr_idx),
                                                       at_(prev_idx));
    }
    inline bool heap_condition_(float curr_val, float prev_val) const {
        static_cast<HeapType &>(*this).heap_condition_(curr_val, prev_val);
    }

    void heapify_(int idx) {
        int curr = idx;
        int left = left_(idx);
        int right = right_(idx);
        while (!is_leaf_(curr) && !heap_condition_(curr, left) &&
               !heap_condition_(curr, right)) {
            if heap_condition_ (left, right) {
                swap_(curr, right);
            } else {
                swap_(curr, left);
            }
        }
    }

    void swap_(int idx1, int idx2) {
        type_t *temp = heap_[idx1];
        heap_[idx1] = heap_[idx2];
        heap_[idx2] = temp;
    }

    type_t **heap_;
    type_t *val_;
    int idx_;
    int size_;
};

template <typename type_t>
class BinaryMaxHeap : BinaryHeap<BinaryMaxHeap, type_t> {
  private:
    inline bool heap_condition_(float curr_val, float prev_val) const {
        return curr_val < prev_val;
    }
};

template <typename type_t>
class BinaryMinHeap : BinaryHeap<BinaryMinHeap, type_t> {
  private:
    inline bool heap_condition_(float curr_val, float prev_val) const {
        return curr_val > prev_val;
    }
};
