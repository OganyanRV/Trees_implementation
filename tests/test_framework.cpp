#define CATCH_CONFIG_MAIN
#include "../catch/catch.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "full_test_set.h"

using std::cout;

/**
 * Framework for testing ITree based structures
 */
class TestFramework {
public:
    TestFramework() {
#ifdef RELEASE_BUILD
        cout << "Test framework started at release build\n\n";
#else
        cout << "Test framework started at debug build\n\n";
#endif
        /// All types of trees are listed below.
        types_.emplace("AVL tree", ImplType::kAVL);
        types_.emplace("Cartesian tree", ImplType::kCartesian);
        types_.emplace("Red-Black tree", ImplType::kRB);
        types_.emplace("Skip list", ImplType::kSkipList);
        types_.emplace("Splay tree", ImplType::kSplay);

        /**
         * All tests are listed below.
         * We use '!' for useful and essential tests,
         * '%' for simple and demonstrative tests.
         */
        tests_.emplace("%_simple_test", SomeTest);
        tests_.emplace("%_rb_only_black_height_test", RBBlackHeightTest);
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

    /**
     * This function runs given test with tree types satisfying @param tree_predicate
     * @tparam TreePredicate Tree predicate functor type
     * @param test_name Name of the test to run
     * @param tree_predicate Functor for choosing trees
     */
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

    /**
     * This function generalizes 'RunTest()' to use all available trees
     * @param test_name Name of the test to run
     */
    void RunTestForAll(const std::string &test_name) {
        RunTest(test_name, Every());
    }

    /**
     * This function runs tests satisfying @param test_predicate
     * with tree types satisfying @param tree_predicate
     * @tparam TestPredicate Test predicate functor type
     * @tparam TreePredicate Tree predicate functor type
     * @param test_predicate Functor for choosing tests
     * @param tree_predicate Functor for choosing trees
     */
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

    /**
     * This function generalizes 'RunTests()' to use all available trees
     * @tparam TestPredicate Test predicate functor type
     * @param test_predicate Functor for choosing tests
     */
    template <class TestPredicate>
    void RunTestsForAll(TestPredicate test_predicate) {
        RunTests(test_predicate, Every());
    }

    /**
     * This function generalizes 'RunTests()' to check all available tests
     * @tparam TreePredicate Tree predicate functor type
     * @param tree_predicate Functor for choosing trees
     */
    template <class TreePredicate>
    void RunAll(TreePredicate tree_predicate) {
        RunTests(Every(), tree_predicate);
    }

    /**
     * This function generalizes 'RunTests()' to check all available tests
     * on all available trees
     */
    void RunAllForAll() {
        RunTests(Every(), Every());
    }

    /**
     * This function returns a 'std::vector' of tests satisfying @param test_predicate
     * @tparam TestPredicate Test predicate functor type
     * @param test_predicate Functor for choosing tests
     * @return 'std::vector' of test names satisfying @param test_predicate
     */
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

    /**
     * This function returns all available tests
     * @return 'std::vector' of all available test names
     */
    [[nodiscard]] std::vector<std::string> ShowAllTests() const {
        std::vector<std::string> result;
        for (const auto &test : tests_) {
            result.emplace_back(test.first);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

private:
    /// Types of trees (name + type)
    std::map<std::string, ImplType> types_;
    /// Tests (name + function)
    std::map<std::string, std::function<void(ImplType)>> tests_;

    /// Functor for every object in a list
    class Every {
    public:
        /**
         * Functor, which always returns true
         * @param arg String to compare with
         * @return True
         */
        bool operator()(const std::string &arg) {
            return true;
        }
    };
};

/// Functor for objects, which has a substring in their name matching to @param str_
class Substr {
public:
    /**
     * Constructor for the functor
     * @param str Substring, which we are going to find
     */
    explicit Substr(const char *str) : str_(str) {
    }

    /**
     * Functor
     * @param arg String to compare with
     * @return True if 'str_' is contained in @param arg
     */
    bool operator()(const std::string &arg) {
        return arg.find(str_) != std::string::npos;
    }

private:
    std::string str_;
};

/// Functor for objects, which name matches to the given string
class FullMatch {
public:
    /**
     * Constructor for the functor
     * @param str String, which we are going to compare with
     */
    explicit FullMatch(const char *str) : str_(str) {
    }

    /**
     * Functor
     * @param arg String to compare with
     * @return True if 'str_' is matched with @param arg
     */
    bool operator()(const std::string &arg) {
        return arg == str_;
    }

private:
    std::string str_;
};
