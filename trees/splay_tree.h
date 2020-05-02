#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>

template <class T>
class ITree;

template <class T>
class SplayTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    struct Node {
        Node() {
            left_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            right_ = nullptr;
            value_ = std::nullopt;
        }

        explicit Node(const T& value) : value_(value) {
            left_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            right_ = nullptr;
        }

        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            parent_ = other.parent_;
            right_ = other.right_;
        }

        std::shared_ptr<Node> left_;
        std::weak_ptr<Node> parent_;
        std::shared_ptr<Node> right_;
        std::optional<T> value_;
    };

    SplayTree() {
        begin_ = root_ = end_ = std::make_shared<Node>();
        size_ = 0;
    }

    template <class InitIterator>
    SplayTree(InitIterator begin, InitIterator end) : SplayTree() {
        for (InitIterator cur(begin); cur != end; ++cur) {
            Insert(*cur);
        }
    }

    SplayTree(std::initializer_list<T> list) : SplayTree() {
        for (const T& value : list) {
            Insert(value);
        }
    }

    SplayTree(const SplayTree& other) : SplayTree() {
        for (const T& value : other) {
            Insert(value);
        }
    }

    SplayTree(SplayTree&& other) noexcept : SplayTree() {
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }

    SplayTree(std::shared_ptr<ITree<T>> other)
        : SplayTree(*dynamic_cast<SplayTree<T>*>(other.get())) {
    }

    SplayTree& operator=(const SplayTree& other) {
        if (root_ == other.root_) {
            return *this;
        }
        root_ = std::make_shared<Node>();
        begin_ = root_;
        end_ = root_;
        size_ = 0;
        for (const T& value : other) {
            Insert(value);
        }
        return *this;
    }

    SplayTree& operator=(SplayTree&& other) noexcept {
        if (root_ == other.root_) {
            return *this;
        }
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
        return *this;
    }

    ~SplayTree() override {
        root_ = begin_ = end_ = nullptr;
        size_ = 0;
    }

    [[nodiscard]] size_t Size() const override {
        return size_;
    }

    [[nodiscard]] bool Empty() const override {
        return !size_;
    }

    std::shared_ptr<BaseImpl> Find(const T& value) const override {
        std::optional<T> val(value);
        return const_cast<SplayTree<T>*>(this)->FindImpl(root_, val);
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        std::optional<T> val(value);
        return const_cast<SplayTree<T>*>(this)->LowerBoundImpl(root_, val);
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertImpl(root_, new_node)) {
            ++size_;
        }
    }

    void Erase(const T& value) override {
        std::optional<T> val(value);
        if (EraseImpl(root_, value)) {
            --size_;
        }
    }

    void Clear() override {
        root_ = std::make_shared<Node>();
        begin_ = root_;
        end_ = root_;
        size_ = 0;
    }

private:
    std::shared_ptr<Node> begin_;
    std::shared_ptr<Node> end_;
    std::shared_ptr<Node> root_;
    size_t size_;

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class SplayTreeItImpl : public BaseImpl {
    private:
        std::shared_ptr<Node> it_;

    public:
        SplayTreeItImpl() = delete;

        explicit SplayTreeItImpl(std::shared_ptr<Node> ptr) : it_(ptr) {
        }

        SplayTreeItImpl(const SplayTreeItImpl& other) : it_(other.it_) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<SplayTreeItImpl>(*this);
        }

        void Increment() override {
            if (!it_->value_) {
                throw std::runtime_error("Index out of range while increasing");
            }
            if (it_->right_) {
                it_ = it_->right_;
                while (it_->left_) {
                    it_ = it_->left_;
                }
            } else {
                auto parent = it_->parent_.lock();
                while (parent && (parent->right_ == it_)) {
                    it_ = parent;
                    parent = it_->parent_.lock();
                }

                it_ = parent;
            }
        }

        void Decrement() override {
            if (it_->left_) {
                it_ = it_->left_;
                while (it_->right_) {
                    it_ = it_->right_;
                }
            } else {
                auto parent = it_->parent_.lock();
                while (parent && parent->left_ == it_) {
                    it_ = parent;
                    parent = it_->parent_.lock();
                }
                if (parent) {
                    it_ = parent;
                } else {
                    throw std::runtime_error("Index out of range while decreasing");
                }
            }
        }

        const T Dereferencing() const override {
            if (!it_->value_) {
                throw std::runtime_error("Index out of range on operator*");
            }
            return *(it_->value_);
        }

        const T* Arrow() const override {
            if (!it_->value_) {
                throw std::runtime_error("Index out of range on operator->");
            }
            return &(*it_->value_);
        }

        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::dynamic_pointer_cast<SplayTreeItImpl>(other);
            if (!casted) {
                return false;
            }
            return it_ == casted->it_;
        }
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<SplayTreeItImpl>(begin_);
    }

    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<SplayTreeItImpl>(end_);
    }

    std::shared_ptr<BaseImpl> FindImpl(std::shared_ptr<Node> from,
                                            const std::optional<T>& value) {
        auto cur_node = std::shared_ptr<Node>(from);
        while (true) {
            if (!from) {
                return End();
            }
            if (value < cur_node->value_) {
                if (cur_node->left_) {
                    cur_node = cur_node->left_;
                } else {
                    Splay(cur_node);
                    return End();
                }
            } else if (cur_node->value_ < value) {
                if (cur_node->right_) {
                    cur_node = cur_node->right_;
                } else {
                    Splay(cur_node);
                    return End();
                }
            } else {
                Splay(cur_node);
                return std::make_shared<SplayTreeItImpl>(root_);
            }
        }
    }

    std::shared_ptr<Node> Merge(std::shared_ptr<Node> left_subtree,
                                std::shared_ptr<Node> right_subtree) {
        if (!right_subtree) {
            return left_subtree;
        } else if (!left_subtree) {
            return right_subtree;
        } else if (left_subtree < right_subtree) {
            left_subtree->right_ = Merge(left_subtree->right_, right_subtree);
            if (left_subtree->right_) {
                left_subtree->right_->parent_ = left_subtree;
            }
            return left_subtree;
        } else {
            right_subtree->left_ = Merge(left_subtree, right_subtree->left_);
            if (right_subtree->left_) {
                right_subtree->left_->parent_ = right_subtree;
            }
            return right_subtree;
        }
    }

    bool EraseImpl(std::shared_ptr<Node>& from, const std::optional<T>& value) {
        auto cur_node = std::shared_ptr<Node>(from);
        while (true) {
            if (!from) {
                return false;
            }
            if (value < cur_node->value_) {
                if (cur_node->left_) {
                    cur_node = cur_node->left_;
                } else {
                   Splay(cur_node);
                   return false;
                }
            } else if (cur_node->value_ < value) {
                if (cur_node->right_) {
                    cur_node = cur_node->right_;
                } else {
                    Splay(cur_node);
                    return false;
                }
            } else {
                Splay(cur_node);
                root_ = Merge(root_->left_, root_->right_);
                UpdateBegin();
                return true;
            }
        }
    }

    bool InsertImpl(std::shared_ptr<Node> from, std::shared_ptr<Node> new_node) {
        if (!from) {
            root_ = new_node;
            return true;
        }
        bool CycleControl = true;
        auto tmp = std::shared_ptr<Node>(from);
        while (CycleControl) {
            if (new_node->value_ < tmp->value_) {
                if (tmp->left_) {
                    tmp = tmp->left_;
                } else {
                    new_node->parent_ = tmp;
                    tmp->left_ = new_node;
                    tmp = tmp->left_;
                    CycleControl = false;
                }
            } else if (tmp->value_ < new_node->value_) {
                if (tmp->right_) {
                    tmp = tmp->right_;
                } else {
                    new_node->parent_ = tmp;
                    tmp->right_ = new_node;
                    tmp = tmp->right_;
                    CycleControl = false;
                }
            } else {
                Splay(tmp);
                return false;
            }
        }
        Splay(tmp);
        return true;
    }

    std::shared_ptr<BaseImpl> LowerBoundImpl(std::shared_ptr<Node> from,
                                                  const std::optional<T>& value) {
        if (value < from->value_) {
            if (from->left_) {
                return LowerBoundImpl(from->left_, value);
            } else {
                return std::make_shared<SplayTreeItImpl>(from);
            }
        } else if (from->value_ < value) {
            if (from->right_) {
                return LowerBoundImpl(from->right_, value);
            } else {
                Splay(from);
                auto tmp = std::make_shared<SplayTreeItImpl>(from);
                tmp->Increment();
                return tmp;
            }
        } else {
            Splay(from);
            return std::make_shared<SplayTreeItImpl>(from);
        }
    }

    void UpdateBegin() {
        std::shared_ptr<Node> tmp(root_);
        while (tmp->left_) {
            tmp = tmp->left_;
        }
        begin_ = tmp;
    }

    void Splay(std::shared_ptr<Node> from) {
        while (true) {
            std::shared_ptr<Node> par = from->parent_.lock();
            if (!par) {
                break;
            }
            std::shared_ptr<Node> grandpar = par->parent_.lock();
            if (!grandpar)  // Zig
            {
                if (par->right_ == from) {
                    LeftRotate(par);
                } else {
                    RightRotate(par);
                }
                break;
            }
            if (grandpar->right_ == par) {
                if (par->right_ == from) {  // ZigZig
                    LeftRotate(grandpar);
                    LeftRotate(par);
                } else {  // ZigZag
                    RightRotate(par);
                    LeftRotate(grandpar);
                }
            } else {
                if (par->right_ == from) {  // ZigZag
                    LeftRotate(par);
                    RightRotate(grandpar);
                } else {  // ZigZig
                    RightRotate(grandpar);
                    RightRotate(par);
                }
            }
        }
        root_ = from;
        UpdateBegin();
    }

    void RightRotate(std::shared_ptr<Node>& from) {
        std::shared_ptr<Node> par = from->parent_.lock();
        std::shared_ptr<Node> left = from->left_;
        if (par) {
            if (par->right_ == from) {
                par->right_ = left;
            } else {
                par->left_ = left;
            }
        }
        if (left) {
            std::shared_ptr<Node> leftright = left->right_;
            if (leftright) {
                leftright->parent_ = from;
            }
            left->parent_ = par;
            left->right_ = from;
            from->left_ = leftright;
        }
        from->parent_ = left;
    }

    void LeftRotate(std::shared_ptr<Node>& from) {
        std::shared_ptr<Node> par = from->parent_.lock();
        std::shared_ptr<Node> right = from->right_;
        if (par) {
            if (par->left_ == from) {
                par->left_ = right;
            } else {
                par->right_ = right;
            }
        }
        if (right) {
            std::shared_ptr<Node> rightleft = right->left_;
            if (rightleft) {
                rightleft->parent_ = from;
            }
            right->parent_ = par;
            right->left_ = from;
            from->right_ = rightleft;
        }
        from->parent_ = right;
    }
};
