#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>

template <class T>
class ITree;

template <class T>
class SplayTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    struct Node {
        Node() {
            parent_ = left_ = right_ = nullptr;
            value_ = std::nullopt;
        }

        explicit Node(const T& value) : value_(value) {
            parent_ = left_ = right_ = nullptr;
        }

        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            parent_ = other.parent_;
            right_ = other.right_;
        }

        ~Node() {
            left_ = nullptr;
            right_ = nullptr;
        }

        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
        std::optional<T> value_;
    };

    SplayTree() {
        begin_ = end_ = root_ = std::make_shared<Node>();
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
        begin_ = end_ = root_ = std::make_shared<Node>();
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
        size_ = 0;
        std::queue<std::shared_ptr<Node>> nodes;
        nodes.emplace(root_);
        begin_ = end_ = root_ = nullptr;
        while (!nodes.empty()) {
            std::shared_ptr<Node> cur = nodes.front();
            nodes.pop();
            if (cur->left_) {
                nodes.emplace(cur->left_);
            }
            if (cur->right_) {
                nodes.emplace(cur->right_);
            }
            cur->left_ = nullptr;
            cur->right_ = nullptr;
        }
    }

    [[nodiscard]] size_t Size() const override {
        return size_;
    }

    [[nodiscard]] bool Empty() const override {
        return !size_;
    }

    std::shared_ptr<BaseImpl> Find(const T& value) const override {
        std::optional<T> val(value);
        std::shared_ptr<Node> cur_node = root_;
        while (cur_node) {
            if (val < cur_node->value_) {
                if (cur_node->left_) {
                    cur_node = cur_node->left_;
                } else {
                    const_cast<SplayTree<T>*>(this)->Splay(cur_node);
                    return End();
                }
            } else if (cur_node->value_ < val) {
                if (cur_node->right_) {
                    cur_node = cur_node->right_;
                } else {
                    const_cast<SplayTree<T>*>(this)->Splay(cur_node);
                    return End();
                }
            } else {
                const_cast<SplayTree<T>*>(this)->Splay(cur_node);
                return std::make_shared<SplayTreeItImpl>(root_);
            }
        }
        return End();
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        std::optional<T> val(value);
        std::shared_ptr<Node> cur = root_;
        while (true) {
            if (val < cur->value_) {
                if (cur->left_) {
                    cur = cur->left_;
                } else {
                    const_cast<SplayTree<T>*>(this)->Splay(cur);
                    return std::make_shared<SplayTreeItImpl>(root_);
                }
            } else if (cur->value_ < val) {
                if (cur->right_) {
                    cur = cur->right_;
                } else {
                    auto it = std::make_shared<SplayTreeItImpl>(cur);
                    it->Increment();
                    const_cast<SplayTree<T>*>(this)->Splay(it->GetPointer());
                    return it;
                }
            } else {
                const_cast<SplayTree<T>*>(this)->Splay(cur);
                return std::make_shared<SplayTreeItImpl>(root_);
            }
        }
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        std::shared_ptr<Node> cur = root_;
        while (true) {
            if (new_node->value_ < cur->value_) {
                if (cur->left_) {
                    cur = cur->left_;
                } else {
                    new_node->parent_ = cur;
                    cur->left_ = new_node;
                    if (begin_ == cur) {
                        begin_ = new_node;
                    }
                    cur = cur->left_;
                    break;
                }
            } else if (cur->value_ < new_node->value_) {
                if (cur->right_) {
                    cur = cur->right_;
                } else {
                    new_node->parent_ = cur;
                    cur->right_ = new_node;
                    cur = cur->right_;
                    break;
                }
            } else {
                Splay(cur);
                return;
            }
        }
        Splay(cur);
        ++size_;
    }

    void Erase(const T& value) override {
        std::optional<T> val(value);
        auto cur_node = root_;
        while (true) {
            if (val < cur_node->value_) {
                if (cur_node->left_) {
                    cur_node = cur_node->left_;
                } else {
                    Splay(cur_node);
                    return;
                }
            } else if (cur_node->value_ < val) {
                if (cur_node->right_) {
                    cur_node = cur_node->right_;
                } else {
                    Splay(cur_node);
                    return;
                }
            } else {
                break;
            }
        }
        Splay(cur_node);
        if (root_ == begin_) {
            auto it = std::dynamic_pointer_cast<SplayTreeItImpl>(Begin());
            it->Increment();
            begin_ = it->GetPointer();
        }
        --size_;
        std::shared_ptr<Node> left_sub = root_->left_, right_sub = root_->right_;
        root_ = right_sub;
        cur_node = root_;
        while (left_sub) {
            right_sub = right_sub->left_;
            cur_node->left_ = left_sub;
            left_sub->parent_ = cur_node;
            cur_node = cur_node->left_;
            if (right_sub) {
                left_sub = left_sub->right_;
                cur_node->right_ = right_sub;
                right_sub->parent_ = cur_node;
                cur_node = cur_node->right_;
            } else {
                break;
            }
        }
    }

    void Clear() override {
        size_ = 0;
        std::queue<std::shared_ptr<Node>> nodes;
        nodes.emplace(root_);
        begin_ = end_ = root_ = nullptr;
        while (!nodes.empty()) {
            std::shared_ptr<Node> cur = nodes.front();
            nodes.pop();
            if (cur->left_) {
                nodes.emplace(cur->left_);
            }
            if (cur->right_) {
                nodes.emplace(cur->right_);
            }
            cur->left_ = nullptr;
            cur->right_ = nullptr;
        }
        begin_ = end_ = root_ = std::make_shared<Node>();
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

        explicit SplayTreeItImpl(std::shared_ptr<Node> other) : it_(other) {
        }

        SplayTreeItImpl(const SplayTreeItImpl& other) : it_(other.it_) {
        }

        std::shared_ptr<Node> GetPointer() {
            return it_;
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
                while (it_->parent_.lock()->right_ == it_) {
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

    void Splay(std::shared_ptr<Node> from) {
        std::shared_ptr<Node> parent = from->parent_.lock();
        while (parent) {
            std::shared_ptr<Node> grandparent = parent->parent_.lock();
            if (!grandparent) {
                if (parent->right_ == from) {
                    Zag(from, parent);
                } else {
                    Zig(from, parent);
                }
                break;
            }
            if (grandparent->right_ == parent) {
                if (parent->right_ == from) {
                    ZagZag(from, parent, grandparent);
                } else {
                    ZagZig(from, parent, grandparent);
                }
            } else {
                if (parent->right_ == from) {
                    ZigZag(from, parent, grandparent);
                } else {
                    ZigZig(from, parent, grandparent);
                }
            }
            parent = from->parent_.lock();
        }
        root_ = from;
    }

    static void Zig(std::shared_ptr<Node> x, std::shared_ptr<Node> y) {
        std::shared_ptr<Node> hanger = y->parent_.lock();
        bool left_child = hanger && hanger->left_ == y;
        y->left_ = x->right_;
        if (y->left_) {
            y->left_->parent_ = y;
        }
        x->right_ = y;
        y->parent_ = x;
        SetHanger(x, hanger, left_child);
    }

    static void Zag(std::shared_ptr<Node> x, std::shared_ptr<Node> y) {
        std::shared_ptr<Node> hanger = y->parent_.lock();
        bool left_child = hanger && hanger->left_ == y;
        y->right_ = x->left_;
        if (y->right_) {
            y->right_->parent_ = y;
        }
        x->left_ = y;
        y->parent_ = x;
        SetHanger(x, hanger, left_child);
    }

    static void ZigZig(std::shared_ptr<Node> x, std::shared_ptr<Node> y, std::shared_ptr<Node> z) {
        std::shared_ptr<Node> hanger = z->parent_.lock();
        bool left_child = hanger && hanger->left_ == z;
        z->left_ = y->right_;
        if (z->left_) {
            z->left_->parent_ = z;
        }
        z->parent_ = y;
        y->right_ = z;
        y->left_ = x->right_;
        if (y->left_) {
            y->left_->parent_ = y;
        }
        y->parent_ = x;
        x->right_ = y;
        SetHanger(x, hanger, left_child);
    }

    static void ZagZag(std::shared_ptr<Node> x, std::shared_ptr<Node> y, std::shared_ptr<Node> z) {
        std::shared_ptr<Node> hanger = z->parent_.lock();
        bool left_child = hanger && hanger->left_ == z;
        z->right_ = y->left_;
        if (z->right_) {
            z->right_->parent_ = z;
        }
        z->parent_ = y;
        y->left_ = z;
        y->right_ = x->left_;
        if (y->right_) {
            y->right_->parent_ = y;
        }
        y->parent_ = x;
        x->left_ = y;
        SetHanger(x, hanger, left_child);
    }

    static void ZigZag(std::shared_ptr<Node> x, std::shared_ptr<Node> y, std::shared_ptr<Node> z) {
        std::shared_ptr<Node> hanger = z->parent_.lock();
        bool left_child = hanger && hanger->left_ == z;
        z->left_ = x->right_;
        if (z->left_) {
            z->left_->parent_ = z;
        }
        z->parent_ = x;
        x->right_ = z;
        y->right_ = x->left_;
        if (y->right_) {
            y->right_->parent_ = y;
        }
        y->parent_ = x;
        x->left_ = y;
        SetHanger(x, hanger, left_child);
    }

    static void ZagZig(std::shared_ptr<Node> x, std::shared_ptr<Node> y, std::shared_ptr<Node> z) {
        std::shared_ptr<Node> hanger = z->parent_.lock();
        bool left_child = hanger && hanger->left_ == z;
        z->right_ = x->left_;
        if (z->right_) {
            z->right_->parent_ = z;
        }
        z->parent_ = x;
        x->left_ = z;
        y->left_ = x->right_;
        if (y->left_) {
            y->left_->parent_ = y;
        }
        y->parent_ = x;
        x->right_ = y;
        SetHanger(x, hanger, left_child);
    }

    static void SetHanger(std::shared_ptr<Node> x, std::shared_ptr<Node> hanger, bool left_child) {
        x->parent_ = hanger;
        if (hanger) {
            if (left_child) {
                hanger->left_ = x;
            } else {
                hanger->right_ = x;
            }
        }
    }
};