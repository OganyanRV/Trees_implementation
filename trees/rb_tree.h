#pragma once
#include <initializer_list>
#include <memory>

#include "abstract_tree.h"

template <class T>
class RBTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    RBTree();

    template <class InitIterator>
    RBTree(InitIterator begin, InitIterator end);
    RBTree(std::initializer_list<T> list);

    RBTree(const RBTree& other);
    RBTree(RBTree&& other) noexcept;
    RBTree& operator=(const RBTree& other);
    RBTree& operator=(RBTree&& other) noexcept;

    ~RBTree() override;

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

    class RBTreeItImpl : public BaseImpl {
    public:
        RBTreeItImpl() = delete;
        RBTreeItImpl(const RBTreeItImpl& other);

        std::shared_ptr<BaseImpl> Clone() const override;
        void Increment() override;
        void Decrement() override;
        T Dereferencing() const override;
        const T* Arrow() const override;
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override;
    };

    std::shared_ptr<BaseImpl> Begin() const override;
    std::shared_ptr<BaseImpl> End() const override;
};
