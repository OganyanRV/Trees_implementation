#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>

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

    SplayTree(SplayTree&& other) noexcept : SplayTree() {  // Maybe should make Swapfun
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
        root_ = nullptr;
        begin_ = nullptr;
        end_ = nullptr;
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
        return const_cast<SplayTree<T>*>(this)->FindRec(root_, val);
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        std::optional<T> val(value);
        return const_cast<SplayTree<T>*>(this)->LowerBoundRec(root_, val);
    }

    void Insert(const T& value) override {
        std::optional<T> val(value);
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

        explicit SplayTreeItImpl(std::shared_ptr<Node> ptr) {
            it_ = ptr;
        }

        SplayTreeItImpl(const SplayTreeItImpl& other) {
            it_ = other.it_;
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
                while (it_->parent_.lock() && it_->parent_.lock()->right_ == it_) {
                    it_ = it_->parent_.lock();
                }
                it_ = it_->parent_.lock();
            }
        }

        void Decrement() override {
            if (it_->left_) {
                it_ = it_->left_;
                while (it_->right_) {
                    it_ = it_->right_;
                }
            } else {
                while (it_->parent_.lock() && it_->parent_.lock()->left_ == it_) {
                    it_ = it_->parent_.lock();
                }
                if (it_->parent_.lock()) {
                    it_ = it_->parent_.lock();
                } else {
                    throw std::runtime_error("Index out of range while decreasing");
                }
            }
        }

        const T Dereferencing() const override {
            if (it_ && !it_->value_) {
                throw std::runtime_error("Index out of range on operator*");
            }
            return *(it_->value_);
        }

        const T* Arrow() const override {
            if (it_ && !it_->value_) {
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
        std::shared_ptr<Node> GetPointer() {
            return it_;
        }
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<SplayTreeItImpl>(begin_);
    }

    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<SplayTreeItImpl>(end_);
    }

    std::shared_ptr<BaseImpl> Root() const {
        return std::make_shared<SplayTreeItImpl>(root_);
    }

    std::shared_ptr<BaseImpl> FindRec(std::shared_ptr<Node> from, const std::optional<T>& value) {
        if (!from) {
            return End();
        }
        if (value < from->value_) {
            return FindRec(from->left_, value);
        } else if (from->value_ < value) {
            return FindRec(from->right_, value);
        } else {
            Splay(from);
            return Root();
        }
    }

    std::shared_ptr<Node> Merge(std::shared_ptr<Node> l, std::shared_ptr<Node> r) {
        if (!r) {
            return l;
        } else if (!l) {
            return r;
        } else if (l < r) {
            l->right_ = Merge(l->right_, r);
            if (l->right_) {
                l->right_->parent_ = l;
            }
            return l;
        } else {
            r->left_ = Merge(l, r->left_);
            if (r->left_) {
                r->left_->parent_ = r;
            }
            return r;
        }
    }

    bool EraseImpl(std::shared_ptr<Node>& from, const std::optional<T>& value) {
        bool result;
        if (!from) {
            return false;
        } else if (value < from->value_) {
            result = EraseImpl(from->left_, value);
        } else if (from->value_ < value) {
            result = EraseImpl(from->right_, value);
        } else {
            Splay(from);
            root_ = Merge(root_->left_, root_->right_);
            UpdateBeg();
            return true;
        }
        return result;
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
                return false;
            }
        }
        Splay(tmp);
        return true;
    }

    std::shared_ptr<BaseImpl> LowerBoundRec(std::shared_ptr<Node> from,
                                            const std::optional<T>& value) {
        if (value < from->value_) {
            if (from->left_) {
                return LowerBoundRec(from->left_, value);
            } else {
                return std::make_shared<SplayTreeItImpl>(from);
            }
        } else if (from->value_ < value) {
            if (from->right_) {
                return LowerBoundRec(from->right_, value);
            } else {
                Splay(from);
                auto tmp = std::make_shared<SplayTreeItImpl>(from);
                tmp->Increment();
                return tmp;
            }
        } else {
            return std::make_shared<SplayTreeItImpl>(from);
        }
    }

    void UpdateBeg() {
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
            if (!grandpar)  // It is a Zig`s case
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
        UpdateBeg();
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
