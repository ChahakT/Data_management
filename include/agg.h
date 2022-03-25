#pragma once

class SmartAgg {
    bool showLog;
public:
    SmartAgg() : showLog(true) {};
    explicit SmartAgg(bool showLog_) : showLog(showLog_) {}
    std::vector<MPI_Comm> get_comm(std::vector<std::vector<int>> &comm_groups);
    template <class T>
    void run(T* v, const int n, std::vector<std::vector<int>> &comm_groups);
    template <class T>
    void run(std::vector<T> &vec, std::vector<std::vector<int>> &comm_groups);
};

template <class T>
class SimpleAgg {
    std::vector<T*> buf;
public:
    explicit SimpleAgg(size_t n);
    void run(T* v, size_t n, int root, MPI_Comm comm);
    void run(std::vector<T> &vec, int root, MPI_Comm comm);
    ~SimpleAgg();
};
