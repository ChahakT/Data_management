#pragma once
#include <utility>
#include <vector>
#include <functional> // std::hash
#include <cassert>
#include <stdexcept> // std::logic_error
#include <iostream>

typedef struct ompi_communicator_t *MPI_Comm; // typedef here to avoid incl mpi.h
class SimpleIntersect {
public:
    template <class T>
    std::vector<T> run(T *vR, T *vS, const int nR, const int nS, MPI_Comm comm);
    template <class T>
    std::vector<T> run(std::vector <T> &vR, std::vector <T> &vS, MPI_Comm comm);
};

class SmartIntersect {
public:
    std::vector<MPI_Comm> get_comm(std::vector<std::vector<int>> &comm_groups);
    template <class T>
    void run(T *vR, T *vS, const int nR, const int nS,
       std::vector<std::vector<int>> &comm_groups, std::vector<std::vector<int>> &dist);
    template <class T>
    void run(std::vector<T> &vR, std::vector<T> &vS,
       std::vector<std::vector<int>> &comm_groups, std::vector<std::vector<int>> &dist);
};


struct IntersectHash {
public:
    explicit IntersectHash(std::vector<int>  v_): v(std::move(v_)) {
        if (v.empty())
            throw std::logic_error("cannot create IntersectHash with 0 vals");
        for (size_t i = 1; i < v.size(); i++) v[i] += v[i-1];
    }

    template <class T>
    int hash(const T& x) const {
        const auto hash_v = std::hash<T>()(x) % v.back();
        const auto it = std::lower_bound(v.begin(), v.end(), hash_v);
        assert(it != v.end());
        return it - v.begin();
    }

    int hash(int x, int flag) const {
        const auto hash_v = x % v.back();
        const auto it = std::upper_bound(v.begin(), v.end(), hash_v);
        assert(it != v.end());
        return it - v.begin();
    }

private:
    std::vector<int> v;
};

const uint seed = 15;
const int nS = 4, nR = 3;
