#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

#include "../trees/cartesian_tree.h"

void fail(const char* message) {
    std::cerr << "Fail:\n";
    std::cerr << message << "\n";
    std::cout << "-1 bad output\n";  // to get WA
    exit(0);
}

namespace internal_tests {

/* check if class provides correct interface
 * in terms of constness */
void check_constness() {
    std::cerr << "check constness... ";
    const CartesianTree<int> s{-4, 5, 3, 0, 7};
    if (s.find(3) == s.end())
        fail("3 not found, incorrect find");
    if (*s.lower_bound(2) != 3 || s.lower_bound(8) != s.end() || *s.lower_bound(-2) != 0)
        fail("incorrect lower_bound");
    if (s.empty())
        fail("incorrect empty");
    if (s.size() != 5)
        fail("incorrect size");
    auto first = s.begin();
    CartesianTree<int>::iterator last = s.end();
    if (*first != -4 || *(--last) != 7)
        fail("incorrect begin or end");
    std::cerr << "ok!\n";
}

/* check if class correctly implements
 * copy constructor and = operator */
void check_copy_correctness() {
    std::cerr << "check copy... ";
    int elems[] = {3, 3, -1, 6, 0, 0, 17, -5, 4, 2};
    CartesianTree<int> s1(elems, elems + 10);
    std::set<int> set_elems(elems, elems + 10);
    CartesianTree<int> s2 = s1;
    s2.insert(5);
    s2.insert(18);
    s2.insert(-2);
    auto s1_it = s1.begin(), s2_it = s2.begin();
    auto s_it = set_elems.begin();
    while (s1_it != s1.end() || s2_it != s2.end() || s_it != set_elems.end()) {
        if (*s2_it == 5 || *s2_it == 18 || *s2_it == -2) {
            ++s2_it;
            continue;
        }
        if (*s1_it != *s2_it || *s1_it != *s_it || *s2_it != *s_it)
            fail("fail after copy construction and insertions");
        ++s1_it, ++s2_it, ++s_it;
    }

    s1 = s2;
    s2.insert(19);
    auto cur_end = s2.end();
    cur_end--;
    s1_it = s1.begin(), s2_it = s2.begin();
    while (s1_it != s1.end() || s2_it != cur_end) {
        if (*s1_it != *s2_it)
            fail("wrong = operator");
        ++s1_it, ++s2_it;
    }

    s1 = s1 = s2;
    s1_it = s1.begin(), s2_it = s2.begin();
    while (s1_it != s1.end() || s2_it != s2.end()) {
        if (*s1_it != *s2_it)
            fail("wrong = operator");
        ++s1_it, ++s2_it;
    }
    std::cerr << "ok!\n";
}

/* check if class correctly handles empty set */
void check_empty() {
    std::cerr << "check empty set handling... ";
    CartesianTree<std::string> s;
    if (!s.empty())
        fail("wrong empty");
    auto begin = s.begin(), end = s.end();
    if (begin != end)
        fail("incorrect handling empty set");
    std::string elem("abacaba");
    s.insert(elem);
    if (*s.lower_bound("aac") != elem)
        fail("wrong lower_bound");
    CartesianTree<std::string> empty;
    CartesianTree<std::string> s2{"opa"};
    s2 = empty;
    if (s2.size())
        fail("incorrect size");
    CartesianTree<std::string> s3(s2);
    if (!s3.empty())
        fail("incorrect empty");
    std::cerr << "ok!\n";
}

/* check if class correctly works with iterators */
void check_iterators() {
    std::cerr << "check iterators... ";
    CartesianTree<std::pair<int, int> > s{{-3, 5}, {5, 5}, {-4, 1}, {-4, 4}, {0, 1}, {3, 0}};
    if (s.begin()->second != 1 || (++s.begin())->first != -4)
        fail("wrong begin()");
    CartesianTree<std::pair<int, int> >::iterator cur = s.end();
    CartesianTree<int> small{1};
    ITree<int>::iterator it;
    it = small.begin();
    if (*it != 1)
        fail("incorrect begin");
    auto begin = s.begin();
    begin++;
    cur--;
    if (begin == cur)
        fail("wrong == for iterators");
    while (begin != cur) {
        ++begin;
        --cur;
    }
    if (*begin != *cur)
        fail("wrong Iterators");
    std::cerr << "ok!\n";
}

struct StrangeInt {
    int x;
    static int counter;
    StrangeInt() {
        ++counter;
    }
    StrangeInt(int x) : x(x) {
        ++counter;
    }
    StrangeInt(const StrangeInt& rs) : x(rs.x) {
        ++counter;
    }
    bool operator<(const StrangeInt& rs) const {
        return x < rs.x;
    }

    static void init() {
        counter = 0;
    }

    ~StrangeInt() {
        --counter;
    }

    friend std::ostream& operator<<(std::ostream& out, const StrangeInt& x) {
        out << x.x;
        return out;
    }
};
int StrangeInt::counter;

/* check if class uses only < for elements comparing */
void check_operator_less() {
    std::cerr << "check operator <... ";

    CartesianTree<StrangeInt> s{-5, -3, -6, 13, 7, 1000, 963};
    auto it = s.lower_bound(999);
    ++it;
    if (it != s.end())
        fail("wrong ++ for iterator");
    std::cerr << "ok!\n";
}

/* check if class correctly implements destructor */
void check_destructor() {
    std::cerr << "check destructor... ";
    StrangeInt::init();
    {
        CartesianTree<StrangeInt> s{5, 4, 3, 2, 1, 0};
        if (s.size() != 6)
            fail("wrong size");
    }
    if (StrangeInt::counter)
        fail("wrong destructor (or constructors)");
    {
        CartesianTree<StrangeInt> s{-3, 3, -2, 2, -1, 1};
        CartesianTree<StrangeInt> s1(s);
        s1.insert(0);
        CartesianTree<StrangeInt> s2(s1);
        if (s1.find(0) == s1.end())
            fail("wrong find");
        s1 = s;
        if (s1.find(0) != s1.end())
            fail("wrong find");
    }
    if (StrangeInt::counter)
        fail("wrong destructor (or constructors)");
    std::cerr << "ok!\n";
}

/* check erase for correctness */
void check_erase() {
    std::cerr << "check erase... ";
    CartesianTree<std::string> s{"abacaba", "hello", "p"};
    s.erase("miss");
    s.erase("hello");
    if (s.size() != 2)
        fail("Bad size");
    s.erase("p");
    if (*s.begin() != "abacaba")
        fail("Bad begin");
    s.erase("1");
    s.erase("abacaba");
    s.erase("012");
    if (!s.empty())
        fail("Bad empty");
    std::cerr << "ok!\n";
}

void run_all() {
    check_constness();
    check_empty();
    check_operator_less();
    check_iterators();
    check_erase();
    check_copy_correctness();
    check_destructor();
}
}  // namespace internal_tests

int main() {
    internal_tests::run_all();
    return 0;
}