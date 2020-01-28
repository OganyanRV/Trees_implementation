#include <iostream>

#include "test_framework.cpp"

using std::cout;

int main() {
    std::shared_ptr<ITree<int>> tree = IFactory::Produce<int, CartesianTree<int>>();
    std::cout << tree->empty() << ' ' << tree->size();
}