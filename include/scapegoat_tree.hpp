#ifndef SCAPEGOAT_TREE_HPP
#define SCAPEGOAT_TREE_HPP

#include "binary_search_tree.hpp"

template <typename T, typename Compare = std::less<T>, typename Node = BinaryNode<T>>
class ScapegoatTree : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;

   protected:
    double alpha = 0.75;
    bool isUnbalanced(std::shared_ptr<Node>& node);
    std::shared_ptr<Node> rebuild(std::shared_ptr<Node>& node);
    void maintain(const T& value);

   public:
    ScapegoatTree() = default;
    ScapegoatTree(const ScapegoatTree&) = delete;
    ScapegoatTree(ScapegoatTree&&) = default;
    ScapegoatTree& operator=(const ScapegoatTree&) = delete;
    ScapegoatTree& operator=(ScapegoatTree&&) = default;
    ~ScapegoatTree() = default;

    void insert(const T& value) override;
};

template <typename T, typename Compare, typename Node>
bool ScapegoatTree<T, Compare, Node>::isUnbalanced(std::shared_ptr<Node>& node) {
    if (node == nullptr)
        return true;
    size_t size = node->count;
    size_t leftSize = node->left ? node->left->count : 0;
    size_t rightSize = node->right ? node->right->count : 0;
    // std::cout << "Left size: " << leftSize << std::endl;
    // std::cout << "Right size: " << rightSize << std::endl;
    // std::cout << "Alpha * size: " << alpha * size << std::endl;
    return std::max(leftSize, rightSize) > alpha * size;
}

template <typename T, typename Compare, typename Node>
std::shared_ptr<Node> ScapegoatTree<T, Compare, Node>::rebuild(std::shared_ptr<Node>& node) {
    // std::cout << "Rebuilding..." << std::endl;
    // std::cout << "Node: " << node->value << std::endl;
    std::vector<std::shared_ptr<Node>> nodes;
    std::function<void(const std::shared_ptr<Node>&)> collect = [&](const std::shared_ptr<Node>& node) -> void { nodes.push_back(node); };
    inorderTraversal(node, collect);
    std::function<std::shared_ptr<Node>(int, int)> build = [&](int left, int right) {
        if (left > right)
            return std::shared_ptr<Node>(nullptr);
        int mid = (left + right) >> 1;
        nodes[mid]->left = build(left, mid - 1);
        nodes[mid]->right = build(mid + 1, right);
        if (nodes[mid]->left)
            nodes[mid]->left->parent = nodes[mid];
        if (nodes[mid]->right)
            nodes[mid]->right->parent = nodes[mid];
        nodes[mid]->update();
        return nodes[mid];
    };
    return build(0, nodes.size() - 1);
}

template <typename T, typename Compare, typename Node>
void ScapegoatTree<T, Compare, Node>::maintain(const T& value) {
    std::shared_ptr<Node> current = root;
    while (current) {
        if (isUnbalanced(current)) {
            std::shared_ptr<Node> parent = current->parent.lock();
            current = rebuild(current);
            current->parent = parent;
            if (parent == nullptr)
                root = current;
            else
                parent->children[compare(parent->value, current->value)] = current;
            break;
        }
        current = current->children[compare(current->value, value)];
    }
}

template <typename T, typename Compare, typename Node>
void ScapegoatTree<T, Compare, Node>::insert(const T& value) {
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
    maintain(value);
}

#endif  // SCAPEGOAT_TREE_HPP