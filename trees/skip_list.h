#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>

template <class T>
class ITree;

template <class T>
class SkipList : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
public:
    struct Node {
        Node() {
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
            up_=nullptr;
            value_ = std::nullopt;
        }

        explicit Node(const T& value) : value_(value) {
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
            up_=nullptr;
        }

        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            down_ = other.down_;
            right_ = other.right_;
            up_=other.up_;
        }

        std::shared_ptr<Node> down_;
        std::weak_ptr<Node> left_;
        std::shared_ptr<Node> up_;
        std::shared_ptr<Node> right_;
        std::optional<T> value_;
    };

    SkipList() {
        head_ = std::make_shared<Node>();
        head_->value_=std::numeric_limits<int>::min();
        end_ = std::make_shared<Node>();
        end_->value_=std::numeric_limits<int>::max();
        head_->right_=end_;
        end_->left_=head_;
        size_ = 0;
    }

    template <class InitIterator>
    SkipList(InitIterator begin, InitIterator end): SkipList() {
        for (InitIterator cur(begin); cur != end; ++cur) {
            Insert(*cur);
        }
    }

    SkipList(std::initializer_list<T> list): SkipList() {
        for (const T& value : list) {
            Insert(value);
        }
    }

    SkipList(const SkipList& other) : SkipList() {
        std::swap(head_, other.head_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }
    SkipList(SkipList&& other) noexcept;
    SkipList(std::shared_ptr<ITree<T>> other)
            : SkipList(*dynamic_cast<SkipList<T>*>(other.get())) {
    }
    SkipList& operator=(const SkipList& other) {
        if (head_ == head_.root_) {
            return *this;
        }
        head_ = std::make_shared<Node>();
        head_->value_=std::numeric_limits<int>::min();
        end_ = std::make_shared<Node>();
        end_->value_=std::numeric_limits<int>::max();
        head_->right_=end_;
        end_->left_=head_;
        size_ = 0;
        for (const T& value : other) {
            Insert(value);
        }
        return *this;
    }
    SkipList& operator=(SkipList&& other) noexcept {
        if (head_ == head_.root_) {
            return *this;
        }
        std::swap(head_, other.head_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
        return *this;
    }

    ~SkipList() override {
        head_ = nullptr;
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
        return FindRecursive(head_, val);
    }

    void Erase(const T& value) override {
        std::optional<T> val(value);
        if (EraseImpl(head_, value)) {
            --size_;
        }
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        std::optional<T> val(value);
        return LowerBoundRecursive(head_, val);
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertRecursive(head_, new_node)) {
            ++size_;
        }
    }

    void Clear() override {
        head_ = std::make_shared<Node>();
        end_ = head_;
        size_ = 0;
    }

private:
    std::shared_ptr<Node> head_;
    std::shared_ptr<Node> end_;
    size_t size_;
    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class  SkipListItImpl : public BaseImpl {
    private:
        std::shared_ptr<Node> it_;
    public:
        SkipListItImpl() = delete;

        explicit SkipListItImpl(std::shared_ptr<Node> ptr) : it_(ptr) {}

        SkipListItImpl(const SkipListItImpl& other) : it_(other.it_) {}

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<SkipListItImpl>(*this);
        }
        void Increment() override {
            if (it_->value_==std::numeric_limits<int>::max()) {
                throw std::runtime_error("Index out of range while increasing");
            }
            while (it_->down_) it_=it_->down_;
            it_=it_->right_;
            while (it_->up_) it_=it_->up;
        }

        void Decrement() override {
            while (it_->down_) it_=it_->down_;
            it_=it_->left_;
            while (it_->up_) it_=it_->up;
            if (it_->value_==std::numeric_limits<int>::min()) {
                throw std::runtime_error("Index out of range while increasing");
            }

        }
        const T Dereferencing() const override {
            if (!it_->value_) {
                throw std::runtime_error("Index out of range on operator*");
            }
            return *(it_->value_);
        }
        const T* Arrow() const override {
            {
                if (!it_->value_) {
                    throw std::runtime_error("Index out of range on operator->");
                }
                return &(*it_->value_);
            }
        }
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::dynamic_pointer_cast<SkipListItImpl>(other);
            if (!casted) {
                return false;
            }
            return it_ == casted->it_;
        }
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<SkipListItImpl>(head_->right_);
    }
    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<SkipListItImpl>(end_);
    }

    std::shared_ptr<BaseImpl> FindRecursive(std::shared_ptr<Node> from, const std::optional<T>& value) {
        if (from.value_ == std::optional<T>::max()) {
            return End();
        }
        if (from->right_->value_ < value) {
            return FindRecursive(from->right_,value);
        }
        if (!from.down_) {
            return from;
        }
        return FindRecursive(from.down,value);
    }


    bool EraseRecursive(std::shared_ptr<Node>& from, const std::optional<T>& value) {
        if (from->value_ == std::optional<T>::max()) {
            return false;
        }
        if (from->right_->value_ < value) {
            return EraseRecursive(from->right_,value);
        }
        if (from->value_ == value) {
            from->left_->right_=from->right_;
            from->right_->left_=from->left_;
            if (!from->down_ ) {
                return true;
            }
            return EraseRecursive(from->down,value);
        }
        if (from->down_ ) {
            return EraseRecursive(from->down,value);

            bool EraseImpl(std::shared_ptr<Node>& from, const std::optional<T>& value) {
                if (from.value_ == std::optional<T>::max()) {
                    return false;
                }
                if (from.value_ < value) {
                    return FindRecursive(from->right_,value);
                }
                if (from.value_ == value) {
                    from.left_.right_=from.right_;
                    from.right_.left_=from.left_;
                    if (!from.down_ ) {
                        return true;
                    }
                    return FindRecursive(from.down,value);
                }
                if (from.down_ ) {
                    return FindRecursive(from.down,value);

                }
                return false;

            }


            std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from,
                                                          const std::optional<T>& value) {
                if (from->value_ == std::optional<T>::max()) {
                    return End();
                }
                if (from->right_->value_ < value) {
                    return LowerBoundRecursive(from->right_,value);
                }
                if (!from->down_) {
                    auto tmp = std::make_shared<SkipListItImpl> (from);
                    return tmp->Increment();
                }
                return LowerBoundRecursive(from->down_,value);
            }

            bool InsertRecursive(std::shared_ptr<Node> from, std::shared_ptr<Node> new_node) {
                // бред написал тупой
                if (from->value_ == std::optional<T>::max() || from->value_ == new_node->value_) {
                    return false;
                }
                while (from->down) {
                    from = from->down_;
                }
                if (from.value_ < new_node->value_) {
                    return InsertRecursive(from->right_,new_node->value_);
                }
                if (!from.down_) {
                    if (from->left_.lock()) {
                        from->left_->right=new_node;
                    }
                    if (from->right_.lock()) {
                        from->right->left_=new_node;
                    }
                    from
                    if (coin_flip) {
                        auto up_node = std::make_shared<Node> (new_node);
                        up_node->down_ = new_node;
                        BuildLvl(up_node);
                    }
                    return true;
                }
                return InsertRecursive(from->down_,new_node->value_);
            }

            void BuildLvl(std::shared_ptr<Node> new_node) {
                new_node
                if (coin_flip) {
                    auto up_node = std::make_shared<Node> (new_node);
                    up_node->down_ = new_node;
                    BuildLvl(up_node);
                }
            }

        };

