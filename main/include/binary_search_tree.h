#pragma once

#include <math.h>

struct FloatEqual {
    bool operator()(const float &f1, const float &f2) const {
        return fabs(f1 - f2) < 1E-9;
    }
};

template <typename key_t> struct BinarySearchTreeNode {
    BinarySearchTreeNode() : key(), left(nullptr), right(nullptr) {}
    BinarySearchTreeNode(key_t new_key)
        : key(new_key), left(nullptr), right(nullptr) {}

    inline bool is_leaf() const { return left == nullptr && right == nullptr; }

    key_t key;

    struct BinarySearchTreeNode<key_t> *left;
    struct BinarySearchTreeNode<key_t> *right;
};

template <typename key_t, class BinaryPredicate> class BinarySearchTree {
  public:
    typedef BinarySearchTreeNode<key_t> node_t;

    BinarySearchTree() : root_(nullptr), size_(0) {}
    ~BinarySearchTree() {}

    void insert(node_t *new_node) {
        new_node->left = nullptr;
        new_node->right = nullptr;
        insert_node_(new_node);
    }

    node_t *search(key_t query_key) const {
        node_t *curr = root_;
        while (curr) {
            if (equal_(query_key, curr->key)) {
                return curr;
            } else if (query_key > curr->key) {
                curr = curr->right;
            } else {
                curr = curr->left;
            }
        }
        return nullptr;
    }

    bool remove(key_t del_key) {
        node_t *del_prev;
        node_t *del_node = search_with_prev_(del_key, &del_prev);
        return remove_node_(del_node, del_prev);
    }

    bool remove(node_t *del_node) {
        if (!del_node) {
            return false;
        }
        return remove(del_node->key);
    }

    node_t *max() const { return max_from_node_(root_); }
    node_t *min() const { return min_from_node_(root_); }

    inline int size() const { return size_; }

  private:
    void insert_node_(node_t *new_node) {
        if (!root_) {
            root_ = new_node;
        } else {
            node_t *prev = root_;
            node_t *curr = root_;
            while (curr) {
                prev = curr;
                if (new_node->key > curr->key) {
                    curr = curr->right;
                } else {
                    curr = curr->left;
                }
            }

            if (new_node->key > prev->key) {
                prev->right = new_node;
            } else {
                prev->left = new_node;
            }
        }

        ++size_;
    }

    node_t *search_with_prev_(key_t query_key, node_t **prev) const {
        *prev = nullptr;
        node_t *curr = root_;
        while (curr) {
            if (equal_(query_key, curr->key)) {
                return curr;
            } else if (query_key > curr->key) {
                *prev = curr;
                curr = curr->right;
            } else {
                *prev = curr;
                curr = curr->left;
            }
        }
        return nullptr;
    }

    bool remove_node_(node_t *del_node, node_t *del_prev) {
        if (!del_node || !root_) {
            return false;
        }

        if (del_node->is_leaf()) {
            if (del_node == root_) {
                root_ = nullptr;
            } else if (del_node == del_prev->left) {
                del_prev->left = nullptr;
            } else {
                del_prev->right = nullptr;
            }
        } else if (del_node->left) {
            node_t *prev = nullptr;
            node_t *curr = del_node->left;
            while (curr->right) {
                prev = curr;
                curr = curr->right;
            }

            if (prev) {
                prev->right = curr->left;
                curr->left = del_node->left;
                curr->right = del_node->right;
            } else {
                curr->right = del_node->right;
            }

            if (del_node == root_) {
                root_ = curr;
            } else if (del_node == del_prev->left) {
                del_prev->left = curr;
            } else {
                del_prev->right = curr;
            }
        } else {
            node_t *prev = nullptr;
            node_t *curr = del_node->right;
            while (curr->left) {
                prev = curr;
                curr = curr->left;
            }

            if (prev) {
                prev->left = curr->right;
                curr->left = del_node->left;
                curr->right = del_node->right;
            } else {
                curr->left = del_node->left;
            }

            if (del_node == root_) {
                root_ = curr;
            } else if (del_node == del_prev->left) {
                del_prev->left = curr;
            } else {
                del_prev->right = curr;
            }
        }
        --size_;
        delete del_node;
        return true;
    }

    node_t *max_from_node_(node_t *node) const {
        node_t *curr = node;
        while (curr->right) {
            curr = curr->right;
        }
        return curr;
    }

    node_t *min_from_node_(node_t *node) const {
        node_t *curr = node;
        while (curr->left) {
            curr = curr->left;
        }
        return curr;
    }

    node_t *root_;
    int size_;

    BinaryPredicate equal_;
};
