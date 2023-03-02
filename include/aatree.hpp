#ifndef AA_TREE_HPP
#define AA_TREE_HPP

#include "binary_search_tree.hpp"

template <typename T>
class AATreeNode {
   public:
    T value;
    std::weak_ptr<AATreeNode<T>> parent;
    std::shared_ptr<AATreeNode<T>> children[2];
    std::shared_ptr<AATreeNode<T>>& left = children[0];
    std::shared_ptr<AATreeNode<T>>& right = children[1];
    size_t size, count, repeat, level;

    AATreeNode() = default;
    AATreeNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), size(repeat), count(repeat), repeat(repeat), level(1) {}

    ~AATreeNode() = default;

    inline void update() {
        count = 1 + (left ? left->count : 0) + (right ? right->count : 0);
        size = repeat + (left ? left->size : 0) + (right ? right->size : 0);
    }
};

template <typename T, typename Compare = std::less<T>, typename Node = AATreeNode<T>>
class AATree : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;
    using BinarySearchTree<T, Compare, Node>::rotateLeft;
    using BinarySearchTree<T, Compare, Node>::rotateRight;
    using BinarySearchTree<T, Compare, Node>::rotate;

   protected:
    std::shared_ptr<Node> skew(const std::shared_ptr<Node>& node);
    std::shared_ptr<Node> split(const std::shared_ptr<Node>& node);
    std::shared_ptr<Node> decreaseLevel(const std::shared_ptr<Node>& node);

    std::shared_ptr<Node> insert(std::shared_ptr<Node> node, const T& value);
    std::shared_ptr<Node> remove(std::shared_ptr<Node> node, const T& value);

   public:
    AATree() = default;
    AATree(const AATree&) = delete;
    AATree(AATree&&) = default;
    AATree& operator=(const AATree&) = delete;
    AATree& operator=(AATree&&) = default;
    ~AATree() = default;

    void insert(const T& value) override;
    void remove(const T& value) override;
};

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AATree<T, Compare, Node>::skew(const std::shared_ptr<Node>& node) {
    if (node == nullptr || node->left == nullptr)
        return node;
    if (node->left->level != node->level)
        return node;
    return rotateRight(node);
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AATree<T, Compare, Node>::split(const std::shared_ptr<Node>& node) {
    if (node == nullptr || node->right == nullptr || node->right->right == nullptr)
        return node;
    if (node->right->right->level != node->level)
        return node;
    (node->right->level)++;
    return rotateLeft(node);
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AATree<T, Compare, Node>::decreaseLevel(const std::shared_ptr<Node>& node) {
    if (node == nullptr)
        return node;
    size_t minLevel = std::min(node->left ? node->left->level : node->level,
                               node->right ? node->right->level : node->level) +
                      1;
    if (minLevel >= node->level)
        return node;
    node->level = minLevel;
    if (node->right && node->right->level > node->level)
        node->right->level = node->level;
    return node;
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AATree<T, Compare, Node>::insert(std::shared_ptr<Node> node, const T& value) {
    if (node == nullptr)
        return std::make_shared<Node>(value);
    if (compare(value, node->value)) {
        node->left = insert(node->left, value);
        if (node->left)
            node->left->parent = node;
    } else if (compare(node->value, value)) {
        node->right = insert(node->right, value);
        if (node->right)
            node->right->parent = node;
    } else {
        node->repeat++;
    }

    node->update();
    node = skew(node);
    node = split(node);
    return node;
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AATree<T, Compare, Node>::remove(std::shared_ptr<Node> node, const T& value) {
    if (node == nullptr)
        return node;
    if (compare(value, node->value)) {
        node->left = remove(node->left, value);
        if (node->left)
            node->left->parent = node;
    } else if (compare(node->value, value)) {
        node->right = remove(node->right, value);
        if (node->right)
            node->right->parent = node;
    } else {
        if (node->repeat > 1)
            node->repeat--;
        else if (node->left == nullptr && node->right == nullptr)
            return nullptr;
        else if (node->left == nullptr || node->right == nullptr)
            return node->left ? node->left : node->right;
        else {
            std::shared_ptr<Node> successor = getSuccessor(node);
            std::swap(node->value, successor->value);
            node->right = remove(node->right, value);
            if (node->right)
                node->right->parent = node;
        }
    }

    node->update();
    node = decreaseLevel(node);
    node = skew(node);
    node->right = skew(node->right);
    if (node->right)
        node->right->right = skew(node->right->right);
    node = split(node);
    node->right = split(node->right);
    return node;
}

template <typename T, typename Compare, typename Node>
void AATree<T, Compare, Node>::insert(const T& value) {
    root = insert(root, value);
    if (root)
        root->parent.reset();
}

template <typename T, typename Compare, typename Node>
void AATree<T, Compare, Node>::remove(const T& value) {
    root = remove(root, value);
    if (root)
        root->parent.reset();
}

#endif  // AA_TREE_HPP
