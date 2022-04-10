#pragma once

#include <utility>
#include <vector>
#include <functional> // std::hash
#include <cassert>
#include <unordered_map>
#include <stdexcept> // std::logic_error
#include <iostream>

typedef struct ompi_communicator_t *MPI_Comm; // typedef here to avoid incl mpi.h
class SimpleIntersect {
public:
    template<class T>
    std::vector<T> run(T *vR, T *vS, int nR, int nS, MPI_Comm comm);
    template<class T>
    std::vector<T> run(std::vector<T> &vR, std::vector<T> &vS, MPI_Comm comm);
};


struct IntersectHash {
public:
    explicit IntersectHash(std::vector<int> v_) : v(std::move(v_)) {
        if (v.empty())
            throw std::logic_error("cannot create IntersectHash with 0 vals");
        for (size_t i = 1; i < v.size(); i++) v[i] += v[i - 1];
    }

    template<class T>
    int hash(const T &x) const {
        const auto hash_v = std::hash<T>()(x) % v.back();
        const auto it = std::lower_bound(v.begin(), v.end(), hash_v);
        assert(it != v.end());
        return it - v.begin();
    }

    [[nodiscard]] long hash(int x, int flag) const {
        const auto hash_v = x % v.back();
        const auto it = std::upper_bound(v.begin(), v.end(), hash_v);
        assert(it != v.end());
        return it - v.begin();
    }

private:
    std::vector<int> v;
};


template<class T>
class SmartIntersect {
public:
    explicit SmartIntersect(std::vector<std::vector<int>> &comm_groups, std::vector<std::vector<int>> &group_dist);
    void run(T *vR, T *vS, int nR, int nS);
    void run(std::vector<T> &vR, std::vector<T> &vS);
private:
    std::vector<std::vector<int>> commGroupRanks;
    std::vector<MPI_Comm> commList;
    std::vector<IntersectHash> hashObjList;

    std::unordered_map<int, std::vector<T>> partition_vector(T *v, int n, int comm_pos);
    void exchangePartitions(std::unordered_map<int, std::vector<T>>& partition_results, int comm_pos);
};

const uint seed = 15;
