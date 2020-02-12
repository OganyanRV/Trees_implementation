#pragma once
#include <initializer_list>
#include <memory>

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

        explicit Node(const T& value) {
            left_ = nullptr;
            parent_ = nullptr;
            right_ = nullptr;
            value_ = value;
        }

        Node(const Node& other) {
            left_ = other.left_;
            parent_ = other.parent_;
            right_ = other.right_;
            value_ = other.value_;
        }

        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> parent_;
        std::shared_ptr<Node> right_;
        std::optional<T> value_;
    };

    SplayTree() {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
    }

    template <class InitIterator>
    SplayTree(InitIterator begin, InitIterator end) {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
        for (InitIterator cur(begin); cur != end; ++cur) {
            insert(*cur);
        }
    }

    SplayTree(std::initializer_list<T> list) {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
        for (const T& value : list) {
            insert(value);
        }
    }

    SplayTree(const SplayTree& other) : SplayTree() {
        for (const T& value : other) {
            Insert(value);
        }
    }

    void SwapTree(SplayTree&& other) noexcept {
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }

    SplayTree(SplayTree&& other) noexcept : SplayTree() {
        SwapTree(other);
    }

    SplayTree(std::shared_ptr<ITree<T>> other)
        : SplayTree(*dynamic_cast<CartesianTree<T>*>(other.get())) {
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
        SwapTree(other);
        return *this;
    }

    ~SplayTree() override {
        root_ = std::make_shared<Node>();
        begin_ = root_;
        end_ = root_;
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
        return FindRec(root_, val);
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        std::optional<T> val(value);
        return LowerBoundRec(root_, value);
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertImpl(root_, new_node)) {
            ++size_;
        }
        RecalcBeginEnd();
    }

    void Erase(const T& value) override {
        std::optional<T> val(value);
        if (EraseImpl(root_, value)) {
            --size_;
        }
        RecalcBeginEnd();
    }

    void Clear() override {  // где он нужен?
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

        SplayTreeItImpl(std::shared_ptr<Node> ptr) {
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
                while (it_->parent_.lock()->left_ == it_ && it_->parent_.lock()) {
                    it_ = it_->parent_.lock();
                }
                if (it_->parent_.lock()) {
                    it_ = it_->parent_.lock();
                } else {
                    throw std::runtime_error("Index out of range while decreasing");
                }
            }
        }

        T Dereferencing() const override {
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

        bool IsEqual(std::shared_ptr<BaseImpl> other) const override;  // delat`?

        std::shared_ptr<BaseImpl> Begin() const override {
            return std::make_shared<SplayTreeItImpl>(begin_);
        }

        std::shared_ptr<BaseImpl> End() const override {
            return std::make_shared<SplayTreeItImpl>(end_);
        }

        std::shared_ptr<BaseImpl> Root() const override {
            return std::make_shared<SplayTreeItImpl>(root_);
        }

        std::shared_ptr<BaseImpl> FindRec(std::shared_ptr<Node> from,
                                          const std::optional<T>& value) const {
            if (!from) {
                return End();
            }
            if (value < from->value_) {
                return FindRec(from->left_, value);
            } else if (value > from->value_) {
                return FindRec(from->right_, value);
            } else {
                Splay(from);
                return Root();
            }
        }

        static std::shared_ptr<Node> Merge(std::shared_ptr<Node> l, std::shared_ptr<Node> r) {
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
                    r->left_->parent_ = к;
                }
                return r;
            }
        }

        static bool EraseImpl(
            std::shared_ptr<Node>& from,
            const std::optional<T>& value) {  // возможно внутри ирэйс написать файнд вручную
            if (find(from, value) == End()) {
                return false;
            }
            from = Merge(from->left_, from->right_);
            return true;
        }

        static bool InsertImpl(std::shared_ptr<Node> from,
                               const std::optional<T>& value) {  // сделать инсерт рекурсией
            if (!from) {
                from->left_ = nullptr;
                from->right_ = nullptr;
                from->parent_ = nullptr;
                from->value_ = value;
                return true;
            }
            bool f = true;
            auto tmp = std::shared_ptr<Node> from;
            while (f) {
                if (value == tmp->value) {
                    return false;
                }
                if (value < tmp->value) {
                    if (tmp->left_) {
                        tmp = tmp->left_;
                    } else {
                        tmp->left_->left_ = nullptr;
                        tmp->left_->right_ = nullptr;
                        tmp->left_->parent_ = tmp;
                        tmp->left_->value_ = value;
                        tmp = tmp->left_;
                        f = false;
                    }
                } else if (value > tmp->value) {
                    if (tmp->right_) {
                        tmp = tmp->right_;
                    } else {
                        tmp->right_->left_ = nullptr;
                        tmp->right_->right_ = nullptr;
                        tmp->right_->parent_ = tmp;
                        tmp->right_->value_ = value;
                        tmp = tmp->right_;
                        f = false;
                    }
                }
            }
            Splay(tmp);
            return true;
        }

        static std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from,
                                                             const std::optional<T>& value) {
            if (value < from->value_) {
                if (from->left_) {
                    return LowerBoundRecursive(from->left_, value);
                } else {
                    return std::make_shared<SplayTreeItImpl>(from);
                }
            } else if (from->value_ < value) {
                if (from->right_) {
                    return LowerBoundRecursive(from->right_, value);
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

        void UpdateBegEnd() {
            std::shared_ptr<Node> tmp(root_);
            while (tmp->left_) {
                tmp = tmp->left_;
            }
            begin_ = tmp;
            tmp = root_;
            while (tmp->right_) {
                tmp = tmp->right_;
            }
            tmp->right_ = end_;
            end_->parent_ = tmp;
        }

        void RemoveEnd() {
            std::shared_ptr<Node> tmp(end_->parent_.lock());
            if (tmp) {
                tmp->right_ = nullptr;
                end_->parent = nullptr;
            }
            if (root_ == end_) {
                root_ = nullptr;
            }
        }

        void Splay(std::shared_ptr<Node> from) {  // parpar = grandpa
            RemoveEnd();
            std::shared_ptr<Node> par = from->parent_.lock();
            bool f = true;
            while (f) {
                if (!par) {
                    f = false;
                }
                std::shared_ptr<Node> parpar = par->parent_.lock();
                if (!parpar)  // It is a Zig`s case
                {
                    if (par->right_ == from) {
                        LeftRotate(par);
                    } else {
                        RightRotate(par);
                    }
                    f = false;
                }
                if (parpar->right_ == par) {
                    if (par->right_ == from) {  // ZigZag
                        LeftRotate(parpar);
                        LeftRotate(par);
                    } else {  // ZigZig
                        RightRotate(parpar);
                        LeftRotate(par);
                    }
                } else {
                    if (parpar->right_ == from) {  // ZigZig
                        LeftRotate(parpar);
                        RightRotate(par);
                    } else {  // ZigZag
                        RightRotate(parpar);
                        RightRotate(parpar);
                    }
                }
            }
            root_ = from;
            UpdateBegEnd();
        }

        void RightRotate(std::shared_ptr<Node>& from) {
            std::shared_ptr<Node> par = from->parent_.lock();
            std::shared_ptr<Node> left = from->left_.lock();
            if (par) {
                if (par->right_ == from) {
                    par->right_ = left;
                } else {
                    par->left_ = left;
                }
            }
            if (left) {
                std::shared_ptr<Node> leftright = left->right_.lock();
                if (leftright) {
                    leftright->parent_ = from;
                }
                left->parent_ = par;
                left->right_ = from;
            }
            from->parent = left;
            from->left_ = leftright;
        }

        void LeftRotate(std::shared_ptr<Node>& from) {
            std::shared_ptr<Node> par = from->parent_.lock();
            std::shared_ptr<Node> right = from->right_.lock();
            if (par) {
                if (par->left_ == from) {
                    par->left_ = right;
                } else {
                    par->right_ = right;
                }
            }
            if (right) {
                std::shared_ptr<Node> rightleft = right->left_.lock();
                if (rightleft) {
                    rightleft->parent_ = from;
                }
                right->parent_ = par;
                right->left_ = from;
            }
            from->parent = right;
            from->right = rightleft;
        }
    };
};
