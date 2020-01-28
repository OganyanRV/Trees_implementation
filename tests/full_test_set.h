#pragma once
#include <iostream>

#include "../trees/abstract_tree.h"

using std::cout;

enum class ImplType { kAVL, kCartesian, kRB, kSplay };

template <class T>
std::shared_ptr<ITree<T>> MakeTree(ImplType type = ImplType::kRB) {
    if (type == ImplType::kAVL) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<AVLTree<T>>();
    } else if (type == ImplType::kCartesian) {
        return std::make_shared<CartesianTree<T>>();
    } else if (type == ImplType::kRB) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<RBTree<T>>();
    } else if (type == ImplType::kSplay) {
        throw std::runtime_error("Tree is not implemented yet");
        // return std::make_shared<SplayTree<T>>();
    } else {
        throw std::runtime_error("Impossible behaviour");
    }
}

void SomeTest(const std::string& name, ImplType type) {
    auto tree = MakeTree<int>(ImplType::kCartesian);
    tree->insert(1);
    cout << "testing " << name << ". Result: " << *tree->begin() << '\n';
}