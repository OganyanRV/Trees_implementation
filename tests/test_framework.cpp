#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../trees/abstract_tree.cpp"

template <class T>
class IFactory {
public:
    virtual std::unique_ptr<ITree<T>> Produce() = 0;
    virtual ~IFactory() = default;
};

template <class TreeType, class T>
class TreeFactory : public IFactory<T> {
public:
    std::unique_ptr<ITree<T>> Produce() override {
        ITree<T> *ptr = new TreeType<T>();
        return std::make_unique<TreeType<T>>();
    }
};

class TestRegistry {
public:
    TestRegistry(const TestRegistry &) = delete;
    TestRegistry(TestRegistry &&) = delete;
    TestRegistry *operator=(const TestRegistry &) = delete;
    TestRegistry *operator=(TestRegistry &&) = delete;

    static TestRegistry &Instance() {
        static TestRegistry test_registry;
        return test_registry;
    }

    template <class TestClass>
    void RegisterClass(const std::string &class_name) {
        if (factories_.find(class_name) != factories_.end()) {
            throw std::runtime_error("This factory is already exists");
        }
        factories_[class_name] = std::make_shared<TestFactory<TestClass>>();
    }

    std::unique_ptr<ITest> CreateTest(const std::string &class_name) {
        auto it = factories_.find(class_name);
        if (it == factories_.end()) {
            throw std::out_of_range("Factory was not registered");
        }
        return (*it).second->Produce();
    }

    void RunTest(const std::string &class_name) {
        std::shared_ptr<ITest> test = CreateTest(class_name);
        try {
            test->SetUp();
            test->Run();
            test->TearDown();
        } catch (std::exception &ex) {
            std::exception_ptr exception_ptr = std::current_exception();
            test->TearDown();
            std::rethrow_exception(exception_ptr);
        }
    }

    template <class Predicate>
    void RunTests(Predicate callback) {
        std::vector<std::string> to_be_tested;
        for (const auto &factory : factories_) {
            if (callback(factory.first)) {
                to_be_tested.emplace_back(factory.first);
            }
        }
        std::sort(to_be_tested.begin(), to_be_tested.end());
        for (const std::string &test : to_be_tested) {
            RunTest(test);
        }
    }

    template <class Predicate>
    std::vector<std::string> ShowTests(Predicate callback) const {
        std::vector<std::string> result;
        for (const auto &factory : factories_) {
            if (callback(factory.first)) {
                result.emplace_back(factory.first);
            }
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    std::vector<std::string> ShowAllTests() const {
        return ShowTests([](const std::string &class_name) { return true; });
    }

    void Clear() {
        factories_.clear();
    }

private:
    TestRegistry() = default;

    std::unordered_map<std::string, std::shared_ptr<IFactory>> factories_;
};
