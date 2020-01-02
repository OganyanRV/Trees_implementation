#include <initializer_list>
#include <memory>

#include "abstract_tree.cpp"

template <class T>
class SplayTree : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    SplayTree();

    template <class InitIterator>
    SplayTree(InitIterator begin, InitIterator end);
    SplayTree(std::initializer_list<T> list);

    SplayTree(const SplayTree& other);
    SplayTree(SplayTree&& other) noexcept;
    SplayTree& operator=(const SplayTree& other);
    SplayTree& operator=(SplayTree&& other) noexcept;

    ~SplayTree() override;

    [[nodiscard]] size_t size() const override;
    [[nodiscard]] bool empty() const override;

    std::shared_ptr<BaseImpl> find_impl(const T& value) const override;
    std::shared_ptr<BaseImpl> lower_bound_impl(const T& value) const override;

    void insert(const T& value) override;
    void erase(const T& value) override;

    void clear() override;

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class SplayTreeItImpl : public BaseImpl {
    public:
        SplayTreeItImpl() = delete;
        SplayTreeItImpl(const SplayTreeItImpl& other);

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
