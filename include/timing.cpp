#include <iostream>
#include <chrono>
#include <vector>
#include <numeric>

class Stats {
    std::vector<uint64_t> stats;
    std::string name;
public:
    Stats(): name("unnnamed"){}
    Stats(const std::string name_): name(std::move(name_)) {}
    void add(uint64_t ns) {
#ifndef DISBALE_BENCH
        stats.push_back(ns);
#endif
    }
    ~Stats() {
#ifndef DISBALE_BENCH
        std::cout << name << ", ";
        if (stats.size() == 0) {
            std::cout << "X\n";
        } else if (stats.size() == 1) {
            std::cout << stats.front() << "\n";
        } else {
            const auto sum = std::accumulate(stats.begin(), stats.end(), 0ULL);
            std::cout <<(int) ( ((double) sum) / stats.size() ) << "\n";
        }
#endif
    }
};

struct Clocker {
    const std::chrono::time_point<std::chrono::high_resolution_clock> start;
    Stats& stats;
    Clocker(Stats& stat): start(std::chrono::high_resolution_clock::now()), stats(stat){}
    uint64_t get_ns() const {
#ifndef DISBALE_BENCH
        using namespace std::chrono;
        return duration_cast<nanoseconds>(high_resolution_clock::now() - start).count();
#endif
        return 0;
    }
    ~Clocker() {
#ifndef DISBALE_BENCH
        stats.add(get_ns());
#endif
    }
};