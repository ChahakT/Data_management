#pragma once

class SimpleIntersect {
public:
    template <class T>
    std::vector<T> run(T *vR, T *vS, const int nR, const int nS, MPI_Comm comm);
    template <class T>
    std::vector<T> run(std::vector <T> &vR, std::vector <T> &vS, MPI_Comm comm);
};

const uint seed = 15;
const int nS = 4, nR = 3;