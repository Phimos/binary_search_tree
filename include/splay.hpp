#ifndef SPLAY_HPP
#define SPLAY_HPP

#include "binary_search_tree.hpp"

template <typename T, typename Compare = std::less<T>, typename Node = BinaryNode<T>>
class Splay : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;
    using BinarySearchTree<T, Compare, Node>::rotate;

   private:
    void splay(const std::shared_ptr<Node>& node);

   public:
    Splay() = default;
    Splay(const Splay&) = delete;
    Splay(Splay&&) = default;
    Splay& operator=(const Splay&) = delete;
    Splay& operator=(Splay&&) = default;
    ~Splay() = default;

    bool contains(const T& value) override;
    void insert(const T& value) override;
    void remove(const T& value) override;
    size_t rank(const T& value) override;
    T select(size_t rank) override;
};

template <typename T, typename Compare, typename Node>
void Splay<T, Compare, Node>::splay(const std::shared_ptr<Node>& node) {
    std::shared_ptr<Node> current;
    for (current = node; current->parent.lock() != nullptr; rotate(current->parent.lock(), getDirection(current) ^ 1))
        if (current->parent.lock()->parent.lock())
            rotate(getDirection(current) == getDirection(current->parent.lock()) ? current->parent.lock()->parent.lock() : current->parent.lock(), getDirection(current) ^ 1);
    root = current;
}

template <typename T, typename Compare, typename Node>
bool Splay<T, Compare, Node>::contains(const T& value) {
    std::shared_ptr<Node> node = root;
    while (node) {
        if (!compare(value, node->value) && !compare(node->value, value)) {
            splay(node);
            return true;
        }
        node = node->children[compare(node->value, value)];
    }
    return false;
}

template <typename T, typename Compare, typename Node>
void Splay<T, Compare, Node>::insert(const T& value) {
    if (root == nullptr) {
        root = std::make_shared<Node>(value);
        return;
    }
    std::shared_ptr<Node> node = root;
    while (true) {
        if (!compare(value, node->value) && !compare(node->value, value)) {
            node->repeat++;
            node->update();
            splay(node);
            return;
        }
        int direction = compare(node->value, value);
        if (!node->children[direction]) {
            node->children[direction] = std::make_shared<Node>(value);
            node->children[direction]->parent = node;
            node->update();
            splay(node->children[direction]);
            return;
        }
        node = node->children[direction];
    }
}

template <typename T, typename Compare, typename Node>
void Splay<T, Compare, Node>::remove(const T& value) {
    rank(value);
    if (root->value != value)
        return;
    if (root->repeat > 1) {
        root->repeat--;
        root->update();
        return;
    }
    if (!root->left && !root->right) {
        root = nullptr;
        return;
    }
    if (!root->left || !root->right) {
        root = root->children[root->left == nullptr];
        root->parent.reset();
        return;
    }
    std::shared_ptr<Node> current = root;
    std::shared_ptr<Node> node = root->left;
    while (node->right)
        node = node->right;
    splay(node);
    current->right->parent = node;
    node->right = current->right;
    node->update();
}

template <typename T, typename Compare, typename Node>
size_t Splay<T, Compare, Node>::rank(const T& value) {
    std::shared_ptr<Node> node = root;
    size_t rank = 0;
    while (node) {
        if (!compare(value, node->value) && !compare(node->value, value)) {
            splay(node);
            return rank + (node->left ? node->left->count : 0) + 1;
        }
        if (compare(node->value, value)) {
            rank += (node->left ? node->left->count : 0) + node->repeat;
            node = node->right;
        } else
            node = node->left;
    }
    return rank;
}

template <typename T, typename Compare, typename Node>
T Splay<T, Compare, Node>::select(size_t rank) {
    assert(rank <= this->size());
    std::shared_ptr<Node> node = root;
    while (node) {
        size_t left_count = node->left ? node->left->count : 0;
        if (rank <= left_count)
            node = node->left;
        else if (rank <= left_count + node->repeat) {
            splay(node);
            return node->value;
        } else {
            rank -= left_count + node->repeat;
            node = node->right;
        }
    }
    return T();
}

#endif  // SPLAY_HPP