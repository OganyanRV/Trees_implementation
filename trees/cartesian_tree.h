#pragma once
#include <initializer_list>
#include <exception>
#include <memory>
#include <random>
#include <optional>

template <class T>
class ITree;

template <class T>
bool operator<(const std::optional<T>& lhs, const std::optional<T>& rhs) {
    return (lhs && (!rhs || *lhs < *rhs));
}

template <class T>
class CartesianTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

    class Random {
    public:
        static uint32_t Next() {
            static Random rand = Random();
            return rand.dist_(rand.gen_);
        }

    private:
        Random() {
            std::random_device device;
            gen_ = std::mt19937(device());
            dist_ =
                std::uniform_int_distribution<uint32_t>(1, std::numeric_limits<uint32_t>::max());
        }

        std::mt19937 gen_;
        std::uniform_int_distribution<uint32_t> dist_;
    };

public:
    struct Node {
        Node() {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            priority_ = Random::Next();
            value_ = std::nullopt;
        }
        explicit Node(const T& value) : value_(value) {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            priority_ = Random::Next();
        }
        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            right_ = other.right_;
            parent_ = other.parent_;
            priority_ = other.priority_;
        }

        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
        uint32_t priority_;
        std::optional<T> value_;
    };

    CartesianTree() {
        root_ = std::make_shared<Node>();
        begin_ = root_;
        end_ = root_;
        size_ = 0;
    }

    template <class InitIterator>
    CartesianTree(InitIterator begin, InitIterator end) : CartesianTree() {
        for (InitIterator cur(begin); cur != end; ++cur) {
            Insert(*cur);
        }
    }
    CartesianTree(std::initializer_list<T> list) : CartesianTree() {
        for (const T& value : list) {
            Insert(value);
        }
    }

    CartesianTree(const CartesianTree& other) : CartesianTree() {
        for (const T& value : other) {
            Insert(value);
        }
    }
    CartesianTree(CartesianTree&& other) noexcept : CartesianTree() {
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }
    CartesianTree(std::shared_ptr<ITree<T>> other)
        : CartesianTree(*dynamic_cast<CartesianTree<T>*>(other.get())) {
    }
    CartesianTree& operator=(const CartesianTree& other) {
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
    CartesianTree& operator=(CartesianTree&& other) noexcept {
        if (root_ == other.root_) {
            return *this;
        }
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
        return *this;
    }

    ~CartesianTree() override {
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
        if (val < root_->value_) {
            if (root_->left_) {
                return LowerBoundRec(root_->left_, val);
            } else {
                return std::make_shared<CartesianTreeItImpl>(root_);
            }
        } else if (root_->value_ < val) {
            if (root_->right_) {
                return LowerBoundRec(root_->right_, val);
            } else {
                return End();
            }
        } else {
            return std::make_shared<CartesianTreeItImpl>(root_);
        }
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertRec(root_, new_node)) {
            ++size_;
        }
        RecalcBeginEnd();
    }
    void Erase(const T& value) override {
        std::optional<T> val(value);
        if (EraseRec(root_, val)) {
            --size_;
        }
        RecalcBeginEnd();
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
    size_t size_{};

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class CartesianTreeItImpl : public BaseImpl {
    public:
        CartesianTreeItImpl() = delete;
        explicit CartesianTreeItImpl(std::shared_ptr<Node> pointer) : it_(pointer) {
        }
        CartesianTreeItImpl(const CartesianTreeItImpl& other) : it_(other.it_) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<CartesianTreeItImpl>(*this);
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
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::static_pointer_cast<CartesianTreeItImpl>(other);
            if (!casted) {
                return false;
            }
            return it_ == casted->it_;
        }

    private:
        std::shared_ptr<Node> it_;
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<CartesianTreeItImpl>(begin_);
    }
    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<CartesianTreeItImpl>(end_);
    }

    /* ---------------------------------------------------
     * ----------------PRIVATE FUNCTIONS------------------
     * ---------------------------------------------------
     */

    static std::shared_ptr<Node> Merge(std::shared_ptr<Node> lhs, std::shared_ptr<Node> rhs) {
        if (!lhs) {
            return rhs;
        }
        if (!rhs) {
            return lhs;
        }
        if (lhs->priority_ < rhs->priority_) {
            lhs->right_ = Merge(lhs->right_, rhs);
            if (lhs->right_) {
                lhs->right_->parent_ = lhs;
            }
            return lhs;
        } else {
            rhs->left_ = Merge(lhs, rhs->left_);
            if (rhs->left_) {
                rhs->left_->parent_ = rhs;
            }
            return rhs;
        }
    }

    static void Split(std::shared_ptr<Node> root, const std::optional<T>& value,
                      std::shared_ptr<Node>& left_sub, std::shared_ptr<Node>& right_sub) {
        std::shared_ptr<Node> new_subtree = nullptr;
        if (value < root->value_) {
            if (!root->left_) {
                left_sub = nullptr;
            } else {
                Split(root->left_, value, left_sub, new_subtree);
                root->left_ = new_subtree;
                if (new_subtree) {
                    new_subtree->parent_ = root;
                }
                if (left_sub) {
                    left_sub->parent_ = std::weak_ptr<Node>();
                }
            }
            right_sub = root;
        } else {
            if (!root->right_) {
                right_sub = nullptr;
            } else {
                Split(root->right_, value, new_subtree, right_sub);
                root->right_ = new_subtree;
                if (new_subtree) {
                    new_subtree->parent_ = root;
                }
                if (right_sub) {
                    right_sub->parent_ = std::weak_ptr<Node>();
                }
            }
            left_sub = root;
        }
    }

    std::shared_ptr<BaseImpl> FindRec(std::shared_ptr<Node> from,
                                      const std::optional<T>& value) const {
        if (!from) {
            return End();
        }
        if (value < from->value_) {
            return FindRec(from->left_, value);
        } else if (from->value_ < value) {
            return FindRec(from->right_, value);
        } else {
            return std::make_shared<CartesianTreeItImpl>(from);
        }
    }
    static std::shared_ptr<BaseImpl> LowerBoundRec(std::shared_ptr<Node> from,
                                                   const std::optional<T>& value) {
        if (value < from->value_) {
            if (from->left_) {
                return LowerBoundRec(from->left_, value);
            } else {
                return std::make_shared<CartesianTreeItImpl>(from);
            }
        } else if (from->value_ < value) {
            if (from->right_) {
                return LowerBoundRec(from->right_, value);
            } else {
                auto impl = std::make_shared<CartesianTreeItImpl>(from);
                impl->Increment();
                return impl;
            }
        } else {
            return std::make_shared<CartesianTreeItImpl>(from);
        }
    }

    static bool InsertRec(std::shared_ptr<Node>& from, std::shared_ptr<Node> new_node) {
        if (!from) {
            from = new_node;
            return true;
        } else if (from->priority_ >= new_node->priority_) {
            std::shared_ptr<Node> left_sub = nullptr, right_sub = nullptr;
            Split(from, new_node->value_, left_sub, right_sub);
            if (left_sub) {
                std::shared_ptr<Node> max_v = left_sub;
                while (max_v->right_) {
                    max_v = max_v->right_;
                }
                if (max_v->value_ < new_node->value_) {
                    from = Merge(Merge(left_sub, new_node), right_sub);
                    return true;
                } else if (new_node->value_ < max_v->value_) {
                    throw std::runtime_error("Error in function Split()");
                } else {
                    from = Merge(left_sub, right_sub);
                    return false;
                }
            } else {
                new_node->right_ = right_sub;
                if (right_sub) {
                    right_sub->parent_ = new_node;
                }
                from = new_node;
                return true;
            }
        } else if ((from->value_ < new_node->value_) || (new_node->value_ < from->value_)) {
            bool result;
            if (new_node->value_ < from->value_) {
                result = InsertRec(from->left_, new_node);
                if (from->left_) {
                    from->left_->parent_ = from;
                }
            } else {
                result = InsertRec(from->right_, new_node);
                if (from->right_) {
                    from->right_->parent_ = from;
                }
            }
            return result;
        }
        return false;
    }
    static bool EraseRec(std::shared_ptr<Node>& from, const std::optional<T>& value) {
        bool result;
        if (!from) {
            return false;
        } else if (value < from->value_) {
            result = EraseRec(from->left_, value);
            if (from->left_) {
                from->left_->parent_ = from;
            }
        } else if (from->value_ < value) {
            result = EraseRec(from->right_, value);
            if (from->right_) {
                from->right_->parent_ = from;
            }
        } else {
            from = Merge(from->left_, from->right_);
            return true;
        }
        return result;
    }

    void RecalcBeginEnd() {
        std::shared_ptr<Node> cur_node = root_;
        while (cur_node->left_) {
            cur_node = cur_node->left_;
        }
        begin_ = cur_node;
        cur_node = root_;
        while (cur_node->right_) {
            cur_node = cur_node->right_;
        }
        end_ = cur_node;
    }

    // No longer need
    static void FreeMemory(std::shared_ptr<Node> from) {
        if (!from) {
            return;
        }
        FreeMemory(from->left_);
        FreeMemory(from->right_);
        from->left_ = nullptr;
        from->right_ = nullptr;
        from->parent_ = nullptr;
    }
};
