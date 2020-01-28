#pragma once
#include <algorithm>
#include <functional>
#include <string>
#include <unordered_map>

#include "full_test_set.h"

class TestFramework {
public:
    TestFramework() {
        // All types of trees are listed below
        types_.emplace("AVL tree", ImplType::kAVL);
        types_.emplace("Cartesian tree", ImplType::kCartesian);
        types_.emplace("Red-Black tree", ImplType::kRB);
        types_.emplace("Splay tree", ImplType::kSplay);

        // All tests are listed below
        tests_.emplace("%_simple_test", SomeTest);
    }

    template <class TreePredicate>
    void RunTest(const std::string &test_name, TreePredicate tree_predicate) {
        auto it = tests_.find(test_name);
        if (it == tests_.end()) {
            return;
        }
        for (auto &type : types_) {
            if (tree_predicate(type.first)) {
                it->second(type.first, type.second);
            }
        }
    }

    void RunTestForAll(const std::string &test_name) {
        RunTest(test_name, Every());
    }

    template <class TestPredicate, class TreePredicate>
    void RunTests(TestPredicate test_predicate, TreePredicate tree_predicate) {
        std::unordered_map<std::string, ImplType> trees_for_tests;
        for (auto &type : types_) {
            if (tree_predicate(type.first)) {
                trees_for_tests.insert(type);
            }
        }
        for (auto &test : tests_) {
            if (test_predicate(test.first)) {
                for (auto &tree : trees_for_tests) {
                    test.second(tree.first, tree.second);
                }
            }
        }
    }

    template <class TestPredicate>
    void RunTestsForAll(TestPredicate test_predicate) {
        RunTests(test_predicate, Every());
    }

    template <class TreePredicate>
    void RunAll(TreePredicate tree_predicate) {
        RunTests(Every(), tree_predicate);
    }

    void RunAllForAll() {
        RunTests(Every(), Every());
    }

    template <class TestPredicate>
    std::vector<std::string> ShowTests(TestPredicate test_predicate) const {
        std::vector<std::string> result;
        for (const auto &test : tests_) {
            if (test_predicate(test.first)) {
                result.emplace_back(test.first);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<std::string> ShowAllTests() const {
        std::vector<std::string> result;
        for (const auto &test : tests_) {
            result.emplace_back(test.first);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

private:
    std::unordered_map<std::string, ImplType> types_;
    std::unordered_map<std::string, std::function<void(const std::string &, ImplType)>> tests_;

    class Every {
    public:
        bool operator()(const std::string &arg) {
            return true;
        }
    };
};

class Substr {
public:
    explicit Substr(const char *str) : str_(str) {
    }

    bool operator()(const std::string &arg) {
        return arg.find(str_) != std::string::npos;
    }

private:
    std::string str_;
};

class FullMatch {
public:
    explicit FullMatch(const char *str) : str_(str) {
    }

    bool operator()(const std::string &arg) {
        return arg == str_;
    }

private:
    std::string str_;
};
