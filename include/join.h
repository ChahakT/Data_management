#pragma once

#include <utility>
#include <vector>
#include <functional> // std::hash
#include <cassert>
#include <unordered_map>
#include <stdexcept> // std::logic_error
#include <iostream>

typedef struct ompi_communicator_t *MPI_Comm; // typedef here to avoid incl mpi.h

template<class T>
class SimpleJoin {
public:
    std::vector<T> run(T *vR, T *vS, int nR, int nS, MPI_Comm comm);
    std::vector<T> run(std::vector<T> &vR, std::vector<T> &vS, MPI_Comm comm);
    SimpleJoin() = default;
    SimpleJoin(const SimpleJoin&) = delete;
    SimpleJoin& operator= (const SimpleJoin&) = delete;
};


struct JoinHash {
public:
    explicit JoinHash(std::vector<int> v_) : v(std::move(v_)) {
        if (v.empty())
            throw std::logic_error("cannot create JoinHash with 0 vals");
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
class SmartJoin {
public:
    explicit SmartJoin(std::vector<std::vector<int>> &comm_groups, std::vector<std::vector<int>> &group_dist);
    std::vector<T> run(T *vR, T *vS, uint nR, uint nS);
    std::vector<T> run(std::vector<T> &vR, std::vector<T> &vS);
    SmartJoin(const SmartJoin&) = delete;
    SmartJoin& operator= (const SmartJoin&) = delete;
    ~SmartJoin();
private:
    std::vector<std::vector<int>> commGroupRanks;
    std::vector<MPI_Comm> commList;
    std::vector<JoinHash> hashObjList;

    std::unordered_map<int, std::vector<T>> partition_vector_intra(T *v, int n, int comm_pos) const;
    std::unordered_map<int, std::vector<T>> partition_vector_inter(T *v, int n) const;
    std::vector<T> exchange_partitions(std::unordered_map<int, std::vector<T>> &partition_results, int comm_rank, int comm_size,
                             MPI_Comm comm);
    std::vector<T> exchange_partitions_intra(std::unordered_map<int, std::vector<T>> &partition_results, int comm_pos);
    std::vector<T> exchange_partitions_inter(std::unordered_map<int, std::vector<T>> &world_partitions);
    std::vector<T> findSetIntersection(std::vector<T>& v1, std::vector<T>& v2);
};