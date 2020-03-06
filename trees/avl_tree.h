#pragma once
#include <initializer_list>
#include <memory>

template <class T>
class ITree;

template <class T>
class AVLTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    struct Node {
        Node() {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            value_ = std::nullopt;
            height_ = 1;
        }

        explicit Node(const T& value) : value_(value) {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            height_ = 1;
        }

        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            right_ = other.right_;
            parent_ = other.parent_;
            height_ = other.height_;
        }

        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
        std::optional<T> value_;
        uint8_t height_;
    };

    AVLTree() {
        end_ = std::make_shared<Node>();
        begin_ = end_;
        root_ = end_;
        size_ = 0;
    }

    template <class InitIterator>
    AVLTree(InitIterator begin, InitIterator end) : AVLTree() {
        for (InitIterator cur(begin); cur != end; ++cur) {
            Insert(*cur);
        }
    }
    AVLTree(std::initializer_list<T> list)  : AVLTree() {
        for (const T& value : list) {
            Insert(value);
        }
    }

    AVLTree(const AVLTree &other) : AVLTree() {
        for (const T& value : other) {
            Insert(value);
        }
    }
    AVLTree(AVLTree &&other) noexcept : AVLTree() {
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }
    AVLTree(std::shared_ptr<ITree<T>> other) : AVLTree(*dynamic_cast<AVLTree<T> *>(other.get())) {
    }
    AVLTree &operator=(const AVLTree &other) {
        if (root_ == other.root_) {
            return *this;
        }
        end_ = std::make_shared<Node>();
        root_ = end_;
        begin_ = end_;
        size_ = 0;
        for (const T& value : other) {
            Insert(value);
        }
        return *this;
    }
    AVLTree &operator=(AVLTree &&other) noexcept {
        if (root_ == other.root_) {
            return *this;
        }
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
        return *this;
    }

    ~AVLTree() override {
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

    std::shared_ptr<BaseImpl> Find(const T &value) const override {
        return FindRecursive(root_, value);
    }
    std::shared_ptr<BaseImpl> LowerBound(const T &value) const override {
        std::optional val(value);
        return LowerBoundRecursive(root_, val);
    }

    void Insert(const T &value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertImplementation(new_node)) {
            ++size_;
        }
    }
    void Erase(const T &value) override {
        auto nodeInTree = std::static_pointer_cast<AVLTreeItImpl>(Find(value));
        if (nodeInTree->IsEqual(End())) {
            return;
        }

        EraseImplementation(nodeInTree->GetPointer());
        --size_;
    }

    void Clear() override {
        root_ = std::shared_ptr<Node>();
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

    class AVLTreeItImpl : public BaseImpl {
    public:
        AVLTreeItImpl() = delete;
        explicit AVLTreeItImpl(std::shared_ptr<Node> pointer) : it_(pointer) {
        }
        AVLTreeItImpl(const AVLTreeItImpl &other) : it_(other.it_) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<AVLTreeItImpl>(*this);
        }
        void Increment() override {
            if (!(it_->value_)) {
                throw std::runtime_error("Index out of range while increasing");
            }
            if (it_->right_) {
                it_ = it_->right_;
                while (it_->left_) {
                    it_ = it_->left_;
                }
            } else {
                auto parent = (it_->parent_).lock();
                while (parent && (parent->right_ == it_)) {
                    it_ = parent;
                    parent = (it_->parent_).lock();
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
            if (it_ && !it_->value_) {
                throw std::runtime_error("Index out of range on operator*");
            }
            return it_->value_.value();
        }

        const T *Arrow() const override {
            if (it_ && !it_->value_) {
                throw std::runtime_error("Index out of range on operator->");
            }
            return &(it_->value_).value();
        }
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::dynamic_pointer_cast<AVLTreeItImpl>(other);
            if (!casted) {
                return false;
            }
            return it_ == casted->it_;
        }

        std::shared_ptr<Node> GetPointer() {
            return it_;
        }

    private:
        std::shared_ptr<Node> it_;
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<AVLTreeItImpl>(begin_);
    }
    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<AVLTreeItImpl>(end_);
    }

    /* ---------------------------------------------------
     * ----------------PRIVATE FUNCTIONS------------------
     * ---------------------------------------------------
     */

    std::shared_ptr<BaseImpl> FindRecursive(std::shared_ptr<Node> from, const std::optional<T>& value) const {
        if (!from)
            return End();
        if (value < from->value_) {
            return FindRecursive(from->left_, value);
        } else if (from->value_ < value) {
            return FindRecursive(from->right_, value);
        } else {
            return std::make_shared<AVLTreeItImpl>(from);
        }
    };

    static std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from,
                                                         const std::optional<T>& value) {
        if (value < from->value_) {
            if (from->left_) {
                return LowerBoundRecursive(from->left_, value);
            } else {
                return std::make_shared<AVLTreeItImpl>(from);
            }
        } else if (from->value_ < value) {
            if (from->right_) {
                return LowerBoundRecursive(from->right_, value);
            } else {
                auto impl = std::make_shared<AVLTreeItImpl>(from);
                impl->Increment();
                return impl;
            }
        } else {
            return std::make_shared<AVLTreeItImpl>(from);
        }
    }

    //Set begin_ after modification
    void RecaclBegin() {
        auto node = root_;
        while (node->left_) {
            node = node->left_;
        }
        begin_ = node;
    }

    void RecaclHeight(std::shared_ptr<Node> node) {
        if (!node) {
            return;
        }

        uint8_t hl, hr;

        if (!node->left_) {
            hl = 0;
        } else {
            hl = node->left_->height_;
        }

        if (!node->right_) {
            hr = 0;
        } else {
            hr = node->right_->height_;
        }

        if (hl > hr) {
            node->height_ = hl + 1;
        } else {
            node->height_ = hr + 1;
        }
    }

    int AVLBalanceFactor(std::shared_ptr<Node> node) const {
        uint8_t hl, hr;

        if (!node->left_) {
            hl = 0;
        } else {
            hl = node->left_->height_;
        }

        if (!node->right_) {
            hr = 0;
        } else {
            hr = node->right_->height_;
        }
        return hr - hl;
    }

    void LeftRotate(std::shared_ptr<Node> from) {
        auto right_node = from->right_;

        from->right_ = right_node->left_;
        if (right_node->left_) {
            right_node->left_->parent_ = from;
        }

        right_node->left_ = from;
        right_node->parent_ = from->parent_;

        auto parent = from->parent_.lock();
        if (parent) {
            if (parent->right_ == from) {
                parent->right_ = right_node;
            } else {
                parent->left_ = right_node;
            }
        } else {
            root_ = right_node;
        }

        from->parent_ = right_node;
        RecaclHeight(from);
        RecaclHeight(right_node);
    }

    void RightRotate(std::shared_ptr<Node> from) {
        auto left_node = from->left_;

        from->left_ = left_node->right_;
        if (left_node->right_) {
            left_node->right_->parent_ = from;
        }

        left_node->right_ = from;
        left_node->parent_ = from->parent_;

        auto parent = from->parent_.lock();
        if (parent) {
            if (parent->right_ == from) {
                parent->right_ = left_node;
            } else {
                parent->left_ = left_node;
            }
        } else {
            root_ = left_node;
        }

        from->parent_ = left_node;
        RecaclHeight(from);
        RecaclHeight(left_node);
    }

    void AVLFixBalance(std::shared_ptr<Node> node) {
        RecaclHeight(node);
        if (AVLBalanceFactor(node) == 2) {
            if (node->right_ && AVLBalanceFactor(node->right_) < 0) {
                RightRotate(node->right_);
            }
            LeftRotate(node);
            return;
        }
        if (AVLBalanceFactor(node) == -2) {
            if (node->left_ && AVLBalanceFactor(node->left_) > 0) {
                LeftRotate(node->left_);
            }
            RightRotate(node);
            return;
        }
    }

    bool InsertImplementation(const std::shared_ptr<Node>& new_node) {
        if (!(root_)) {
            root_ = new_node;
            RecaclBegin();
            return true;
        }

        auto cur_node = root_;
        auto next_node = cur_node;
        while (next_node) {
            cur_node = next_node;
            if (new_node->value_ < cur_node->value_) {
                next_node = cur_node->left_;

            } else if (cur_node->value_ < new_node->value_) {
                next_node = cur_node->right_;
            } else {
                RecaclBegin();
                return false;
            }
        }

        if (new_node->value_ < cur_node->value_) {
            cur_node->left_ = new_node;
        } else {
            cur_node->right_ = new_node;
        }
        new_node->parent_ = cur_node;

        while (cur_node) {
            AVLFixBalance(cur_node);
            cur_node = cur_node->parent_.lock();
        }
        RecaclBegin();
        return true;
    }

    void EraseImplementation(std::shared_ptr<Node> delete_node) {
        auto parent = delete_node->parent_.lock();
        std::shared_ptr<Node> child_node;

         //Node doesn't have children
        if (!delete_node->right_ && !delete_node->left_) {
            if (parent) {
                if (parent->left_ == delete_node) {
                    parent->left_ = nullptr;
                } else {
                    parent->right_ = nullptr;
                }
            } else {
                root_ = nullptr;
            }
            //Node has only 1 child
        } else if ((delete_node->right_ && !delete_node->left_) || (!delete_node->right_ && delete_node->left_)) {

            child_node = delete_node->right_ ? delete_node->right_ : delete_node->left_;
            if (!parent) {
                root_ = child_node;

            } else {
                if (parent->left_ == delete_node) {
                    parent->left_ = child_node;
                } else {
                    parent->right_ = child_node;
                }
                child_node->parent_ = parent;
            }
        } else {
            std::shared_ptr<Node> swap_node = delete_node->right_;
            while (swap_node->left_) {
                swap_node = swap_node->left_;
            }
            parent = swap_node->parent_.lock();
            if (swap_node == delete_node->right_) {
                SwapWithChild(delete_node, swap_node);
                parent = swap_node;
            } else {
                SwapWithOffspring(delete_node, swap_node);
            }
        }

        while (parent) {
            AVLFixBalance(parent);
            parent = parent->parent_.lock();
        }

        RecaclBegin();
    }

    //When swap node is child
    void SwapWithChild(std::shared_ptr<Node> from_node, std::shared_ptr<Node> swap_node) {
        auto parent = from_node->parent_.lock();
        if (parent) {
            if (parent->left_ == from_node) {
                parent->left_ = swap_node;
            } else {
                parent->right_ = swap_node;
            }
        } else {
            root_ = swap_node;
        }
        swap_node->parent_ = parent;

        swap_node->left_ = from_node->left_;
        if (from_node->left_) {
            from_node->left_->parent_ = swap_node;
        }
    }

    //When swap node is not child
    void SwapWithOffspring(std::shared_ptr<Node> from_node, std::shared_ptr<Node> swap_node) {
        auto from_parent = from_node->parent_.lock();
        if (from_parent) {
            if (from_parent->left_ == from_node) {
                from_parent->left_ = swap_node;
            } else {
                from_parent->right_ = swap_node;
            }
        } else {
            root_ = swap_node;
        }

        auto swap_parent = swap_node->parent_.lock();
        swap_parent->left_ = swap_node->right_;
        if (swap_node->right_) {
            swap_node->right_->parent_ = swap_parent;
        }

        swap_node->parent_ = from_node->parent_;

        swap_node->right_ = from_node->right_;
        from_node->right_->parent_ = swap_node;

        swap_node->left_ = from_node->left_;
        if (from_node->left_) {
            from_node->left_->parent_ = swap_node;
        }
    }
};

