#pragma once
#include <cassert>
#include <memory>

/**
 * Comparison between two values in tree node.
 * 'std::nullopt' is always larger than any argument.
 * @tparam T Value type
 * @param lhs LHS comparison argument
 * @param rhs RHS comparison argument
 * @return True if LHS < RHS
 */
template <class T>
bool operator<(const std::optional<T>& lhs, const std::optional<T>& rhs) {
    return (lhs && (!rhs || *lhs < *rhs));
}

/**
 * Abstract class that works as BST with chosen tree algorithm inside
 * @tparam T Tree value type
 */
template <class T>
class ITree {
protected:
    ITree() = default;

    /**
     * Abstract class that is needed for iterators implementation
     */
    class ITreeItImpl {
    public:
        /**
         * Virtual destructor is needed for every abstract class
         */
        virtual ~ITreeItImpl() = default;

        /**
         * Implements iterator copying
         * @return Copy of the stored iterator
         */
        virtual std::shared_ptr<ITreeItImpl> Clone() const = 0;

        /**
         * Implements moving the iterator to the next element
         */
        virtual void Increment() = 0;

        /**
         * Implements moving the iterator to the previous element
         */
        virtual void Decrement() = 0;

        /**
         * Implements getting a value from the iterator
         * @return Value that the iterator points to
         */
        virtual const T Dereferencing() const = 0;

        /**
         * Implements getting a pointer to a value from the iterator
         * @return Pointer to a value that the iterator points to
         */
        virtual const T* Arrow() const = 0;

        /**
         * Implements iterator comparison
         * @param other Iterator to compare with
         * @return True if iterators are equal
         */
        virtual bool IsEqual(std::shared_ptr<ITreeItImpl> other) const = 0;
    };

    /**
     * @return Pointer to an iterator implementation to the leftmost element in the tree
     */
    virtual std::shared_ptr<ITreeItImpl> Begin() const = 0;

    /**
     * @return Pointer to an iterator implementation
     * to the next to the rightmost element in the tree
     */
    virtual std::shared_ptr<ITreeItImpl> End() const = 0;

    /**
     * @return Number of elements in the tree
     */
    [[nodiscard]] virtual size_t Size() const = 0;

    /**
     * @return True if there is no elements in the tree
     */
    [[nodiscard]] virtual bool Empty() const = 0;

    /**
     * Method for finding an element in the tree
     * @param value Element to find
     * @return Pointer to an iterator implementation to an element if it is found or
     * pointer to an iterator implementation to the next to the rightmost element in the tree
     */
    virtual std::shared_ptr<ITreeItImpl> Find(const T& value) const = 0;

    /**
     * Method for finding an element which would be the next to the given element in the tree
     * @param value Element to find
     * @return Pointer to an iterator implementation to an element
     * which would be the next to the given element in the tree
     */
    virtual std::shared_ptr<ITreeItImpl> LowerBound(const T& value) const = 0;

    /**
     * Method for inserting an element to the tree
     * @param value Element to insert
     */
    virtual void Insert(const T& value) = 0;

    /**
     * Method for deleting an element from the tree
     * @param value Element to delete
     */
    virtual void Erase(const T& value) = 0;

    /**
     * Method for deleting all the elements from the tree
     */
    virtual void Clear() = 0;

public:
    /**
     * Virtual destructor is needed for every class with virtual methods
     */
    virtual ~ITree() = default;

    /**
     * ITree iterator class.
     * Satisfies C++ LegacyBidirectionalIterator named requirement
     * Uses Pimpl pattern
     */
    class iterator {
    public:
        iterator() = delete;
        explicit iterator(std::shared_ptr<ITreeItImpl> pimpl) : pimpl_(pimpl) {
        }
        iterator(const iterator& other) : pimpl_(other.pimpl_) {
        }
        ~iterator() = default;

        iterator& operator++() {
            pimpl_->Increment();
            return *this;
        }
        iterator operator++(int) {
            iterator cpy(pimpl_->Clone());
            pimpl_->Increment();
            return cpy;
        }
        iterator& operator--() {
            pimpl_->Decrement();
            return *this;
        }
        iterator operator--(int) {
            iterator cpy(pimpl_->Clone());
            pimpl_->Decrement();
            return cpy;
        }

        const T operator*() const {
            return pimpl_->Dereferencing();
        }
        const T* operator->() const {
            return pimpl_->Arrow();
        }

        bool operator==(const iterator& other) const {
            return pimpl_->IsEqual(other.pimpl_);
        }
        bool operator!=(const iterator& other) const {
            return !pimpl_->IsEqual(other.pimpl_);
        }

    private:
        /**
         * pointer to the specific implementation
         */
        std::shared_ptr<ITreeItImpl> pimpl_;
    };

    /**
     * @return Iterator to the leftmost element in the tree
     */
    iterator begin() const {
        return iterator(Begin());
    }

    /**
     * @return Iterator to the next to the rightmost element in the tree
     */
    iterator end() const {
        return iterator(End());
    }

    /**
     * @return Number of elements in the tree
     */
    [[nodiscard]] size_t size() const {
        return Size();
    }

    /**
     * @return True if there is no elements in the tree
     */
    [[nodiscard]] bool empty() const {
        return Empty();
    }

    /**
     * Method for finding an element in the tree
     * @param value Value to find
     * @return Iterator to an element if it is found or
     * iterator to the next to the rightmost element in the tree
     */
    iterator find(const T& value) const {
        return iterator(Find(value));
    }

    /**
     * Method for finding an element which would be the next to the given element in the tree
     * @param value Value to find
     * @return Iterator to an element which would be the next to the given element in the tree
     */
    iterator lower_bound(const T& value) const {
        return iterator(LowerBound(value));
    }

    /**
     * Method for inserting an element to the tree
     * @param value Value to insert
     * It may already be stored in the tree
     */
    void insert(const T& value) {
        Insert(value);
    }

    /**
     * Method for deleting an element from the tree
     * @param value Value to delete
     * It might not be stored in the tree
     */
    void erase(const T& value) {
        Erase(value);
    }

    /**
     * Method for deleting all the elements from the tree
     */
    void clear() {
        Clear();
    }
};
