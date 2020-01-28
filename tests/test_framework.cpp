#pragma once
#include <algorithm>
#include <exception>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../trees/tree_factory.h"

class TestFramework{
public:
private:
    std::unordered_map<std::string, std::shared_ptr<IFactory>> factories;
std::unordered_map<std::string,std::function<void(std::shared_ptr<IFactory)
};