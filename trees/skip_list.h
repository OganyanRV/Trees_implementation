#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>

template <class T>
class ITree;

template <class T>
class SkipList : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

    class Optional {
    private:
        std::shared_ptr<T> value_;
        char info_;

    public:
        Optional() = delete;
        Optional(Optional&&) = delete;

        Optional(char newinfo) {
            info_ = newinfo;
        }

        Optional(const T& value) {
            value_ = std::make_shared<T>(const_cast<T&>(value));
            info_ = 'v';
        }

        Optional(const Optional& other) {
            value_ = other.value_;
            info_ = other.info_;
        }

        ~Optional() {
            value_ = nullptr;
        }

        std::shared_ptr<T> GetValue() const {
            return this->value_;
        }

        char GetInfo() const {
            return this->info_;
        }

        bool operator<(const Optional& other) const {
            if (this->info_ == 'v') {
                if (other.info_ == 'v') {
                    return *(this->value_) < *(other.value_);
                } else {
                    return other.info_ != 'b';
                }
            } else if (this->info_ == 'b') {
                return other.info_ != 'b';
            } else {
                return false;
            }
        }
    };
    class Random {
    public:
        static uint32_t Next(uint32_t to = 1u) {
            static Random rand = Random();
            std::uniform_int_distribution<uint32_t> dist(0, to);
            return dist(rand.gen_);
        }

    private:
        Random() {
            std::random_device device;
            gen_ = std::mt19937(device());
        }

        std::mt19937 gen_;
    };

public:
    struct Node {
        Node() = delete;

        explicit Node(char value_info) : value_(value_info) {
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
        }

        explicit Node(const T& value) : value_(value) {
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
        }

        explicit Node(const Optional& value_info) : value_(value_info) {
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
        }

        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            down_ = other.down_;
            right_ = other.right_;
        }

        ~Node() {
            left_ = right_ = down_ = nullptr;
        }

        std::shared_ptr<Node> down_;
        std::weak_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        Optional value_;
    };

    SkipList() {
        head_bot = std::make_shared<Node>('b');
        end_bot = std::make_shared<Node>('e');
        head_bot->right_ = end_bot;
        end_bot->left_ = head_bot;
        head_top = head_bot;
        end_top = end_bot;
        size_ = 0;
    }

    template <class InitIterator>
    SkipList(InitIterator begin, InitIterator end) : SkipList() {
        for (InitIterator cur(begin); cur != end; ++cur) {
            Insert(*cur);
        }
    }

    SkipList(std::initializer_list<T> list) : SkipList() {
        for (const T& value : list) {
            Insert(value);
        }
    }

    SkipList(const SkipList& other) : SkipList() {
        for (const T& value : other) {
            Insert(value);
        }
    }
    SkipList(SkipList&& other) noexcept {
        std::swap(head_top, other.head_top);
        std::swap(end_top, other.end_top);
        std::swap(head_bot, other.head_bot);
        std::swap(end_bot, other.end_bot);
        std::swap(size_, other.size_);
    }
    SkipList(std::shared_ptr<ITree<T>> other) : SkipList(*dynamic_cast<SkipList<T>*>(other.get())) {
    }

    SkipList& operator=(const SkipList& other) {
        if (head_top == other.head_top) {
            return *this;
        }
        head_bot = std::make_shared<Node>('b');
        end_bot = std::make_shared<Node>('e');
        head_bot->right_ = end_bot;
        end_bot->left_ = head_bot;
        head_top = head_bot;
        end_top = end_bot;
        size_ = 0;
        for (const T& value : other) {
            Insert(value);
        }
        return *this;
    }

    SkipList& operator=(SkipList&& other) noexcept {
        if (head_top == other.head_top) {
            return *this;
        }
        std::swap(head_top, other.head_top);
        std::swap(end_top, other.end_top);
        std::swap(head_bot, other.head_bot);
        std::swap(end_bot, other.end_bot);
        std::swap(size_, other.size_);
        return *this;
    }

    ~SkipList() override {
        head_top = head_bot = end_top = end_bot = nullptr;
        size_ = 0;
    }

    [[nodiscard]] size_t Size() const override {
        return size_;
    }

    [[nodiscard]] bool Empty() const override {
        return !size_;
    }

    std::shared_ptr<BaseImpl> Find(const T& value) const override {
        Optional val(value);
        return FindImpl(head_top, val);
    }

    void Erase(const T& value) override {
        Optional val(value);
        if (EraseImpl(head_top, val)) {
            --size_;
        }
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        Optional val(value);
        return LowerBoundImpl(head_top, val);
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertImpl(head_top, new_node)) {
            ++size_;
        }
    }

    void Clear() override {
        head_bot = std::make_shared<Node>('b');
        end_bot = std::make_shared<Node>('e');
        head_bot->right_ = end_bot;
        end_bot->left_ = head_bot;
        head_top = head_bot;
        end_top = end_bot;
        size_ = 0;
    }

private:
    std::shared_ptr<Node> head_top;
    std::shared_ptr<Node> end_top;
    std::shared_ptr<Node> head_bot;
    std::shared_ptr<Node> end_bot;
    size_t size_;
    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class SkipListItImpl : public BaseImpl {
    private:
        std::shared_ptr<Node> it_;

    public:
        SkipListItImpl() = delete;

        explicit SkipListItImpl(std::shared_ptr<Node> ptr) : it_(ptr) {
        }

        SkipListItImpl(const SkipListItImpl& other) : it_(other.it_) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<SkipListItImpl>(*this);
        }

        void Increment() override {
            if (!it_->right_) {
                throw std::runtime_error("Index out of range while increasing");
            }
            it_ = it_->right_;
        }

        void Decrement() override {
            it_ = it_->left_.lock();
            if (!it_->left_.lock()) {
                throw std::runtime_error("Index out of range while decreasing");
            }
        }

        const T Dereferencing() const override {
            if (it_->value_.GetInfo() != 'v') {
                throw std::runtime_error("Index out of range on operator*");
            }
            return *(it_->value_.GetValue());
        }

        const T* Arrow() const override {
            if (it_->value_.GetInfo() != 'v') {
                throw std::runtime_error("Index out of range on operator->");
            }
            return &(*it_->value_.GetValue());
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
        return std::make_shared<SkipListItImpl>(head_bot->right_);
    }

    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<SkipListItImpl>(end_bot);
    }

    std::shared_ptr<BaseImpl> FindImpl(std::shared_ptr<Node> from, const Optional& value) const {
        while (true) {
            if (!from->right_) {
                return End();
            }
            if (from->right_->value_ < value) {
                from = from->right_;
            } else if (!from->down_) {
                if (value < from->right_->value_) {
                    return End();
                }
                return std::make_shared<SkipListItImpl>(from->right_);
            } else {
                from = from->down_;
            }
        }
    }

    bool EraseImpl(std::shared_ptr<Node>& from, const Optional& value) {
        if (!from->right_) {
            return false;
        }
        if (from->right_->value_ < value) {
            return EraseImpl(from->right_, value);
        }
        if (value < from->right_->value_) {
            if (!from->down_) {
                return false;
            } else {
                return EraseImpl(from->down_, value);
            }
        }
        auto cur_node = from->right_;
        auto next_node = cur_node->right_;
        next_node->left_ = from;
        from->right_ = next_node;
        if (from == head_top && from->right_ == end_top) {
            if (head_top->down_) {
                head_top = head_top->down_;
                end_top = end_top->down_;
            }
        }
        while (cur_node->down_) {
            cur_node = cur_node->down_;
            auto prev_from = cur_node->left_.lock();
            next_node = cur_node->right_;
            prev_from->right_ = next_node;
            next_node->left_ = prev_from;
            if (prev_from == head_top && prev_from->right_ == end_top) {
                if (head_top->down_) {
                    head_top = head_top->down_;
                    end_top = end_top->down_;
                }
            }
        }
        return true;
    }

    static std::shared_ptr<BaseImpl> LowerBoundImpl(std::shared_ptr<Node> from,
                                                    const Optional& value) {
        while (true) {
            if (from->right_->value_ < value) {
                from = from->right_;
            } else if (!from->down_) {
                return std::make_shared<SkipListItImpl>(from->right_);
            } else {
                from = from->down_;
            }
        }
    }

    bool InsertImpl(std::shared_ptr<Node> from, std::shared_ptr<Node> new_node) {

        std::vector<std::shared_ptr<Node>> node_path;
        while (true) {
            if (!from->right_) {
                return false;
            } else if (from->right_->value_ < new_node->value_) {
                from = from->right_;
            } else {
                if (from->down_) {
                    node_path.push_back(from);
                    from = from->down_;
                } else {
                    if (new_node->value_ < from->right_->value_) {
                        new_node->left_ = from;
                        new_node->right_ = from->right_;
                        from->right_->left_ = new_node;
                        from->right_ = new_node;
                        BuildLvl(node_path, new_node);
                        return true;
                    }
                    return false;
                }
            }
        }
    }

    void BuildLvl(std::vector<std::shared_ptr<Node>> node_path, std::shared_ptr<Node> from) {
        while (!Random::Next()) {
            std::shared_ptr<Node> up_node;
            up_node = std::make_shared<Node>(from->value_);
            up_node->down_ = from;
            if (node_path.size() != 0) {
                auto prev = node_path.back();
                up_node->left_ = prev;
                up_node->right_ = prev->right_;
                prev->right_->left_ = up_node;
                prev->right_ = up_node;
                node_path.pop_back();
            } else {
                std::shared_ptr<Node> new_head, new_end, tmp_head, tmp_end;
                tmp_head = head_top;
                tmp_end = end_top;
                new_head = std::make_shared<Node>('b');
                new_end = std::make_shared<Node>('e');
                new_head->down_ = tmp_head;
                new_head->right_ = up_node;
                up_node->left_ = new_head;
                up_node->right_ = new_end;
                new_end->down_ = tmp_end;
                new_end->left_ = up_node;
                head_top = new_head;
                end_top = new_end;
                break;
            }
            from = up_node;
        }
        return;
    }
};
