#ifndef __SPLAY_HPP__
#define __SPLAY_HPP__

#include "binary_search_tree.hpp"

template <typename T, typename Compare = std::less<T>, typename Node = BinaryNode<T>>
class Splay : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;

   private:
    void rotate(std::shared_ptr<Node> node) {
        std::shared_ptr<Node> parent = node->parent.lock();
        std::shared_ptr<Node> grandparent = parent->parent.lock();
        int dir = getDirection(node);

        parent->children[dir] = node->children[dir ^ 1];
        if (node->children[dir ^ 1])
            node->children[dir ^ 1]->parent = parent;
        node->children[dir ^ 1] = parent;
        parent->parent = node;
        node->parent = grandparent;
        if (grandparent)
            grandparent->children[grandparent->right.get() == parent.get()] = node;
        else
            root = node;
        parent->update();
        node->update();
    }

    void splay(std::shared_ptr<Node> node) {
        while (node->parent.lock()) {
            if (node->parent.lock()->parent.lock())
                rotate(getDirection(node) == getDirection(node->parent.lock()) ? node->parent.lock() : node);
            rotate(node);
        }
        root = node;
    }

   public:
    Splay() = default;
    Splay(const Splay&) = delete;
    Splay(Splay&&) = default;
    Splay& operator=(const Splay&) = delete;
    Splay& operator=(Splay&&) = default;
    ~Splay() = default;

    void insert(const T& value) {
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
            int direction = value > node->value;
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

    size_t rank(const T& value) {
        std::shared_ptr<Node> node = root;
        size_t rank = 0;
        while (node) {
            if (compare(value, node->value))
                node = node->left;
            else {
                rank += node->left ? node->left->count : 0;
                if (!compare(node->value, value)) {
                    splay(node);
                    return rank + 1;
                }
                rank += node->repeat;
                node = node->right;
            }
        }
        return rank;
    }

    T select(size_t rank) {
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

    bool contains(const T& value) {
        std::shared_ptr<Node> node = root;
        while (node) {
            if (compare(value, node->value))
                node = node->left;
            else if (compare(node->value, value))
                node = node->right;
            else {
                splay(node);
                return true;
            }
        }
        return false;
    }

    void remove(const T& value) {
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
        if (!root->left) {
            root = root->right;
            root->parent.reset();
            return;
        }
        if (!root->right) {
            root = root->left;
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
};

#endif  // __SPLAY_HPP__