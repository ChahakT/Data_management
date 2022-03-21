#pragma once
#include <vector>
#include <functional> // std::hash
#include <assert.h>
#include <stdexcept> // std::logic_error

typedef struct ompi_communicator_t *MPI_Comm; // typedef here to avoid incl mpi.h
class SimpleIntersect {
public:
    template <class T>
    std::vector<T> run(T *vR, T *vS, const int nR, const int nS, MPI_Comm comm);
    template <class T>
    std::vector<T> run(std::vector <T> &vR, std::vector <T> &vS, MPI_Comm comm);
};

struct IntersectHash {
    IntersectHash(const std::vector<int>& v_): v(v_) {
        if (v.size() == 0)
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
private:
    std::vector<int> v;
};

const uint seed = 15;
const int nS = 4, nR = 3;
