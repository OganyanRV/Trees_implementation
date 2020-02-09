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
            height_ = 1;
        }

        explicit Node(const T& value) {
            value_ = value;
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            height_ = 1;
        }

        Node(const Node& other) {
            value_ = other.value_;
            height_ = other.height_;
            left_ = other.left_;
            right_ = other.right_;
            parent_ = other.parent_;
        }

        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
        unsigned char height_;
        std::optional<T> value_;
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
        return LowerBoundRecursive(root_, value);
    }

    void Insert(const T &value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertImplementation(new_node)) {
            ++size_;
        }
    }
    void Erase(const T &value) override {
        std::shared_ptr<AVLTreeItImpl> find = std::static_pointer_cast<AVLTreeItImpl>(Find(value));
        if (find->IsEqual(End())) {
            return;
        }

        EraseImplementation(find->GetPointer());
        --size_;
    }

    void Clear() override {
        root_ = end_;
        begin_ = end_;
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
                auto p = (it_->parent_).lock();
                while (p && (p->right_ == it_)) {
                    it_ = p;
                    p = (it_->parent_).lock();
                }
                if (!(it_->parent_).expired()) {
                    it_ = (it_->parent_).lock();
                }
            }
        }
        void Decrement() override {
            if (it_->left_) {
                it_ = it_->left_;
                while (it_->right_) {
                    it_ = it_->right_;
                }
            } else {
                auto p = (it_->parent_).lock();
                if (p) {
                    it_ = p;
                } else {
                    throw std::runtime_error("Index out of range while decreasing");
                }
            }
        }
        const T Dereferencing() const override {
            if (it_ && !(it_->value_).has_value()) {
                throw std::runtime_error("Index out of range on operator*");
            }
            return (it_->value_).value();
        }
        const T *Arrow() const override {
            if (it_ && !(it_->value_)) {
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

    std::shared_ptr<BaseImpl> FindRecursive(std::shared_ptr<Node> from, const T& value) const {

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

    static std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from, const T& value) {
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

    //Remove pointers to end_ when tree is being modified
    void RemoveLast() {
        auto parent = end_->parent_.lock();
        if (parent) {
            parent->right_ = nullptr;
        }
        if (root_ == end_) {
            root_ = nullptr;
        }
        end_->parent_ = std::weak_ptr<Node>();
    }

    //Set begin_ and end_ after modification
    void BLCheck() {
        if (root_ == nullptr) {
            begin_ = end_;
            return;
        }
        auto node = root_;
        while (node->left_) {
            node = node->left_;
        }
        begin_ = node;
        node = root_;
        while (node->right_) {
            node = node->right_;
        }
        node->right_ = end_;
        end_->parent_ = node;
    }

    void FixHeight(std::shared_ptr<Node> node) {
        if (!node) {
            return;
        }

        unsigned char hl, hr;

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

    int BFactor(std::shared_ptr<Node> node) const {
        unsigned char hl, hr;

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
        FixHeight(from);
        FixHeight(right_node);
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
        FixHeight(from);
        FixHeight(left_node);
    }

    void AVLFixBalance(std::shared_ptr<Node> node) {
        FixHeight(node);
        if (BFactor(node) == 2) {
            if (node->right_ && BFactor(node->right_) < 0) {
                RightRotate(node->right_);
            }
            LeftRotate(node);
            return;
        }
        if (BFactor(node) == -2) {
            if (node->left_ && BFactor(node->left_) > 0) {
                LeftRotate(node->left_);
            }
            RightRotate(node);
            return;
        }
    }

    bool InsertImplementation(const std::shared_ptr<Node>& new_node) {
        RemoveLast();

        if (!(root_)) {
            root_ = new_node;
            BLCheck();
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
        BLCheck();
        return true;
    }

    void EraseImplementation(std::shared_ptr<Node> delete_node) {
        RemoveLast();

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
            ReplaceNodes(delete_node, swap_node);
            if (delete_node->right_) {
                parent = delete_node->parent_.lock();
                parent->left_ = delete_node->right_;
                delete_node->right_->parent_ = parent;

            }
        }

        while (parent) {
            AVLFixBalance(parent);
            parent = parent->parent_.lock();
        }

        BLCheck();
    }

    void ReplaceNodes(std::shared_ptr<Node>& node1, std::shared_ptr<Node>& node2) {
        auto node1_parent = (node1->parent_).lock();

        if (node1_parent) {
            if (node1_parent->left_ == node1) {
                node1_parent->left_ = node2;
            } else {
                node1_parent->right_ = node2;
            }
        }
        auto  node2_parent = node2->parent_.lock();
        if (node2_parent) {
            if (node2_parent->left_ == node2) {
                node2_parent->left_ = node1;
            } else {
                node2_parent->right_ = node1;
            }

        }
        if (node2_parent == node1) {
            node2->parent_ = node1->parent_;
            node1->parent_ = node2;
        } else {
            auto tmp = node2->parent_;
            node2->parent_ = node1->parent_;
            node1->parent_ = tmp;
        }

        node2->left_ = node1->left_;
        if (node1->left_) {
            node1->left_->parent_ = node2;
        }
        node1->left_ = nullptr;

        unsigned char height = node2->height_;
        node2->height_ = node1->height_;
        node1->height_ = height;

        auto children = node2->right_;
        node2->right_ = node1->right_;
        if (node1->right_) {
            node1->right_->parent_ = node2;
        }
        node1->right_ = children;
        if (children) {
            children->parent_ = node1;
        }
    }
};

