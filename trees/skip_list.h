#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <stack>
#include <random>

template <class T>
class ITree;

template <class T>
class SkipList : public ITree<T> {
public:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

    class Optional {
    private:
        std::shared_ptr<T> value;
        char info;
    public:

        Optional() {
            info='v';
        }

        Optional(const T& value_) {
            value = std::make_shared<T>(const_cast<T&>(value_));
            info = 'v';
        }

        void setInfo(char newinfo) {
            this->info=newinfo;
        }

        std::shared_ptr<T> GetValue() {
            return this->value;
        }

        char GetInfo() {
            return this->info;
        }

        bool operator<(const Optional& rhs) const {
             if (this->info == 'v') {
                if (rhs.info == 'v') {
                    return *(this->value) < *(rhs.value);
                } else {
                    return rhs.info != 'b';
                }
            } else if (this->info == 'b') {
                return rhs.info == 'v';
            } else {
                return rhs.info != 'v';
            }
        }
    };
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
                std::uniform_int_distribution<uint32_t>(0, 1);
        }
        std::mt19937 gen_;
        std::uniform_int_distribution<uint32_t> dist_;
    };


public:

    struct Node {
        Node() : value_(){
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
        }

        explicit Node(const T& value) : value_(value) {
            left_ = std::weak_ptr<Node>();
            down_ = nullptr;
            right_ = nullptr;
        }

        Node(const Node& other) : value_(other.value_) {
            left_ = other.left_;
            down_ = other.down_;
            right_ = other.right_;
        }

        std::shared_ptr<Node> down_;
        std::weak_ptr<Node> left_;
        std::shared_ptr<Node> right_;
        Optional value_;
    };

    SkipList() {
        head_ = std::make_shared<Node>();
        end_ = std::make_shared<Node>();
        head_->value_.setInfo('b');
        end_->value_.setInfo('e');
        head_->right_ = end_;
        end_->left_ = head_;
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
        for (const T &value : other) {
            Insert(value);
        }

    }
    SkipList(SkipList&& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(end_, other.end_);
        std::swap(size_, other.size_);
    }
    SkipList(std::shared_ptr<ITree<T>> other) : SkipList(*dynamic_cast<SkipList<T>*>(other.get())) {
    }

    SkipList& operator=(const SkipList& other) {
        if (head_ == other.head_) {
            return *this;
        }
        head_ = std::make_shared<Node>();
        end_ = std::make_shared<Node>();
        head_->right_ = end_;
        end_->left_ = head_;
        size_ = 0;
        for (const T& value : other) {
            Insert(value);
        }
        return *this;
    }

    SkipList& operator=(SkipList&& other) noexcept {
        if (head_ == other.head_) {
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
        Optional val(value);
        return FindRecursive(head_, val);
    }

    void Erase(const T& value) override {
        Optional val(value);
        if (EraseRecursive(head_, val)) {
            --size_;
        }
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        Optional val(value);
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
            while (it_->down_) {
                it_ = it_->down_;
            }
            if (!it_->right_) {
                throw std::runtime_error("Index out of range while increasing");
            }
            it_ = it_->right_;
        }

        void Decrement() override {
            while (it_->down_) {
                it_ = it_->down_;
            }
            if (!it_->left_.lock()) {
                throw std::runtime_error("Index out of range while increasing");
            }
            it_ = it_->left_.lock();
        }

        const T Dereferencing() const override {
            if (it_->value_.GetInfo()!='v')  {
                throw std::runtime_error("Index out of range on operator*");
            }
            return *(it_->value_.GetValue());
        }

        const T* Arrow() const override {
            if (it_->value_.GetInfo()!='v') {
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
        return std::make_shared<SkipListItImpl>(head_->right_);
    }

    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<SkipListItImpl>(end_);
    }

    std::shared_ptr<BaseImpl> FindRecursive(std::shared_ptr<Node> from,
                                            const Optional& value) const {
        if (!from->right_) {
            return End();
        }
        if (from->right_->value_ < value) {
            return FindRecursive(from->right_, value);
        }
        if (!from->down_) {
            if (value < from->right_->value_ ) {
                return End();
            }
            return std::make_shared<SkipListItImpl>(from->right_);
        }
        return FindRecursive(from->down_, value);
    }

    bool EraseRecursive(std::shared_ptr<Node>& from, const Optional& value) {
        if (!from->right_) {
            return false;
        }
        if (from->right_->value_ < value) {
            return EraseRecursive(from->right_, value);
        }
        if (!from->down_) {
            if (value < from->right_->value_ ) {
                return false;
            }
            from->left_.lock()->right_ = from->right_;
            from->right_->left_ = from->left_;
            while(from->down_){
                from=from->down_;
                from->left_.lock()->right_ = from->right_;
                from->right_->left_ = from->left_;
            }
            return true;
        }
        return EraseRecursive(from->down_, value);
    }

    std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from,
                                                  const Optional& value) const {
        if (!from->right_) {
            return End();
        }
        if (from->right_->value_ < value) {
            return LowerBoundRecursive(from->right_, value);
        }
        if (!from->down_) {
           //auto tmp = std::make_shared<SkipListItImpl>(from);
            //return tmp->Increment();
            return std::make_shared<SkipListItImpl>(from->right_);
        }
        return LowerBoundRecursive(from->down_, value);
    }

    bool InsertRecursive(std::shared_ptr<Node> from, std::shared_ptr<Node> new_node) {
        // бред написал тупой
        //std::stack<std::shared_ptr<Node>> node_path;
        std::vector<std::shared_ptr<Node>> node_path;
        while (1) {
            if (!from->right_) {
                //этого случая даже не будет
                return false;
            }
            else if (from->right_->value_ < new_node->value_) {
                from=from->right_;
            }
            else {
                if (from->down_) {
                    // node_path.push(from);
                    node_path.push_back(from);
                    from = from->down_;
                }
                else {
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

    //void BuildLvl(std::stack<std::shared_ptr<Node>> node_path, std::shared_ptr<Node> from) {
    void BuildLvl(std::vector<std::shared_ptr<Node>> node_path, std::shared_ptr<Node> from) {
        if (Random::Next()) {
            std::shared_ptr<Node>up_node;
            up_node = std::make_shared<Node>();
            up_node->down_ = from;
            if (node_path.size() != 0) {
                auto prev = node_path.back();
                // auto prev = node_path.top();
                up_node->left_ = prev;
                up_node->right_ = prev->right_;
                prev->right_->left_ = up_node;
                prev->right_ = up_node;
                // node_path.pop();
                node_path.pop_back();
            }
            else {
                up_node->down_ = from;
                std::shared_ptr<Node> new_head, new_end;
                new_head = std::make_shared<Node>();
                new_end = std::make_shared<Node>();
                new_head->down_ = head_;
                new_head->right_ = up_node;
                up_node->right_ = new_end;
                new_end->down_ = end_;
                new_end->left_ = up_node;
                new_head->value_.setInfo('b');
                new_end->value_.setInfo('e');
            }
            BuildLvl(node_path, up_node);
            return;
        }

    }
};

