#pragma once

class SmartAgg {
public:
    std::vector<MPI_Comm> get_comm(std::vector<std::vector<int>> &comm_groups);
    template <class T>
    void run(T* v, const int n, std::vector<std::vector<int>> &comm_groups);
    template <class T>
    void run(std::vector<T> &vec, std::vector<std::vector<int>> &comm_groups);
};

class SimpleAgg {
public:
    template <class T>
    void run(T* v, const int n, const int root, MPI_Comm comm);
    template <class T>
    void run(std::vector<T> &vec, const int root, MPI_Comm comm);
};
