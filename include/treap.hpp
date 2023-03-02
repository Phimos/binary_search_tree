#ifndef TREAP_HPP
#define TREAP_HPP

#include <chrono>
#include <tuple>

#include "binary_search_tree.hpp"

template <typename T>
struct TreapNode {
   public:
    T value;
    std::weak_ptr<TreapNode<T>> parent;
    std::shared_ptr<TreapNode<T>> children[2];
    std::shared_ptr<TreapNode<T>>& left = children[0];
    std::shared_ptr<TreapNode<T>>& right = children[1];
    size_t size, count, repeat;
    std::uint32_t priority;

    TreapNode() = default;
    TreapNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), count(repeat), repeat(repeat), priority(rand()) {}

    ~TreapNode() = default;

    inline void update() {
        count = 1 + (left ? left->count : 0) + (right ? right->count : 0);
        size = repeat + (left ? left->size : 0) + (right ? right->size : 0);
    }
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

template <typename T, typename Compare, typename Node>
class NonRotatingTreap : BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;

   protected:
    std::shared_ptr<Node> merge(const std::shared_ptr<Node>& left,
                                const std::shared_ptr<Node>& right);
    std::shared_ptr<Node> mergeTriple(const std::shared_ptr<Node>& left,
                                      const std::shared_ptr<Node>& middle,
                                      const std::shared_ptr<Node>& right);
    std::tuple<std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Node>>
    splitByValue(const std::shared_ptr<Node>& current, const T& value);
    std::tuple<std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Node>>
    splitByRank(const std::shared_ptr<Node>& current, size_t rank);

   public:
    NonRotatingTreap() = default;
    NonRotatingTreap(const NonRotatingTreap&) = delete;
    NonRotatingTreap(NonRotatingTreap&&) = default;
    NonRotatingTreap& operator=(const NonRotatingTreap&) = delete;
    NonRotatingTreap& operator=(NonRotatingTreap&&) = default;
    ~NonRotatingTreap() = default;

    void insert(const T& value) override;
    void remove(const T& value) override;
};

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> NonRotatingTreap<T, Compare, Node>::merge(
    const std::shared_ptr<Node>& left,
    const std::shared_ptr<Node>& right) {
    if (left == nullptr || right == nullptr)
        return left ? left : right;

    if (left->priority < right->priority) {
        left->right = merge(left->right, right);
        left->right->parent = left;
        left->update();
        return left;
    } else {
        right->left = merge(left, right->left);
        right->left->parent = right;
        right->update();
        return right;
    }
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> NonRotatingTreap<T, Compare, Node>::mergeTriple(
    const std::shared_ptr<Node>& left,
    const std::shared_ptr<Node>& middle,
    const std::shared_ptr<Node>& right) {
    return merge(merge(left, middle), right);
}

template <typename T, typename Compare, typename Node>
std::tuple<std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Node>>
NonRotatingTreap<T, Compare, Node>::splitByValue(const std::shared_ptr<Node>& current,
                                                 const T& value) {
    if (current == nullptr)
        return std::make_tuple(nullptr, nullptr, nullptr);
    if (compare(current->value, value)) {
        auto [left, middle, right] = splitByValue(current->right, value);
        current->right = left;
        current->right->parent = current;
        current->update();
        return std::make_tuple(current, middle, right);
    } else if (compare(value, current->value)) {
        auto [left, middle, right] = splitByValue(current->left, value);
        current->left = right;
        current->left->parent = current;
        current->update();
        return std::make_tuple(left, middle, current);
    } else {
        return std::make_tuple(current->left, current, current->right);
    }
}

template <typename T, typename Compare, typename Node>
std::tuple<std::shared_ptr<Node>, std::shared_ptr<Node>, std::shared_ptr<Node>>
NonRotatingTreap<T, Compare, Node>::splitByRank(const std::shared_ptr<Node>& current,
                                                size_t rank) {
    if (current == nullptr)
        return std::make_tuple(nullptr, nullptr, nullptr);
    size_t leftSize = current->left ? current->left->size : 0;
    if (leftSize >= rank) {
        auto [left, middle, right] = splitByRank(current->left, rank);
        current->left = right;
        current->left->parent = current;
        current->update();
        return std::make_tuple(left, middle, current);
    } else if (leftSize + current->repeat < rank) {
        auto [left, middle, right] = splitByRank(current->right, rank - leftSize - current->repeat);
        current->right = left;
        current->right->parent = current;
        current->update();
        return std::make_tuple(current, middle, right);
    } else {
        return std::make_tuple(current->left, current, current->right);
    }
}

template <typename T, typename Compare, typename Node>
void NonRotatingTreap<T, Compare, Node>::insert(const T& value) {
    if (root == nullptr) {
        root = std::make_shared<Node>(value);
        return;
    }

    auto [left, middle, right] = splitByValue(root, value);
    if (middle == nullptr) {
        middle = std::make_shared<Node>(value);
    } else {
        middle->repeat++;
    }
    root = mergeTriple(left, middle, right);
}

template <typename T, typename Compare, typename Node>
void NonRotatingTreap<T, Compare, Node>::remove(const T& value) {
    auto [left, middle, right] = splitByValue(root, value);
    if (middle == nullptr) {
        root = merge(left, right);
        return;
    }

    if (middle->repeat > 1) {
        middle->repeat--;
        root = mergeTriple(left, middle, right);
        return;
    }

    root = merge(left, right);
}

#endif  // TREAP_HPP