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
    std::vector<std::unique_ptr<T[]>> buf;
public:
    explicit SimpleAgg(size_t n);
    SimpleAgg(const SimpleAgg&) = delete;
    SimpleAgg& operator= (const SimpleAgg&) = delete;
    void run(T* v, size_t n, int root, MPI_Comm comm);
    void run(std::vector<T> &vec, int root, MPI_Comm comm);
};
