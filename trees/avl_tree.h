#pragma once
#include <initializer_list>
#include <memory>

#include "abstract_tree.h"

template <class T>
class AVLTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    AVLTree();

    template <class InitIterator>
    AVLTree(InitIterator begin, InitIterator end);
    AVLTree(std::initializer_list<T> list);

    AVLTree(const AVLTree &other);
    AVLTree(AVLTree &&other) noexcept;
    AVLTree &operator=(const AVLTree &other);
    AVLTree &operator=(AVLTree &&other) noexcept;

    ~AVLTree() override;

    [[nodiscard]] size_t Size() const override;
    [[nodiscard]] bool Empty() const override;

    std::shared_ptr<BaseImpl> Find(const T &value) const override;
    std::shared_ptr<BaseImpl> LowerBound(const T &value) const override;

    void Insert(const T &value) override;
    void Erase(const T &value) override;

    void Clear() override;

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class AVLTreeItImpl : public BaseImpl {
    public:
        AVLTreeItImpl() = delete;
        AVLTreeItImpl(const AVLTreeItImpl &other);

        std::shared_ptr<BaseImpl> Clone() const override;
        void Increment() override;
        void Decrement() override;
        T Dereferencing() const override;
        const T *Arrow() const override;
        bool IsEqual(std::shared_ptr<BaseImpl> other) const override;
    };

    std::shared_ptr<BaseImpl> Begin() const override;
    std::shared_ptr<BaseImpl> End() const override;
};
