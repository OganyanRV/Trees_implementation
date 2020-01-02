#include <iostream>

#include "test_framework.cpp"

#include "../trees/cartesian_tree.cpp"

using std::cout;

int main() {
    std::shared_ptr<ITree<int>> tree = std::make_shared<CartesianTree<int>>();
    for (int i = 0; i < 10; ++i) {
        tree->insert(i);
        for (int val : *tree) {
            cout << val << ' ';
        }
        cout << '\n';
    }

    cout << '\n';

    for (int i = 0; i < 10; ++i) {
        tree->erase(i);
        for (int val : *tree) {
            cout << val << ' ';
        }
        cout << '\n';
    }
}