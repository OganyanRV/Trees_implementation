#pragma once
#include <algorithm>
#include <any>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../trees/abstract_tree.h"
#include "../trees/avl_tree.h"
#include "../trees/cartesian_tree.h"
#include "../trees/rb_tree.h"
#include "../trees/splay_tree.h"

class IFactory {
    virtual std::shared_ptr<std::any> Produce() = 0;
};

template <class T>
class AVLFactory : public IFactory {
public:
    virtual std::shared_ptr<std::any> Produce() {
        return std::make_shared<AVLTree<T>>();
    }
};

template <class T>
class CartesianFactory : public IFactory {
public:
    virtual std::shared_ptr<std::any> Produce() {
        return std::make_shared<CartesianTree<T>>();
    }
};

template <class T>
class RBFactory : public IFactory {
public:
    virtual std::shared_ptr<std::any> Produce() {
        return std::make_shared<RBTree<T>>();
    }
};

template <class T>
class SplayFactory : public IFactory {
public:
    virtual std::shared_ptr<std::any> Produce() {
        return std::make_shared<SplayTree<T>>();
    }
};