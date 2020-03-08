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
            value_ = std::nullopt;
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

        std::shared_ptr<Node> left_;
        std::weak_ptr<Node> down_;
        std::shared_ptr<Node> right_;
        std::optional<T> value_;
    };

    SkipList();

    template <class InitIterator>
    SkipList(InitIterator begin, InitIterator end);
    SkipList(std::initializer_list<T> list);
    SkipList(const SkipList& other);
    SkipList(SkipList&& other) noexcept;
    SkipList(std::shared_ptr<ITree<T>> other)
        : SkipList(*dynamic_cast<SkipList<T>*>(other.get())) {
    }
    SkipList& operator=(const SkipList& other);
    SkipList& operator=(SkipList&& other) noexcept;

    ~SkipList() override;

    [[nodiscard]] size_t Size() const override;
    [[nodiscard]] bool Empty() const override;

    std::shared_ptr<BaseImpl> Find(const T& value) const override;
    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override;

    void Insert(const T& value) override;
    void Erase(const T& value) override;

    void Clear() override;

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class  SkipListItImpl : public BaseImpl {
    public:
        SkipListItImpl() = delete;
        SkipListItImpl(const  SkipListItImpl& other);

        std::shared_ptr<BaseImpl> Clone() const override;
        void Increment() override;
        void Decrement() override;
        const T Dereferencing() const override;
        const T* Arrow() const override;
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override;
    };

    std::shared_ptr<BaseImpl> Begin() const override;
    std::shared_ptr<BaseImpl> End() const override;
};

