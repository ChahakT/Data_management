#include <iostream>
#include <chrono>
#include <utility>
#include <vector>
#include <numeric>

class Stats {
    std::vector<uint64_t> stats;
    std::string name;
    bool showOutput;

    static uint64_t median(std::vector<uint64_t> &vec) {
        size_t n = vec.size() / 2;
        std::nth_element(vec.begin(), vec.begin() + n, vec.end());
        if (vec.size() % 2 == 0) {
            size_t n_1 = std::max(n - 1, (size_t) 0);
            std::nth_element(vec.begin(), vec.begin() + n_1, vec.end());
            return (vec[n] + vec[n_1]) / 2;
        }
        return vec[n];
    }

public:
    Stats() : name("unnamed"), showOutput(false) {}

    explicit Stats(std::string name_, bool showOutput_) : name(std::move(name_)), showOutput(showOutput_) {}

    void add(uint64_t ns) {
#ifndef DISBALE_BENCH
        stats.push_back(ns);
#endif
    }

    ~Stats() {
#ifndef DISBALE_BENCH
        if (!showOutput) return;
        std::cout << name << ", ";
        if (stats.empty()) {
            std::cout << "X\n";
        } else if (stats.size() == 1) {
            std::cout << stats.front() << "\n";
        } else {
            const long double vecMedian = (double) median(stats) / 1e3;
            std::cout << vecMedian << "\n";
        }
#endif
    }
};

struct ClockTracker {
    const std::chrono::time_point<std::chrono::high_resolution_clock> start;
    Stats &stats;

    explicit ClockTracker(Stats &stat) : start(std::chrono::high_resolution_clock::now()), stats(stat) {}

    [[nodiscard]] uint64_t get_ns() const {
#ifndef DISBALE_BENCH
        using namespace std::chrono;
        return duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
#endif
        return 0;
    }

    ~ClockTracker() {
#ifndef DISBALE_BENCH
        stats.add(get_ns());
#endif
    }
};