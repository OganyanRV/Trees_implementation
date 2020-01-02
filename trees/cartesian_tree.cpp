#include <initializer_list>
#include <exception>
#include <memory>
#include <random>

#include "abstract_tree.cpp"

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
            parent_ = nullptr;
        }
        explicit Node(bool end) {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = nullptr;
            if (end) {
                priority_ = 0;
            }
        }
        explicit Node(const T& value) : value_(value) {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = nullptr;
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
        std::shared_ptr<Node> parent_;
        T value_;
        uint32_t priority_;
    };

    CartesianTree() {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
    }

    template <class InitIterator>
    CartesianTree(InitIterator begin, InitIterator end) {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
        for (InitIterator cur(begin); cur != end; ++cur) {
            insert(*cur);
        }
    }
    CartesianTree(std::initializer_list<T> list) {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
        for (const T& value : list) {
            insert(value);
        }
    }

    CartesianTree(const CartesianTree& other) {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
        for (const T& value : other) {
            insert(value);
        }
    }
    CartesianTree(CartesianTree&& other) noexcept {
        begin_ = nullptr;
        last_ = nullptr;
        root_ = nullptr;
        size_ = 0;
        for (T&& value : other) {
            insert(std::move(value));
        }
    }
    CartesianTree& operator=(const CartesianTree& other) {
        if (root_ == other.root_) {
            return *this;
        }
        this->~CartesianTree();
        for (const T& value : other) {
            insert(value);
        }
        return *this;
    }
    CartesianTree& operator=(CartesianTree&& other) noexcept {
        if (root_ == other.root_) {
            return *this;
        }
        begin_ = other.begin_;
        last_ = other.last_;
        root_ = other.root_;
        size_ = other.size_;
        other.begin_ = nullptr;
        other.last_ = nullptr;
        other.root_ = nullptr;
        other.size_ = 0;
        return *this;
    }

    ~CartesianTree() override {
        begin_ = nullptr;
        last_ = nullptr;
        size_ = 0;
        FreeMemory(root_);
        root_ = nullptr;
    }

    [[nodiscard]] size_t size() const override {
        return size_;
    }
    [[nodiscard]] bool empty() const override {
        return !size_;
    }

    std::shared_ptr<BaseImpl> find_impl(const T& value) const override {
        return Find(root_, value);
    }
    std::shared_ptr<BaseImpl> lower_bound_impl(const T& value) const override {
        if (!root_) {
            return End();
        }
        if (value < root_->value_) {
            if (root_->left_) {
                return LowerBound(root_->left_, value);
            } else {
                return std::make_shared<CartesianTreeItImpl>(root_);
            }
        } else if (root_->value_ < value) {
            if (root_->right_) {
                return LowerBound(root_->right_, value);
            } else {
                return End();
            }
        } else {
            return std::make_shared<CartesianTreeItImpl>(root_);
        }
    }

    void insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (Insert(root_, new_node)) {
            ++size_;
        }
        RecalcBeginEnd();
    }
    void erase(const T& value) override {
        if (Erase(root_, value)) {
            --size_;
        }
        RecalcBeginEnd();
    }

    void clear() override {
        this->~CartesianTree<T>();
    }

private:
    std::shared_ptr<Node> begin_;
    std::shared_ptr<Node> last_;
    std::shared_ptr<Node> root_;
    size_t size_;

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class CartesianTreeItImpl : public BaseImpl {
    public:
        CartesianTreeItImpl() = delete;
        explicit CartesianTreeItImpl(std::shared_ptr<Node> pointer, bool is_ended = false) {
            it_ = pointer;
            is_ended_ = is_ended;
        }
        CartesianTreeItImpl(const CartesianTreeItImpl& other)
            : it_(other.it_), is_ended_(other.is_ended_) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<CartesianTreeItImpl>(*this);
        }
        void Increment() override {
            if (is_ended_) {
                throw std::runtime_error("Index out of range while increasing");
            }
            if (it_->right_) {
                it_ = it_->right_;
                while (it_->left_) {
                    it_ = it_->left_;
                }
            } else {
                while (it_->parent_ && it_->parent_->right_ == it_) {
                    it_ = it_->parent_;
                }
                if (it_->parent_) {
                    it_ = it_->parent_;
                } else {
                    while (it_->right_) {
                        it_ = it_->right_;
                    }
                    is_ended_ = true;
                }
            }
        }
        void Decrement() override {
            if (is_ended_) {
                is_ended_ = false;
            } else if (it_->left_) {
                it_ = it_->left_;
                while (it_->right_) {
                    it_ = it_->right_;
                }
            } else {
                while (it_->parent_ && it_->parent_->left_ == it_) {
                    it_ = it_->parent_;
                }
                if (it_->parent_) {
                    it_ = it_->parent_;
                } else {
                    throw std::runtime_error("Index out of range while decreasing");
                }
            }
        }
        T Dereferencing() const override {
            if (is_ended_) {
                throw std::runtime_error("Index out of range on operator*");
            }
            return it_->value_;
        }
        const T* Arrow() const override {
            if (is_ended_) {
                throw std::runtime_error("Index out of range on operator->");
            }
            return &it_->value_;
        }
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::static_pointer_cast<CartesianTreeItImpl>(other);
            if (!casted) {
                return false;
            }
            if (!it_ && !casted->it_) {
                return true;
            }
            if (is_ended_) {
                return casted->is_ended_;
            } else {
                if (casted->is_ended_) {
                    return false;
                } else {
                    return it_ == casted->it_;
                }
            }
        }

    private:
        std::shared_ptr<Node> it_;
        bool is_ended_;
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<CartesianTreeItImpl>(begin_);
    }
    std::shared_ptr<BaseImpl> End() const override {
        std::shared_ptr<Node> end_ = last_;
        return std::make_shared<CartesianTreeItImpl>(end_, true);
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

    static void Split(std::shared_ptr<Node> root, const T& value, std::shared_ptr<Node>& left_sub,
                      std::shared_ptr<Node>& right_sub) {
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
                    left_sub->parent_ = nullptr;
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
                    right_sub->parent_ = nullptr;
                }
            }
            left_sub = root;
        }
    }

    std::shared_ptr<BaseImpl> Find(std::shared_ptr<Node> from, const T& value) const {
        if (!from) {
            return End();
        }
        if (value < from->value_) {
            return Find(from->left_, value);
        } else if (from->value_ < value) {
            return Find(from->right_, value);
        } else {
            return std::make_shared<CartesianTreeItImpl>(from);
        }
    }
    static std::shared_ptr<BaseImpl> LowerBound(std::shared_ptr<Node> from, const T& value) {
        if (value < from->value_) {
            if (from->left_) {
                return LowerBound(from->left_, value);
            } else {
                return std::make_shared<CartesianTreeItImpl>(from);
            }
        } else if (from->value_ < value) {
            if (from->right_) {
                return LowerBound(from->right_, value);
            } else {
                auto impl = std::make_shared<CartesianTreeItImpl>(from);
                impl->Increment();
                return impl;
            }
        } else {
            return std::make_shared<CartesianTreeItImpl>(from);
        }
    }

    static bool Insert(std::shared_ptr<Node>& from, std::shared_ptr<Node> new_node) {
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
                result = Insert(from->left_, new_node);
                if (from->left_) {
                    from->left_->parent_ = from;
                }
            } else {
                result = Insert(from->right_, new_node);
                if (from->right_) {
                    from->right_->parent_ = from;
                }
            }
            return result;
        }
        return false;
    }
    static bool Erase(std::shared_ptr<Node>& from, const T& value) {
        if (!from) {
            return false;
        } else if (!(from->value_ < value) && !(value < from->value_)) {
            from = Merge(from->left_, from->right_);
            return true;
        } else {
            bool result;
            if (value < from->value_) {
                result = Erase(from->left_, value);
                if (from->left_) {
                    from->left_->parent_ = from;
                }
            } else {
                result = Erase(from->right_, value);
                if (from->right_) {
                    from->right_->parent_ = from;
                }
            }
            return result;
        }
    }

    void RecalcBeginEnd() {
        if (!root_) {
            begin_ = nullptr;
            last_ = nullptr;
            return;
        }
        std::shared_ptr<Node> cur_node = root_;
        while (cur_node->left_) {
            cur_node = cur_node->left_;
        }
        begin_ = cur_node;
        cur_node = root_;
        while (cur_node->right_) {
            cur_node = cur_node->right_;
        }
        last_ = cur_node;
    }

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
