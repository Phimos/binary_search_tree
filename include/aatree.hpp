#ifndef AA_TREE_HPP
#define AA_TREE_HPP

#include "binary_search_tree.hpp"
#include "rbtree.hpp"

template <typename T, typename Compare = std::less<T>, typename Node = RBTreeNode<T>>
class AATree : public BinarySearchTree<T, Compare, Node> {
    using BinarySearchTree<T, Compare, Node>::root;
    using BinarySearchTree<T, Compare, Node>::compare;
    using BinarySearchTree<T, Compare, Node>::rotateLeft;
    using BinarySearchTree<T, Compare, Node>::rotateRight;
    using BinarySearchTree<T, Compare, Node>::rotate;

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

#endif  // AA_TREE_HPP