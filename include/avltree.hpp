#ifndef AVL_TREE_HPP
#define AVL_TREE_HPP

#include "binary_search_tree.hpp"

template <typename T>
struct AVLTreeNode {
   public:
    T value;
    std::weak_ptr<AVLTreeNode<T>> parent;
    std::shared_ptr<AVLTreeNode<T>> children[2];
    std::shared_ptr<AVLTreeNode<T>>& left = children[0];
    std::shared_ptr<AVLTreeNode<T>>& right = children[1];
    size_t size, count, repeat, height;

    AVLTreeNode() = default;
    AVLTreeNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), count(repeat), repeat(repeat), height(1) {}

    ~AVLTreeNode() = default;

    inline void update() {
        count = 1 + (left ? left->count : 0) + (right ? right->count : 0);
        size = repeat + (left ? left->size : 0) + (right ? right->size : 0);
        height = 1 + std::max(left ? left->height : 0, right ? right->height : 0);
    }

    inline int factor() const noexcept {
        if (left == nullptr && right == nullptr)
            return 0;
        if (left == nullptr)
            return right->height;
        if (right == nullptr)
            return -left->height;
        return right->height - left->height;
    }
};

template <typename T, typename Compare = std::less<T>, typename Node = AVLTreeNode<T>>
class AVLTree : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;
    using BinarySearchTree<T, Compare, Node>::rotate;
    using BinarySearchTree<T, Compare, Node>::rotateLeft;
    using BinarySearchTree<T, Compare, Node>::rotateRight;

   protected:
    std::shared_ptr<Node> maintain(const std::shared_ptr<Node>& node);
    std::shared_ptr<Node> insert(const std::shared_ptr<Node>& node, const T& value);
    std::shared_ptr<Node> remove(const std::shared_ptr<Node>& node, const T& value);

   public:
    AVLTree() = default;
    AVLTree(const AVLTree&) = delete;
    AVLTree(AVLTree&&) = default;
    AVLTree& operator=(const AVLTree&) = delete;
    AVLTree& operator=(AVLTree&&) = default;
    ~AVLTree() = default;

    void insert(const T& value) override;
    void remove(const T& value) override;
};

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AVLTree<T, Compare, Node>::maintain(const std::shared_ptr<Node>& node) {
    if (node == nullptr)
        return nullptr;
    if (node->factor() < -1) {
        if (node->left->factor() < 0) {
            return rotateRight(node);
        } else {
            rotateLeft(node->left);
            return rotateRight(node);
        }
    } else if (node->factor() > 1) {
        if (node->right->factor() > 0) {
            return rotateLeft(node);
        } else {
            rotateRight(node->right);
            return rotateLeft(node);
        }
    }
    return node;
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AVLTree<T, Compare, Node>::insert(const std::shared_ptr<Node>& node, const T& value) {
    if (node == nullptr) {
        return std::make_shared<Node>(value);
    }
    if (compare(value, node->value)) {
        node->left = insert(node->left, value);
        node->left->parent = node;
    } else if (compare(node->value, value)) {
        node->right = insert(node->right, value);
        node->right->parent = node;
    } else {
        node->repeat++;
    }
    node->update();
    return maintain(node);
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> AVLTree<T, Compare, Node>::remove(const std::shared_ptr<Node>& node, const T& value) {
    if (node == nullptr)
        return nullptr;

    if (compare(value, node->value)) {
        node->left = remove(node->left, value);
        if (node->left != nullptr)
            node->left->parent = node;
    } else if (compare(node->value, value)) {
        node->right = remove(node->right, value);
        if (node->right != nullptr)
            node->right->parent = node;
    } else {
        if (node->repeat > 1) {
            node->repeat--;
        } else if (node->left == nullptr) {
            return node->right;
        } else if (node->right == nullptr) {
            return node->left;
        } else {
            std::shared_ptr<Node> successor = getSuccessor(node);
            std::swap(node->value, successor->value);
            std::swap(node->repeat, successor->repeat);
            node->right = remove(node->right, value);
            if (node->right != nullptr)
                node->right->parent = node;
        }
    }
    node->update();
    return maintain(node);
}

template <typename T, typename Compare, typename Node>
void AVLTree<T, Compare, Node>::insert(const T& value) {
    root = insert(root, value);
    if (root != nullptr)
        root->parent.reset();
}

template <typename T, typename Compare, typename Node>
void AVLTree<T, Compare, Node>::remove(const T& value) {
    root = remove(root, value);
    if (root != nullptr)
        root->parent.reset();
}

#endif  // AVL_TREE_HPP