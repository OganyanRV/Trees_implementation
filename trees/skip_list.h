template <class T>
class ITree;

template <class T>
class SkipList : public ITree<T> {
public:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

private:
    class Optional {
    private:
        std::shared_ptr<T> value_;
        char info_;

    public:
        Optional() = delete;

        Optional(const char newinfo) {
            info_ = newinfo;
        }

        Optional(const T& value) {
            value_ = std::make_shared<T>(const_cast<T&>(value));
            info_ = 'v';
        }


        std::shared_ptr<T> GetValue() {
            return this->value_;
        }

        char GetInfo() {
            return this->info_;
        }

        bool operator<(const Optional& rhs) const {
            if (this->info_ == 'v') {
                if (rhs.info_ == 'v') {
                    return *(this->value_) < *(rhs.value_);
                } else {
                    return rhs.info_ != 'b';
                }
            } else if (this->info_ == 'b') {
                return rhs.info_ == 'v';
            } else {
                return rhs.info_ != 'v';
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
        int limit=1;
        Random() {
            std::random_device device;
            gen_ = std::mt19937(device());
            dist_ = std::uniform_int_distribution<uint32_t>(0, limit);
        }
        Random(int newlimit) {
            limit = newlimit;
            std::random_device device;
            gen_ = std::mt19937(device());
            dist_ = std::uniform_int_distribution<uint32_t>(0, limit);
        }
        std::mt19937 gen_;
        std::uniform_int_distribution<uint32_t> dist_;
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

        explicit Node(std::shared_ptr<Optional> value_info) : value_(*value_info) {
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
        head_top = std::make_shared<Node>('b');
        end_top = std::make_shared<Node>('e');
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
        head_top = std::make_shared<Node>('b');
        end_top = std::make_shared<Node>('e');
        head_bot = std::make_shared<Node>('b');
        end_bot = std::make_shared<Node>('e');
        head_top->right_ = end_top;
        end_top->left_ = head_top;
        head_bot->right_ = end_bot;
        end_bot->left_ = head_bot;
        head_top->down_ = head_bot;
        end_top->down_ = end_bot;
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
        head_top = nullptr;
        head_bot = nullptr;
        end_top = nullptr;
        end_bot = nullptr;
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
        return FindRecursive(head_top, val);
    }

    void Erase(const T& value) override {
        Optional val(value);
        if (EraseRecursive(head_top, val)) {
            --size_;
        }
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        Optional val(value);
        return LowerBoundRecursive(head_top, val);
    }

    void Insert(const T& value) override {
        std::shared_ptr<Node> new_node = std::make_shared<Node>(value);
        if (InsertRecursive(head_top, new_node)) {
            ++size_;
        }
    }

    void Clear() override {
        head_top = std::make_shared<Node>('b');
        end_top = std::make_shared<Node>('e');
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
            if (!casted || !it_) {
                return false;
            }
            auto bot_it = it_;
            auto bot_other = casted->it_;
            while (bot_it->down_) {
                bot_it = bot_it->down_;
            }
            while (bot_other->down_) {
                bot_other = bot_other->down_;
            }
            return bot_other == bot_it;
        }
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<SkipListItImpl>(head_bot->right_);
    }

    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<SkipListItImpl>(end_top);
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
            if (value < from->right_->value_) {
                return End();
            }
            return std::make_shared<SkipListItImpl>(from->right_);
        }
        return FindRecursive(from->down_, value);
    }

    static bool EraseRecursive(std::shared_ptr<Node>& from, const Optional& value) {
        if (!from->right_) {
            return false;
        }
        if (from->right_->value_ < value) {
            return EraseRecursive(from->right_, value);
        }
        if (value < from->right_->value_) {
            if (!from->down_) {
                return false;
            } else {
                return EraseRecursive(from->down_, value);
            }
        }
        auto cur_node = from->right_;
        auto new_from = cur_node->right_;
        new_from->left_ = from;
        from->right_ = new_from;
        RemoveLevels(cur_node);
        return true;
    }

    static void RemoveLevels(std::shared_ptr<Node> from) {
        if (from->down_) {
            from = from->down_;
            auto prev_from = from->left_.lock();
            auto new_from = from->right_;
            prev_from->right_ = new_from;
            new_from->left_.lock() = prev_from;
            RemoveLevels(from);
            return;
        }
        return;
    }

    static std::shared_ptr<BaseImpl> LowerBoundRecursive(std::shared_ptr<Node> from,
                                                  const Optional& value)  {
        if (from->right_->value_ < value) {
            return LowerBoundRecursive(from->right_, value);
        }
        if (!from->down_) {
            return std::make_shared<SkipListItImpl>(from->right_);
        }
        return LowerBoundRecursive(from->down_, value);
    }

    bool InsertRecursive(std::shared_ptr<Node> from, std::shared_ptr<Node> new_node) {

        std::vector<std::shared_ptr<Node>> node_path;
        while (1) {
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

    void BuildLvl(std::vector<std::shared_ptr<Node>> &node_path, std::shared_ptr<Node> from) {
        while (!Random::Next()) {
            std::shared_ptr<Node> up_node;
            up_node = std::make_shared<Node>(std::make_shared<Optional> (from->value_));
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
            }
            from = up_node;
        }
        return;
    }
};
