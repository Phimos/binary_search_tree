#ifndef __TREAP_HPP__
#define __TREAP_HPP__

#include <chrono>

#include "binary_search_tree.hpp"

template <typename T>
struct TreapNode {
   public:
    T value;
    std::weak_ptr<TreapNode<T>> parent;
    std::shared_ptr<TreapNode<T>> children[2];
    std::shared_ptr<TreapNode<T>>& left = children[0];
    std::shared_ptr<TreapNode<T>>& right = children[1];
    size_t count, repeat;
    std::uint32_t priority;

    TreapNode() = default;
    TreapNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), count(repeat), repeat(repeat), priority(rand()) {}

    ~TreapNode() = default;

    inline void update() { count = repeat + (left ? left->count : 0) + (right ? right->count : 0); }
};

template <typename T, typename Compare = std::less<T>, typename Node = TreapNode<T>>
class Treap : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;

   public:
    void rotate(std::shared_ptr<Node>& node, int direction) {
        // direction: 0 - left, 1 - right
        assert(direction == 0 || direction == 1);
        auto child = node->children[direction ^ 1];
        node->children[direction ^ 1] = child->children[direction];
        child->children[direction] = node;
        node->update();
        child->update();
        node = child;
    }

    void insert(std::shared_ptr<Node>& node, const T& value) {
        if (!node) {
            node = std::make_shared<Node>(value);
            return;
        }

        if (!compare(value, node->value) && !compare(node->value, value)) {
            node->repeat++;
            node->update();
            return;
        }

        int direction = compare(node->value, value);
        insert(node->children[direction], value);
        if (node->priority > node->children[direction]->priority)
            rotate(node, direction ^ 1);

        node->update();
    }

    void remove(std::shared_ptr<Node>& node, const T& value) {
        if (!node)
            return;

        if (!compare(value, node->value) && !compare(node->value, value)) {
            if (node->repeat > 1) {
                node->repeat--;
                node->update();
                return;
            }

            if (!node->left && !node->right) {
                node = nullptr;
                return;
            }

            if (!node->left || !node->right) {
                node = node->left ? node->left : node->right;
                return;
            }

            int direction = node->right->priority > node->left->priority;
            rotate(node, direction);
            remove(node->children[direction], value);
        } else {
            int direction = compare(node->value, value);
            remove(node->children[direction], value);
        }
        node->update();
    }

   public:
    Treap() = default;
    Treap(const Treap&) = delete;
    Treap(Treap&&) = default;
    Treap& operator=(const Treap&) = delete;
    Treap& operator=(Treap&&) = default;
    ~Treap() = default;

    void insert(const T& value) { insert(this->root, value); }
    void remove(const T& value) { remove(this->root, value); }
};

#endif  // __TREAP_HPP__