#pragma once
#include <utility>

template <class T>
class SmartAgg {
    std::map<int, std::vector<MPI_Comm>> stepCommGroups;
public:
    explicit SmartAgg(std::map<int, std::vector<std::vector<int>>>& stepCommInstructions);
    void run(T* v, size_t n);
    void run(std::vector<T> &vec);
    ~SmartAgg();
    SmartAgg(const SmartAgg&) = delete;
    SmartAgg& operator= (const SmartAgg&) = delete;
};

template <class T>
class SimpleAgg {
//    std::vector<std::unique_ptr<T[]>> buf;
public:
    explicit SimpleAgg(size_t n);
    SimpleAgg(const SimpleAgg&) = delete;
    SimpleAgg& operator= (const SimpleAgg&) = delete;
    void run(T* v, size_t n, int root, MPI_Comm comm);
    void run(std::vector<T> &vec, int root, MPI_Comm comm);
};

template <class T>
class SmartAggV2 {
    const double beta;
    SmartAggV2(const SmartAggV2&) = delete;
    SmartAggV2& operator= (const SmartAggV2&) = delete;
public:
    double get_fraction() const {
        return 2.0/(3 * beta + 2);
    }
    explicit SmartAggV2(double b): beta(b) { ; }
    void run(T* v, size_t n, int root, MPI_Comm comm);
    void run(std::vector<T> &vec, int root = 0) {
        run(vec.data(), vec.size(), root, MPI_COMM_WORLD);
    }
};
