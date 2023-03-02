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
void AATree<T, Compare, Node>::insert(const T& value) {
    if (root == nullptr) {
        root = std::make_shared<Node>(value);
        return;
    }

    std::shared_ptr<Node> node = root;
    while (true) {
        assert(node->parent.lock() != node);
        if (compare(value, node->value)) {
            if (node->left == nullptr) {
                node->left = std::make_shared<Node>(value);
                node->left->parent = node;
                node = node->left;
                break;
            }
            node = node->left;
        } else if (compare(node->value, value)) {
            if (node->right == nullptr) {
                node->right = std::make_shared<Node>(value);
                node->right->parent = node;
                node = node->right;
                break;
            }
            node = node->right;
        } else {
            node->repeat++;
            node->update();
            break;
        }
    }

    while (node != nullptr) {
        node = skew(node);
        node = split(node);
        node->update();
        node = node->parent.lock();
    }
}

template <typename T, typename Compare, typename Node>
void AATree<T, Compare, Node>::remove(const T& value) {
    if (root == nullptr)
        return;

    std::shared_ptr<Node> current = root;
    while (true) {
        if (!compare(value, current->value) && !compare(current->value, value)) {
            if (current->repeat > 1) {
                --(current->repeat);
                break;
            }
            if (current->left == nullptr && current->right == nullptr) {
                if (isRoot(current)) {
                    root = nullptr;
                    current = nullptr;
                } else {
                    current->parent.lock()->children[isRightChild(current)] = nullptr;
                    current = current->parent.lock();
                }
            } else if (current->left == nullptr) {
                if (isRoot(current))
                    root = current->right;
                else
                    current->parent.lock()->children[isRightChild(current)] = current->right;
                current->right->parent = current->parent;
                current = current->right;
            } else if (current->right == nullptr) {
                if (isRoot(current))
                    root = current->left;
                else
                    current->parent.lock()->children[isRightChild(current)] = current->left;
                current->left->parent = current->parent;
                current = current->left;
            } else {
                std::shared_ptr<Node> successor = getSuccessor(current);
                std::shared_ptr<Node> replacement;
                if (isRightChild(successor)) {
                    successor->left = current->left;
                    current->left->parent = successor;
                    replacement = successor;
                } else {
                    successor->parent.lock()->left = successor->right;
                    if (successor->right)
                        successor->right->parent = successor->parent;
                    successor->left = current->left;
                    successor->right = current->right;
                    current->left->parent = successor;
                    current->right->parent = successor;
                    replacement = successor->parent.lock();
                }
                if (isRoot(current))
                    root = successor;
                else
                    current->parent.lock()->children[isRightChild(current)] = successor;
                successor->parent = current->parent;
                successor->level = current->level;
                current = replacement;
            }
            break;
        }
        current = current->children[compare(current->value, value)];
    }

    std::cout << "rebalancing" << std::endl;

    while (current != nullptr) {
        this->check();
        assert(current->parent.lock() == nullptr || current->parent.lock()->parent.lock() != current->parent.lock());
        current = decreaseLevel(current);
        assert(current->parent.lock() == nullptr || current->parent.lock()->parent.lock() != current->parent.lock());
        current = skew(current);
        assert(current->parent.lock() == nullptr || current->parent.lock()->parent.lock() != current->parent.lock());
        current->right = skew(current->right);
        assert(current->parent.lock() == nullptr || current->parent.lock()->parent.lock() != current->parent.lock());
        if (current->right)
            current->right->right = skew(current->right->right);
        current = split(current);
        current->right = split(current->right);
        current->update();
        std::cout << "current: " << current->value << std::endl;
        if (current->parent.lock())
            std::cout << "current->parent: " << current->parent.lock()->value << std::endl;
        current = current->parent.lock();
    }
}

#endif  // AA_TREE_HPP
