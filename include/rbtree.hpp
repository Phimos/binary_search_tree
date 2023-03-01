#ifndef RBTREE_HPP
#define RBTREE_HPP

#include <algorithm>
#include <functional>
#include <memory>

#include "binary_search_tree.hpp"

enum Color {
    RED = 0,
    BLACK = 1
};

template <typename T>
struct RBTreeNode {
   public:
    T value;
    std::weak_ptr<RBTreeNode<T>> parent;
    std::shared_ptr<RBTreeNode<T>> children[2];
    std::shared_ptr<RBTreeNode<T>>& left = children[0];
    std::shared_ptr<RBTreeNode<T>>& right = children[1];
    size_t size, count, repeat;
    Color color = Color::RED;

    RBTreeNode() = default;
    RBTreeNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), count(repeat), repeat(repeat) {}

    ~RBTreeNode() = default;

    inline void update() {
        count = 1 + (left ? left->count : 0) + (right ? right->count : 0);
        size = repeat + (left ? left->size : 0) + (right ? right->size : 0);
    }
};

template <typename T, typename Compare = std::less<T>, typename Node = RBTreeNode<T>>
class RBTree : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;
    using BinarySearchTree<T, Compare, Node>::rotateLeft;
    using BinarySearchTree<T, Compare, Node>::rotateRight;
    using BinarySearchTree<T, Compare, Node>::rotate;

   public:
    RBTree() = default;
    RBTree(const RBTree&) = delete;
    RBTree(RBTree&&) = default;
    RBTree& operator=(const RBTree&) = delete;
    RBTree& operator=(RBTree&&) = default;
    ~RBTree() = default;

    void insert(const T& value) override;
    void remove(const T& value) override;
};

template <typename T, typename Compare, typename Node>
void RBTree<T, Compare, Node>::insert(const T& value) {
    if (root == nullptr) {
        root = std::make_shared<Node>(value);
        root->color = Color::BLACK;
        return;
    }

    std::shared_ptr<Node> node = root;
    std::shared_ptr<Node> parent = nullptr;
    while (node != nullptr) {
        parent = node;
        if (compare(value, node->value)) {
            node = node->left;
        } else if (compare(node->value, value)) {
            node = node->right;
        } else {
            ++(node->repeat);
            node->update();
            break;
        }
    }

    if (node != nullptr) {
        for (; parent != nullptr; parent = parent->parent.lock())
            parent->update();
        return;
    }

    node = std::make_shared<Node>(value);
    parent->children[compare(parent->value, value)] = node;
    node->parent = parent;

    if (parent == root) {
        root->color = Color::BLACK;
        root->update();
        return;
    }

    std::shared_ptr<Node> grandparent, uncle;

    while (parent->color == Color::RED) {
        grandparent = getGrandparent(node);
        uncle = getUncle(node);
        if (uncle != nullptr && uncle->color == Color::RED) {
            parent->color = uncle->color = Color::BLACK;
            grandparent->color = Color::RED;
            parent->update();
            grandparent->update();
            node = grandparent;
            parent = node->parent.lock();
            if (parent == nullptr) {
                node->color = Color::BLACK;
                return;
            }
        } else {
            if (compare(parent->value, grandparent->value) != compare(value, parent->value)) {
                rotate(parent, compare(value, parent->value));
                rotate(grandparent, compare(parent->value, grandparent->value));
                std::swap(node->color, grandparent->color);
            } else {
                rotate(grandparent, compare(parent->value, grandparent->value));
                std::swap(parent->color, grandparent->color);
            }
            for (; parent != nullptr; parent = parent->parent.lock())
                parent->update();
            return;
        }
    }
}

template <typename T, typename Compare, typename Node>
void RBTree<T, Compare, Node>::remove(const T& value) {
    std::shared_ptr<Node> node = root;
    while (node != nullptr) {
        if (compare(value, node->value))
            node = node->left;
        else if (compare(node->value, value))
            node = node->right;
        else
            break;
    }

    if (node == nullptr)
        return;

    if (node->repeat > 1) {
        --(node->repeat);
        for (; node != nullptr; node = node->parent.lock())
            node->update();
        return;
    }
}
#endif  // RBTREE_HPP