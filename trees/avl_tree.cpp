#include <initializer_list>
#include <memory>

#include "abstract_tree.cpp"

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

    [[nodiscard]] size_t size() const override;
    [[nodiscard]] bool empty() const override;

    std::shared_ptr<BaseImpl> find_impl(const T &value) const override;
    std::shared_ptr<BaseImpl> lower_bound_impl(const T &value) const override;

    void insert(const T &value) override;
    void erase(const T &value) override;

    void clear() override;

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
