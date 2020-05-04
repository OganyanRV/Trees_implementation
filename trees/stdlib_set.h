#pragma once
#include <exception>
#include <initializer_list>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <set>

template <class T>
class ITree;

template <class T>
class StdlibSet : public ITree<T> {
private:
    typedef typename ITree<T>::ITreeItImpl BaseImpl;

public:
    StdlibSet() : set_() {
    }

    template <class InitIterator>
    StdlibSet(InitIterator begin, InitIterator end) : set_(begin, end) {
    }

    StdlibSet(std::initializer_list<T> list) : set_(list) {
    }

    StdlibSet(const StdlibSet& other) : set_(other.set_) {
    }

    StdlibSet(StdlibSet&& other) noexcept : set_(other.set_) {
    }

    StdlibSet(std::shared_ptr<ITree<T>> other)
        : StdlibSet(*dynamic_cast<StdlibSet<T>*>(other.get())) {
    }

    StdlibSet& operator=(const StdlibSet& other) {
        set_ = other.set_;
        return *this;
    }

    StdlibSet& operator=(StdlibSet&& other) noexcept {
        set_ = std::move(other.set_);
        return *this;
    }

    [[nodiscard]] size_t Size() const override {
        return set_.size();
    }

    [[nodiscard]] bool Empty() const override {
        return set_.empty();
    }

    std::shared_ptr<BaseImpl> Find(const T& value) const override {
        return std::make_shared<StdlibSetItImpl>(set_.find(value));
    }

    std::shared_ptr<BaseImpl> LowerBound(const T& value) const override {
        return std::make_shared<StdlibSetItImpl>(set_.lower_bound(value));
    }

    void Insert(const T& value) override {
        set_.insert(value);
    }

    void Erase(const T& value) override {
        set_.erase(value);
    }

    void Clear() override {
        set_.clear();
    }

private:
    std::set<T> set_;

    /* ---------------------------------------------------
     * --------------ITERATOR IMPLEMENTATION--------------
     * ---------------------------------------------------
     */

    class StdlibSetItImpl : public BaseImpl {
    private:
        typename std::set<T>::iterator it_;

    public:
        StdlibSetItImpl() = delete;

        StdlibSetItImpl(const StdlibSetItImpl& other) : it_(other.it_) {
        }

        StdlibSetItImpl(typename std::set<T>::iterator other) : it_(other) {
        }

        std::shared_ptr<BaseImpl> Clone() const override {
            return std::make_shared<StdlibSetItImpl>(*this);
        }

        void Increment() override {
            ++it_;
        }

        void Decrement() override {
            --it_;
        }

        const T Dereferencing() const override {
            return *it_;
        }

        const T* Arrow() const override {
            return &(*it_);
        }

        bool IsEqual(std::shared_ptr<BaseImpl> other) const override {
            auto casted = std::dynamic_pointer_cast<StdlibSetItImpl>(other);
            if (!casted) {
                return false;
            }
            return it_ == casted->it_;
        }
    };

    std::shared_ptr<BaseImpl> Begin() const override {
        return std::make_shared<StdlibSetItImpl>(set_.begin());
    }

    std::shared_ptr<BaseImpl> End() const override {
        return std::make_shared<StdlibSetItImpl>(set_.end());
    }
};