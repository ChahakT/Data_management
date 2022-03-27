#include <utility>

#pragma once

template <class T>
class SmartAgg {
    std::map<int, std::vector<MPI_Comm>> stepCommGroups;
public:
    explicit SmartAgg(std::map<int, std::vector<std::vector<int>>>& stepCommInstructions);
    void run(T* v, size_t n);
    void run(std::vector<T> &vec);
    ~SmartAgg();
};

template <class T>
class SimpleAgg {
    std::vector<std::unique_ptr<T[]>> buf;
public:
    explicit SimpleAgg(size_t n);
    void run(T* v, size_t n, int root, MPI_Comm comm);
    void run(std::vector<T> &vec, int root, MPI_Comm comm);
};
