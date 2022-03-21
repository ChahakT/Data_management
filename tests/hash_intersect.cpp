#include <intersect.h>
#include <assert.h>
#include <bits/stdc++.h>

void check_within(int measured, int expected, int perc = 2) {
    const int min = (1.0 - perc/100.0) * expected;
    const int max = (1.0 + perc/100.0) * expected;
    assert(measured >= min);
    assert(measured <= max);
    std::cerr << "OK! " << measured << " is within: [" << min << ", " << max << "]\n";
}

void test_int() {
    const std::vector<int> v = {100, 50, 200};
    const int sum = std::accumulate(v.begin(), v.end(), 0);
    const int run_fac = 2;
    const int run_cnt = sum * run_fac;;
    IntersectHash hash(v);
    std::vector<int> cnt(v.size(), 0);
    for (int i = 0; i < run_cnt; i++)
        cnt[hash.hash(i)]++;
    for (int i = 0; i < v.size(); i++)
        check_within(cnt[i], v[i] * run_fac);
}
void test_string() {
    const std::vector<int> v = {100, 50, 200};
    const int sum = std::accumulate(v.begin(), v.end(), 0);
    const int run_fac = 2;
    const int run_cnt = sum * run_fac;;
    IntersectHash hash(v);
    std::vector<int> cnt(v.size(), 0);
    for (int i = 0; i < run_cnt; i++)
        cnt[hash.hash(std::to_string(i))]++;
    for (int i = 0; i < v.size(); i++)
        check_within(cnt[i], v[i] * run_fac, 3);
}


int main() {
    test_int();
    test_string();
}
