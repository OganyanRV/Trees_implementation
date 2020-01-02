#include <memory>

template <class T>
class ITree {
protected:
    class ITreeItImpl {
    public:
        virtual ~ITreeItImpl() = default;

        virtual std::shared_ptr<ITreeItImpl> Clone() const = 0;
        virtual void Increment() = 0;
        virtual void Decrement() = 0;
        virtual T Dereferencing() const = 0;
        virtual const T* Arrow() const = 0;
        virtual bool IsEqual(std::shared_ptr<ITreeItImpl> other) const = 0;
    };

    virtual std::shared_ptr<ITreeItImpl> Begin() const = 0;
    virtual std::shared_ptr<ITreeItImpl> End() const = 0;

    virtual std::shared_ptr<ITreeItImpl> find_impl(const T& value) const = 0;
    virtual std::shared_ptr<ITreeItImpl> lower_bound_impl(const T& value) const = 0;

public:
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

        T operator*() const {
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
        std::shared_ptr<ITreeItImpl> pimpl_;
    };

    iterator begin() const {
        return iterator(Begin());
    }
    iterator end() const {
        return iterator(End());
    }

    virtual ~ITree() = default;

    [[nodiscard]] virtual size_t size() const = 0;
    [[nodiscard]] virtual bool empty() const = 0;
    iterator find(const T& value) const {
        return iterator(find_impl(value));
    }
    iterator lower_bound(const T& value) const {
        return iterator(lower_bound_impl(value));
    }

    virtual void insert(const T& value) = 0;
    virtual void erase(const T& value) = 0;

    virtual void clear() = 0;
};
