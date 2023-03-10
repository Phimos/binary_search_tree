#ifndef NODE_HPP
#define NODE_HPP

#include <algorithm>
#include <cassert>
#include <memory>

enum Direction {
    LEFT = 0,
    RIGHT = 1,
    ROOT = 2
};

template <typename Node>
bool isRoot(const std::shared_ptr<Node>& node) noexcept {
    return node->parent.expired();
}

template <typename Node>
bool isLeftChild(const std::shared_ptr<Node>& node) noexcept {
    return !isRoot(node) && (node->parent.lock()->left == node);
}

template <typename Node>
bool isRightChild(const std::shared_ptr<Node>& node) noexcept {
    return !isRoot(node) && (node->parent.lock()->right == node);
}

template <typename Node>
bool isLeaf(const std::shared_ptr<Node>& node) noexcept {
    return (node->left == nullptr) && (node->right == nullptr);
}

template <typename Node>
bool hasGrandparent(const std::shared_ptr<Node>& node) noexcept {
    return !isRoot(node) && (node->parent.lock()->parent.lock() != nullptr);
}

template <typename Node>
bool hasSibling(const std::shared_ptr<Node>& node) noexcept {
    if (isRoot(node))
        return false;
    return node->parent.lock()->children[getDirection(node) ^ 1] != nullptr;
}

template <typename Node>
bool hasUncle(const std::shared_ptr<Node>& node) noexcept {
    return hasGrandparent(node) && hasSibling(node->parent.lock());
}

template <typename Node>
std::shared_ptr<Node> getGrandparent(const std::shared_ptr<Node>& node) noexcept {
    assert(hasGrandparent(node));
    return node->parent.lock()->parent.lock();
}

template <typename Node>
std::shared_ptr<Node> getSibling(const std::shared_ptr<Node>& node) noexcept {
    if (isRoot(node))
        return nullptr;
    if (isLeftChild(node))
        return node->parent.lock()->right;
    return node->parent.lock()->left;
}

template <typename Node>
std::shared_ptr<Node> getUncle(const std::shared_ptr<Node>& node) noexcept {
    if (!hasGrandparent(node))
        return nullptr;
    return getSibling(node->parent.lock());
}

template <typename Node>
size_t getDepth(const std::shared_ptr<Node>& node) noexcept {
    size_t depth = 0;
    std::shared_ptr<Node> current = node;
    while (!isRoot(current)) {
        current = current->parent.lock();
        ++depth;
    }
    return depth;
}

template <typename Node>
size_t getHeight(const std::shared_ptr<Node>& node) noexcept {
    if (node == nullptr)
        return 0;
    return 1 + std::max(getHeight(node->left), getHeight(node->right));
}

template <typename Node>
Direction getDirection(const std::shared_ptr<Node>& node) noexcept {
    if (isRoot(node))
        return Direction::ROOT;
    return isLeftChild(node) ? Direction::LEFT : Direction::RIGHT;
}

template <typename Node>
std::shared_ptr<Node> getPredecessor(const std::shared_ptr<Node>& node) noexcept {
    if (node->left != nullptr) {
        std::shared_ptr<Node> current = node->left;
        while (current->right)
            current = current->right;
        return current;
    }
    if (node->parent.expired())
        return nullptr;
    std::shared_ptr<Node> current = node->parent.lock();
    while (isLeftChild<Node>(current)) {
        if (current->parent.expired())
            return nullptr;
        current = current->parent.lock();
    }
    return isRightChild(current) ? current->parent.lock() : nullptr;
}

template <typename Node>
std::shared_ptr<Node> getSuccessor(const std::shared_ptr<Node>& node) noexcept {
    if (node->right != nullptr) {
        std::shared_ptr<Node> current = node->right;
        while (current->left)
            current = current->left;
        return current;
    }
    if (node->parent.expired())
        return nullptr;
    std::shared_ptr<Node> current = node->parent.lock();
    while (isRightChild<Node>(current)) {
        if (current->parent.expired())
            return nullptr;
        current = current->parent.lock();
    }
    return isLeftChild(current) ? current->parent.lock() : nullptr;
}

template <typename Node>
void preorderTraversal(const std::shared_ptr<Node>& root, std::function<void(const std::shared_ptr<Node>&)> callback) {
    if (root == nullptr)
        return;

    std::shared_ptr<Node> current = root;
    std::stack<std::shared_ptr<Node>> stack;
    stack.push(current);

    while (!stack.empty()) {
        current = stack.top();
        stack.pop();
        callback(current);
        if (current->right)
            stack.push(current->right);
        if (current->left)
            stack.push(current->left);
    }
}

template <typename Node>
void inorderTraversal(const std::shared_ptr<Node>& root, std::function<void(const std::shared_ptr<Node>&)> callback) {
    if (root == nullptr)
        return;

    std::shared_ptr<Node> current = root;
    std::stack<std::shared_ptr<Node>> stack;

    while (current || !stack.empty()) {
        for (; current; current = current->left)
            stack.push(current);
        current = stack.top();
        stack.pop();
        callback(current);
        current = current->right;
    }
}

template <typename Node>
void postorderTraversal(const std::shared_ptr<Node>& root, std::function<void(const std::shared_ptr<Node>&)> callback) {
    if (root == nullptr)
        return;

    std::shared_ptr<Node> current = root;
    std::stack<std::shared_ptr<Node>> stack;
    std::shared_ptr<Node> lastVisited = nullptr;

    while (current || !stack.empty()) {
        for (; current; current = current->left)
            stack.push(current);
        current = stack.top();
        if (!current->right || current->right == lastVisited) {
            callback(current);
            lastVisited = current;
            stack.pop();
            current = nullptr;
        } else
            current = current->right;
    }
}

#endif  // NODE_HPP