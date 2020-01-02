#include <algorithm>
#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class ITest {
public:
    virtual void SetUp() = 0;
    virtual void TearDown() = 0;
    virtual void Run() = 0;
    virtual ~ITest() = default;
};

class IFactory {
public:
    virtual std::unique_ptr<ITest> Produce() = 0;
    virtual ~IFactory() = default;
};

template <class TestClass>
class TestFactory : public IFactory {
public:
    std::unique_ptr<ITest> Produce() override {
        return std::make_unique<TestClass>();
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
