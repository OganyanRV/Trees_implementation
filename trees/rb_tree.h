#pragma once
#include <initializer_list>
#include <memory>
#include <optional>
#include <exception>

template <class T>
class ITree;

template <class T>
bool operator<(const T& value1, const std::optional<T>& value2) {
    if (!value2) {
        return true;
    } else {
        return (value1 < value2.value());
    }
}


template <class T>
class RBTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    struct Node {
        Node() {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            is_red_ = true;
        }

        explicit Node(const T& value) : value_(value) {
            left_ = nullptr;
            right_ = nullptr;
            parent_ = std::weak_ptr<Node>();
            is_red_ = true;
        }

        Node(const Node& other) : value_(other.value_) {
            is_red_ = other.is_red_;
            left_ = other.left_;
            right_ = other.right_;
            parent_ = other.parent_;
        }

        std::shared_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        std::weak_ptr<Node> parent_;
        bool is_red_;  // 0 - black, 1 - red;
        std::optional<T> value_;
    };

    RBTree() {
        end_ = std::make_shared<Node>();
        begin_ = end_;
        root_ = end_;
        size_ = 0;
    }

    template <class InitIterator>
    RBTree(InitIterator begin, InitIterator end) : RBTree() {
        for (InitIterator cur(begin); cur != end; ++cur) {
            Insert(*cur);
        }
    }
    RBTree(std::initializer_list<T> list)  : RBTree() {
        for (const T& value : list) {
            Insert(value);
        }
    }

    RBTree(const RBTree& other) : RBTree() {
        for (const T& value : other) {
                Insert(value);
        }
    }
    RBTree(RBTree&& other) noexcept : RBTree() {
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }
    RBTree(std::shared_ptr<ITree<T>> other) : RBTree(*dynamic_cast<RBTree<T>*>(other.get())) {
    }
    RBTree& operator=(const RBTree& other) {
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
    RBTree& operator=(RBTree&& other) noexcept {
        if (root_ == other.root_) {
            return *this;
        }
        std::swap(root_, other.root_);
        std::swap(begin_, other.begin_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
        return *this;
    }

    ~RBTree() override {
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
        return FindRecursive(root_, value);
    }
    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        return LowerBoundRecursive(root_, value);
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertImplementation(new_node)) {
            ++size_;
        }
    }

    void Erase(const T& value) override {
        std::shared_ptr<RBTreeItImpl> find = std::static_pointer_cast<RBTreeItImpl>(Find(value));
        if (find->IsEqual(End())) {
            return;
        }

        EraseImplementation(find->GetPointer());
        --size_;
    }

    void Clear() override {
        end_->parent_ = std::weak_ptr<Node>();
        root_ = end_;
        begin_ = end_;
        size_ = 0;
    }

    void printT(std::ostream& ostr, std::shared_ptr<Node> p, int lvl) const {

        if (p) {
            printT(ostr, p->left_, lvl + 1);
            for (int i = 0; i < lvl; i++)
                ostr << "   ";
            if (p->value_) {
                ostr << (p->is_red_ ? 'r' : 'b') << *(p->value_) << '\n';
            } else
                ostr << (p->is_red_ ? 'r' : 'b') << "+";
            printT(ostr, p->right_, lvl + 1);
        }
    }

    friend inline std::ostream& operator<<(std::ostream& ostr, const RBTree<T>& tree) {
        tree.printT(ostr, tree.root_, 0);
        return ostr;
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

    class RBTreeItImpl : public BaseImpl {
    public:
        RBTreeItImpl() = delete;
        explicit RBTreeItImpl(std::shared_ptr<Node> pointer)  : it_(pointer) {
        }
        RBTreeItImpl(const RBTreeItImpl& other) : it_(other.it_) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<RBTreeItImpl>(*this);
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
            if (it_ && !(it_->value_).has_value()) {
                throw std::runtime_error("Index out of range on operator*");
            }
        return (it_->value_).value();
        }

        const T* Arrow() const override {
            if (it_ && !(it_->value_)) {
                throw std::runtime_error("Index out of range on operator->");
            }
            return &(it_->value_).value();
        }

        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::dynamic_pointer_cast<RBTreeItImpl>(other);
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
        return std::make_shared<RBTreeItImpl>(begin_);
    }
    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<RBTreeItImpl>(end_);
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
            return std::make_shared<RBTreeItImpl>(from);
        }
    };

    static std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from, const T& value) {
        if (value < from->value_) {
            if (from->left_) {
                return LowerBoundRecursive(from->left_, value);
            } else {
                return std::make_shared<RBTreeItImpl>(from);
            }
        } else if (from->value_ < value) {
            if (from->right_) {
                return LowerBoundRecursive(from->right_, value);
            } else {
                auto impl = std::make_shared<RBTreeItImpl>(from);
                impl->Increment();
                return impl;
            }
        } else {
            return std::make_shared<RBTreeItImpl>(from);
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
        //end_->parent_ = std::weak_ptr<Node>();
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
        RBBalancing(new_node);

        //std::cout << "\nAdding node: " << new_node->value_.value() << "\n------------------------------------------\n" << *this;

        BLCheck();
        return true;
    }

    void RBBalancing(std::shared_ptr<Node> from) {
        // Node doesn't have a parent
        if (from->parent_.expired()) {
            return;
        }

        auto parent = (from->parent_).lock();

        // Everything is okey
        if (parent->is_red_ == false) {
            return;
        }

        // Node doesn't have a grandparent
        if ((parent->parent_).expired()) {
            parent->is_red_ = false;
            return;
        }

        auto grandparent = (parent->parent_).lock();

        if (grandparent->right_ == parent) {
            auto uncle = grandparent->left_;
            // Uncle is red
            if (uncle && uncle->is_red_ == true) {
                parent->is_red_ = false;
                uncle->is_red_ = false;
                grandparent->is_red_ = true;
                RBBalancing(grandparent);
                return;
            }
            // Node is left
            if (parent->left_ == from) {
                if (grandparent->right_ == parent) {
                    grandparent->right_ = from;
                } else {
                    grandparent->left_ = from;
                }
                parent->parent_ = from;
                parent->left_ = from->right_;
                if (from->right_) {
                    from->right_->parent_ = parent;
                }
                from->right_ = parent;
                from->parent_ = grandparent;
                RBBalancing(parent);
                return;
            } else {
                // Big rotation
                if (!(grandparent->parent_).expired()) {
                    auto grandgrandparent = (grandparent->parent_).lock();
                    if (grandgrandparent->right_ == grandparent) {
                        grandgrandparent->right_ = parent;
                    } else {
                        grandgrandparent->left_ = parent;
                    }
                    parent->parent_ = grandgrandparent;
                } else {
                    root_ = parent;
                    parent->parent_ = std::weak_ptr<Node>();
                }
                grandparent->right_ = parent->left_;
                if (parent->left_) {
                    parent->left_->parent_ = grandparent;
                }
                parent->left_ = grandparent;
                grandparent->parent_ = parent;
                parent->is_red_ = false;
                grandparent->is_red_ = true;
                return;
            }
         // Node is right
        } else {
            auto uncle = grandparent->right_;
            // Uncle is red
            if (uncle && uncle->is_red_ == true) {
                parent->is_red_ = false;
                uncle->is_red_ = false;
                grandparent->is_red_ = true;
                RBBalancing(grandparent);
                return;
            }
            // Node is right
            if (parent->right_ == from) {
                if (grandparent->right_ == parent) {
                    grandparent->right_ = from;
                } else {
                    grandparent->left_ = from;
                }
                parent->parent_ = from;
                parent->right_ = from->left_;
                if (from->left_) {
                    from->left_->parent_ = parent;
                }
                from->left_ = parent;
                from->parent_ = grandparent;
                RBBalancing(parent);
                return;
            } else {
                if (!(grandparent->parent_).expired()) {
                    auto ggrandparent = (grandparent->parent_).lock();
                    if (ggrandparent->left_ == grandparent) {
                        ggrandparent->left_ = parent;
                    } else {
                        ggrandparent->right_ = parent;
                    }
                    parent->parent_ = ggrandparent;
                } else {
                    root_ = parent;
                    parent->parent_ = std::weak_ptr<Node>();
                }
                grandparent->left_ = parent->right_;
                if (parent->right_) {
                    parent->right_->parent_ = grandparent;
                }
                parent->right_ = grandparent;
                grandparent->parent_ = parent;
                //RBBalancing(parent);
                parent->is_red_ = false;
                grandparent->is_red_ = true;
                return;
            }
        }
    }


        void EraseImplementation(std::shared_ptr<Node> delete_node) {
        RemoveLast();

        auto parent = delete_node->parent_.lock();
        std::shared_ptr<Node> child_node;
        std::shared_ptr<Node> swap_node = nullptr;
        bool swapHasChild = false;
        //Node doesn't have children
        if (!delete_node->right_ && !delete_node->left_) {
            FixBalance(delete_node);
            if (parent) {
                if (parent->left_ == delete_node) {
                    parent->left_ = nullptr;
                } else {
                    parent->right_ = nullptr;
                }
            } else {
                root_ = nullptr;
            }
//            //When delete the last real node (for iterators)
//            if (end_->parent_.lock() == delete_node) {
//                delete_node->right_ = end_;
//            }
//            BLCheck();
//            return;
            //Node has only 1 child
        } else if ((delete_node->right_&&!delete_node->left_) || (!delete_node->right_&&delete_node->left_)) {

            child_node = delete_node->right_ ? delete_node->right_ : delete_node->left_;
            if (!parent) {
                root_ = child_node;
                child_node->parent_ = std::weak_ptr<Node>();
            } else {
                if(parent->left_ == delete_node) {
                    parent->left_ = child_node;
                } else {
                    parent->right_ = child_node;
                }
                child_node->parent_ = parent;

                if (!delete_node->is_red_ && child_node->is_red_ ) {
                    child_node->is_red_ =  false;
                } else if  (!delete_node->is_red_ && !child_node->is_red_) {
                    FixBalance(child_node);
                }
            }
//            //When delete the last real node (for iterators)
//            if (end_->parent_.lock() == delete_node) {
//                delete_node->right_ = end_;
//            }
//            BLCheck();
//            return;
        } else {
            swap_node = delete_node->right_;

            while (swap_node->left_) {
                swap_node = swap_node->left_;
            }
            if (swap_node->right_) {
                swapHasChild = true;
            }
            if (delete_node->right_ == swap_node) {
                SwapWithChild(delete_node, swap_node);
            } else {
                SwapWithOffspring(delete_node, swap_node);
            }
            //if (delete_node != swap_node) {
//                ReplaceNodes(delete_node, swap_node);
//                if (delete_node == root_) {
//                    root_ = swap_node;
//                }
            //}
            if (swapHasChild) {
                parent = delete_node->parent_.lock();

                if (parent->right_ == delete_node) {
                    parent->right_ = delete_node->right_;
                } else {
                    parent->left_ = delete_node->right_;
                }
                delete_node->right_->parent_ = parent;
                if (!delete_node->right_->is_red_) {
                    FixBalance(delete_node->right_);
                }
            } else {
                parent = delete_node->parent_.lock();

                if (!delete_node->is_red_) {
                    FixBalance(delete_node);
                }
                if (parent->right_ == delete_node) {
                    parent->right_ = nullptr;
                } else {
                    parent->left_ = nullptr;
                }
            }
        }
        //When delete the last real node (for iterators)
        if (end_->parent_.lock() == delete_node) {
            delete_node->right_ = end_;
        }
        BLCheck();
    }

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



        from_node->right_ = swap_node->right_;
        if (swap_node->right_) {
            swap_node->right_->parent_ = from_node;
        }

        swap_node->right_ = from_node;
        from_node->parent_ = swap_node;



        swap_node->left_ = from_node->left_;
        if (from_node->left_) {
            from_node->left_->parent_ = swap_node;
        }

        from_node->left_ = nullptr;

        bool color = swap_node->is_red_;
        swap_node->is_red_ = from_node->is_red_;
        from_node->is_red_ = color;
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

        auto tmp_parent = from_node->parent_;
        from_node->parent_ = swap_node->parent_;
        swap_node->parent_ = tmp_parent;

        auto tmp_right = swap_node->right_;
        swap_node->right_ = from_node->right_;
        from_node->right_->parent_ = swap_node;
        from_node->right_ = tmp_right;
        if (from_node->right_) {
            from_node->right_->parent_ = from_node;
        }

        swap_node->left_ = from_node->left_;
        if (from_node->left_) {
            from_node->left_->parent_ = swap_node;
        }

        from_node->left_ = nullptr;

        bool color = swap_node->is_red_;
        swap_node->is_red_ = from_node->is_red_;
        from_node->is_red_ = color;
    }

    void FixBalance(std::shared_ptr<Node> from) {
        while (!from->is_red_ && root_ != from) {
            auto parent = from->parent_.lock();
            if (!parent) {
                return;
            }
            auto sibling = (parent->left_ == from) ? parent->right_ : parent->left_;
            if (!sibling) {
                return;
            }
            auto sibling_right = sibling->right_;
            auto sibling_left = sibling->left_;
            if (parent->left_ == from) {
                //Brother is red;
                if (sibling->is_red_) {
                    sibling->is_red_ = false;
                    parent->is_red_ = true;
                    LeftRotate(parent);
                    sibling = (parent->left_ == from) ? parent->right_ : parent->left_;
                }
                //Brother and children are black
                if ((!sibling_left || !(sibling_left->is_red_)) &&
                    (!sibling_right || !(sibling_right->is_red_))) {
                    sibling->is_red_ = true;
                    from = parent;
                } else {
                    if (!sibling_right || !(sibling_right->is_red_)) {
                        sibling->is_red_ = true;
                        sibling_left->is_red_ = false;
                        RightRotate(sibling);
                        sibling = (parent->left_ == from) ? parent->right_ : parent->left_;

                        sibling_right = sibling->right_;
                        sibling_left = sibling->left_;
                    }
                    sibling->is_red_ = parent->is_red_;
                    parent->is_red_ = false;
                    if (sibling_right) {
                        sibling->right_->is_red_ = false;
                    }
                    LeftRotate(parent);
                    from = root_;
                }
            } else {
                //Brother is red;
                if (sibling->is_red_) {
                    sibling->is_red_ = false;
                    parent->is_red_ = true;
                    RightRotate(parent);
                    sibling = (parent->left_ == from) ? parent->right_ : parent->left_;
                }
                //Brother is black
                if ((!sibling->left_ || !(sibling->left_->is_red_)) &&
                    (!sibling->right_ || !(sibling->right_->is_red_))) {
                    sibling->is_red_ = true;
                    from = parent;
                } else {
                    if (!sibling_left || !(sibling_left->is_red_)) {
                        sibling->is_red_ = true;
                        sibling_right->is_red_ = false;
                        LeftRotate(sibling);
                        sibling = (parent->left_ == from) ? parent->right_ : parent->left_;
                        sibling_right = sibling->right_;
                        sibling_left = sibling->left_;
                    }
                    sibling->is_red_ = parent->is_red_;
                    parent->is_red_ = false;
                    if (sibling_left) {
                        sibling_left->is_red_ = false;
                    }
                    RightRotate(parent);
                    from = root_;
                }
            }
        }
        from->is_red_ = false;
        root_->is_red_ = false;
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

        bool color = node2->is_red_;
        node2->is_red_ = node1->is_red_;
        node1->is_red_ = color;


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

    void LeftRotate(std::shared_ptr<Node>& from) {
        auto parent = from->parent_.lock();
        auto right_node = from->right_;
        std::shared_ptr<Node> next_node = nullptr;
        if (right_node) {
            next_node = right_node->left_;
        }

        if (parent) {
            if (parent->left_ == from) {
                parent->left_ = right_node;
            } else {
                parent->right_ = right_node;
            }
        } else {
            root_ = right_node;
        }

        if (right_node) {
            right_node->parent_ = parent;
            right_node->left_ = from;
        }

        if (next_node) {
            next_node->parent_ = from;
        }
        from ->parent_ = right_node;
        from->right_ = next_node;
    }

    void RightRotate(std::shared_ptr<Node>& from) {
        auto parent = from->parent_.lock();
        auto left_node = from->left_;
        std::shared_ptr<Node> prev_node = nullptr;

        if (left_node) {
            prev_node = left_node->right_;
        }

        if (parent) {
            if (parent->right_ == from) {
                parent->right_ = left_node;
            } else {
                parent->left_ = left_node;
            }
        } else {
            root_ = left_node;
        }

        if (left_node) {
            left_node->parent_ = parent;
            left_node->right_ = from;
        }

        if (prev_node) {
            prev_node->parent_ = from;
        }
        from ->parent_ = left_node;
        from->left_ = prev_node;
    }
};
