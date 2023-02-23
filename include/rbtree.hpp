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
    size_t count, repeat;
    Color color = Color::RED;

    RBTreeNode() = default;
    RBTreeNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), count(repeat), repeat(repeat) {}

    ~RBTreeNode() = default;
};

template <typename T, typename Compare = std::less<T>, typename Node = RBTreeNode<T>>
class RBTree : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;

   public:
    RBTree() = default;
    RBTree(const RBTree&) = delete;
    RBTree(RBTree&&) = default;
    RBTree& operator=(const RBTree&) = delete;
    RBTree& operator=(RBTree&&) = default;
    ~RBTree() = default;
};

#endif  // RBTREE_HPP