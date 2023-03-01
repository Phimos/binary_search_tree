#ifndef BINART_SEARCH_TREE_HPP
#define BINART_SEARCH_TREE_HPP

#include <cassert>
#include <iostream>
#include <memory>
#include <stack>
#include <vector>

#include "node.hpp"

template <typename T>
struct BinaryNode {
   public:
    T value;
    std::weak_ptr<BinaryNode<T>> parent;
    std::shared_ptr<BinaryNode<T>> children[2];
    std::shared_ptr<BinaryNode<T>>& left = children[0];
    std::shared_ptr<BinaryNode<T>>& right = children[1];
    size_t size, count, repeat;

    BinaryNode() = default;
    BinaryNode(const T& value, size_t repeat = 1)
        : value(std::move(value)), count(repeat), repeat(repeat) {}

    ~BinaryNode() = default;

    inline void update() {
        count = 1 + (left ? left->count : 0) + (right ? right->count : 0);
        size = repeat + (left ? left->size : 0) + (right ? right->size : 0);
    }
};

template <typename T, typename Compare = std::less<T>, typename Node = BinaryNode<T>>
class BinarySearchTree {
   protected:
    std::shared_ptr<Node> root;
    Compare compare = Compare();

    std::shared_ptr<Node> rotateLeft(const std::shared_ptr<Node>& node);
    std::shared_ptr<Node> rotateRight(const std::shared_ptr<Node>& node);
    std::shared_ptr<Node> rotate(const std::shared_ptr<Node>& node, size_t direction);

   public:
    BinarySearchTree() = default;
    BinarySearchTree(const BinarySearchTree&) = delete;
    BinarySearchTree(BinarySearchTree&&) = default;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;
    BinarySearchTree& operator=(BinarySearchTree&&) = default;
    ~BinarySearchTree() = default;

    virtual size_t size() const noexcept { return root ? root->count : 0; }
    virtual bool empty() const noexcept { return root == nullptr; }
    virtual void clear() noexcept { root = nullptr; }
    virtual size_t height() noexcept { return root ? getHeight(root) : 0; }
    virtual void print();

    virtual bool contains(const T& value);
    virtual void insert(const T& value);
    virtual void remove(const T& value);
    virtual size_t rank(const T& value);
    virtual T select(size_t rank);
    virtual T min();
    virtual T max();
    virtual T floor(const T& value);
    virtual T ceil(const T& value);
    virtual std::vector<T> nsmallest(size_t n);
    virtual std::vector<T> nlargest(size_t n);
};

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> BinarySearchTree<T, Compare, Node>::rotateLeft(const std::shared_ptr<Node>& node) {
    assert(node != nullptr && node->right != nullptr);

    auto right = node->right;
    node->right = right->left;
    if (right->left)
        right->left->parent = node;
    right->parent = node->parent;
    if (isRoot(node))
        root = right;
    else if (isLeftChild(node))
        node->parent.lock()->left = right;
    else
        node->parent.lock()->right = right;

    right->left = node;
    node->parent = right;

    node->update();
    right->update();

    return right;
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> BinarySearchTree<T, Compare, Node>::rotateRight(const std::shared_ptr<Node>& node) {
    assert(node != nullptr && node->left != nullptr);

    auto left = node->left;
    node->left = left->right;
    if (left->right)
        left->right->parent = node;
    left->parent = node->parent;
    if (isRoot(node))
        root = left;
    else if (isLeftChild(node))
        node->parent.lock()->left = left;
    else
        node->parent.lock()->right = left;

    left->right = node;
    node->parent = left;

    node->update();
    left->update();

    return left;
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> BinarySearchTree<T, Compare, Node>::rotate(const std::shared_ptr<Node>& node, size_t direction) {
    assert(direction == Direction::LEFT || direction == Direction::RIGHT);
    return direction == Direction::LEFT ? rotateLeft(node) : rotateRight(node);
}

template <typename T, typename Compare, typename Node>
void BinarySearchTree<T, Compare, Node>::print() {
    std::function<void(const std::shared_ptr<Node>&)> printNode = [](const std::shared_ptr<Node>& node) { std::cout << node->value << " "; };
    inorderTraversal(root, printNode);
    std::cout << std::endl;
}

template <typename T, typename Compare, typename Node>
bool BinarySearchTree<T, Compare, Node>::contains(const T& value) {
    std::shared_ptr<Node> current = root;
    while (current) {
        if (!compare(value, current->value) && !compare(current->value, value))
            return true;
        current = current->children[compare(value, current->value)];
    }
    return false;
}

template <typename T, typename Compare, typename Node>
void BinarySearchTree<T, Compare, Node>::insert(const T& value) {
    if (root == nullptr) {
        root = std::make_shared<Node>(value);
        return;
    }
    std::shared_ptr<Node> current = root;
    while (true) {
        if (!compare(value, current->value) && !compare(current->value, value)) {
            ++(current->repeat);
            break;
        }
        size_t dir = compare(current->value, value);
        if (current->children[dir] == nullptr) {
            current->children[dir] = std::make_shared<Node>(value);
            current->children[dir]->parent = current;
            break;
        }
        current = current->children[dir];
    }
    while (current) {
        current->update();
        current = current->parent.lock();
    }
}

template <typename T, typename Compare, typename Node>
void BinarySearchTree<T, Compare, Node>::remove(const T& value) {
    std::shared_ptr<Node> current = root;
    while (current) {
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
                    size_t dir = isRightChild(current);
                    current->parent.lock()->children[dir] = nullptr;
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

                if (isRightChild(successor)) {
                    successor->left = current->left;
                    current->left->parent = successor;
                } else {
                    successor->parent.lock()->left = successor->right;
                    if (successor->right)
                        successor->right->parent = successor->parent;
                    successor->left = current->left;
                    successor->right = current->right;
                    current->left->parent = successor;
                    current->right->parent = successor;
                }
                if (isRoot(current))
                    root = successor;
                else
                    current->parent.lock()->children[isRightChild(current)] = successor;
                successor->parent = current->parent;
                current = successor;
            }
            break;
        }
        current = current->children[compare(current->value, value)];
    }
    while (current) {
        current->update();
        current = current->parent.lock();
    }
}

template <typename T, typename Compare, typename Node>
size_t BinarySearchTree<T, Compare, Node>::rank(const T& value) {
    size_t rank = 0;
    std::shared_ptr<Node> current = root;
    while (current) {
        if (!compare(value, current->value) && !compare(current->value, value))
            return rank + (current->left ? current->left->count : 0) + 1;
        if (compare(value, current->value))
            current = current->left;
        else {
            rank += (current->left ? current->left->count : 0) + current->repeat;
            current = current->right;
        }
    }
    return rank;
}

template <typename T, typename Compare, typename Node>
T BinarySearchTree<T, Compare, Node>::select(size_t rank) {
    std::shared_ptr<Node> current = root;
    while (current) {
        size_t leftCount = current->left ? current->left->count : 0;
        if (rank <= leftCount)
            current = current->left;
        else if (rank <= leftCount + current->repeat)
            return current->value;
        else {
            rank -= leftCount + current->repeat;
            current = current->right;
        }
    }
    return T();
}

template <typename T, typename Compare, typename Node>
T BinarySearchTree<T, Compare, Node>::min() {
    std::shared_ptr<Node> current = root;
    while (current->left)
        current = current->left;
    return current->value;
}

template <typename T, typename Compare, typename Node>
T BinarySearchTree<T, Compare, Node>::max() {
    std::shared_ptr<Node> current = root;
    while (current->right)
        current = current->right;
    return current->value;
}

template <typename T, typename Compare, typename Node>
T BinarySearchTree<T, Compare, Node>::floor(const T& value) {
    std::shared_ptr<Node> current = root;
    std::shared_ptr<Node> result = nullptr;
    while (current) {
        if (!compare(value, current->value) && !compare(current->value, value))
            return current->value;
        if (compare(value, current->value))
            current = current->left;
        else {
            result = current;
            current = current->right;
        }
    }
    return result ? result->value : T();
}

template <typename T, typename Compare, typename Node>
T BinarySearchTree<T, Compare, Node>::ceil(const T& value) {
    std::shared_ptr<Node> current = root;
    std::shared_ptr<Node> result = nullptr;
    while (current) {
        if (!compare(value, current->value) && !compare(current->value, value))
            return current->value;
        if (compare(value, current->value)) {
            result = current;
            current = current->left;
        } else
            current = current->right;
    }
    return result ? result->value : T();
}

template <typename T, typename Compare, typename Node>
std::vector<T> BinarySearchTree<T, Compare, Node>::nsmallest(size_t n) {
    std::vector<T> result;
    if (n == 0 || this->empty())
        return result;

    std::shared_ptr<Node> current = root;
    std::stack<std::shared_ptr<Node>> stack;
    while (current || !stack.empty()) {
        for (; current; current = current->left)
            stack.push(current);
        current = stack.top();
        stack.pop();
        result.push_back(current->value);
        if (result.size() == n)
            break;
        current = current->right;
    }
    return result;
}

template <typename T, typename Compare, typename Node>
std::vector<T> BinarySearchTree<T, Compare, Node>::nlargest(size_t n) {
    std::vector<T> result;
    if (n == 0 || this->empty())
        return result;

    std::shared_ptr<Node> current = root;
    std::stack<std::shared_ptr<Node>> stack;
    while (current || !stack.empty()) {
        for (; current; current = current->right)
            stack.push(current);
        current = stack.top();
        stack.pop();
        result.push_back(current->value);
        if (result.size() == n)
            break;
        current = current->left;
    }
    return result;
}

#endif  // BINART_SEARCH_TREE_HPP