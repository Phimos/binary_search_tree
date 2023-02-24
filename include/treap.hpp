#ifndef TREAP_HPP
#define TREAP_HPP

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
    using BinarySearchTree<T, Compare, Node>::rotate;

   public:
   public:
    Treap() = default;
    Treap(const Treap&) = delete;
    Treap(Treap&&) = default;
    Treap& operator=(const Treap&) = delete;
    Treap& operator=(Treap&&) = default;
    ~Treap() = default;

    void insert(const T& value) override;
    void remove(const T& value) override;
};

template <typename T, typename Compare, typename Node>
void Treap<T, Compare, Node>::insert(const T& value) {
    if (root == nullptr) {
        root = std::make_shared<Node>(value);
        return;
    }

    std::shared_ptr<Node> current = root;
    while (true) {
        if (!compare(value, current->value) && !compare(current->value, value)) {
            current->repeat++;
            current->update();
            break;
        }
        int direction = compare(current->value, value);
        if (!current->children[direction]) {
            current->children[direction] = std::make_shared<Node>(value);
            current->children[direction]->parent = current;
            current = current->children[direction];
            break;
        }
        current = current->children[direction];
    }

    while (!isRoot(current)) {
        int direction = getDirection(current);
        if (current->priority < current->parent.lock()->priority) {
            rotate(current->parent.lock(), direction ^ 1);
        } else {
            current->update();
            current = current->parent.lock();
        }
    }
    current->update();
}

template <typename T, typename Compare, typename Node>
void Treap<T, Compare, Node>::remove(const T& value) {
    if (root == nullptr)
        return;

    std::shared_ptr<Node> current = root;
    while (true) {
        if (!compare(value, current->value) && !compare(current->value, value)) {
            if (current->repeat > 1) {
                current->repeat--;
                break;
            }

            if (!current->left && !current->right) {
                if (isRoot(current)) {
                    root = nullptr;
                    current = nullptr;
                    break;
                }
                current->parent.lock()->children[getDirection(current)] = nullptr;
                current = current->parent.lock();
                break;
            }

            if (!current->left || !current->right) {
                if (isRoot(current)) {
                    root = current->left ? current->left : current->right;
                    root->parent.reset();
                    current = nullptr;
                    break;
                }
                int direction = getDirection(current);
                current->parent.lock()->children[direction] =
                    current->left ? current->left : current->right;
                current->parent.lock()->children[direction]->parent = current->parent;
                current = current->parent.lock();
                break;
            }

            int direction = current->right->priority > current->left->priority;
            rotate(current, direction);
            continue;
        }
        int direction = compare(current->value, value);
        if (!current->children[direction])
            break;
        current = current->children[direction];
    }

    while (current != nullptr) {
        current->update();
        current = current->parent.lock();
    }
}

#endif  // TREAP_HPP