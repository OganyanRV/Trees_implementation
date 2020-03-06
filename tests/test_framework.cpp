#include <algorithm>
#include <functional>
#include <iostream>
#include <string>
#include <map>

#include "full_test_set.h"

using std::cout;

class TestFramework {
public:
    TestFramework() {
#ifdef RELEASE_BUILD
        cout << "Test framework started at release build\n\n";
#else
        cout << "Test framework started at debug build\n\n";
#endif
        // All types of trees are listed below.
        types_.emplace("AVL tree", ImplType::kAVL);
        types_.emplace("Cartesian tree", ImplType::kCartesian);
        types_.emplace("Red-Black tree", ImplType::kRB);
        types_.emplace("Skip list", ImplType::kSkipList);
        types_.emplace("Splay tree", ImplType::kSplay);

        /* All tests are listed below.
         * We'll use '!' for good tests that we are sure of,
         * '%' for useless and demonstrative tests.
         * You can also use your own symbol for your tests.
         */
        tests_.emplace("%_simple_test", SomeTest);
        tests_.emplace("!_emptiness_test", EmptinessTest);
        tests_.emplace("!_empty_iterators_test", EmptyIteratorsTest);
        tests_.emplace("!_empty_copying_test", EmptyCopyingTest);
        tests_.emplace("!_few_elements_test", FewElementsTest);
        tests_.emplace("!_few_elements_iterator_test", FewElementsIteratorTest);
        tests_.emplace("!_few_elements_copying_test", FewElementsCopyingTest);
        tests_.emplace("!_strange_test", StrangeTest);
        tests_.emplace("!_strange_copy_test", StrangeCopyTest);
        tests_.emplace("!_find_and_lower_bound_test", FindAndLBTest);
        tests_.emplace("!_insert_and_erase_test", InsertAndEraseTest);
    }

    template <class TreePredicate>
    void RunTest(const std::string &test_name, TreePredicate tree_predicate) {
        auto it = tests_.find(test_name);
        if (it == tests_.end()) {
            return;
        }
        for (auto &type : types_) {
            if (tree_predicate(type.first)) {
                cout << "Running " << it->first << " on " << type.first << ": ";
                try {
                    it->second(type.second);
                    cout << "Success!\n\n";
                } catch (std::exception &ex) {
                    cout << "Failure: " << ex.what() << "\n\n";
                } catch (...) {
                    cout << "Failure: Unknown exception. PLEASE THROW \"std::exception\" BASED "
                            "EXCEPTIONS\n\n";
                }
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
                cout << "Running " << test.first << " on some trees:\n";
                for (auto &tree : trees_for_tests) {
                    cout << tree.first << ": ";
                    try {
                        test.second(tree.second);
                        cout << "Success!\n";
                    } catch (std::exception &ex) {
                        cout << "Failure: " << ex.what() << "\n";
                    } catch (...) {
                        cout << "Failure: Unknown exception. PLEASE THROW \"std::exception\" BASED "
                                "EXCEPTIONS\n";
                    }
                }
                cout << "Test passed!\n\n";
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

    [[nodiscard]] std::vector<std::string> ShowAllTests() const {
        std::vector<std::string> result;
        for (const auto &test : tests_) {
            result.emplace_back(test.first);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

private:
    std::map<std::string, ImplType> types_;
    std::map<std::string, std::function<void(ImplType)>> tests_;

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
