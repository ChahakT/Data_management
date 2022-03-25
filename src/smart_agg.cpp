#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include <cassert>
#include <map>
#include "agg.h"

template<class T>
SmartAgg<T>::SmartAgg(std::map<int, std::vector<std::vector<int>>>& stepCommInstructions) {
    MPI_Group world_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    auto create_group = [&world_group](std::vector<int> &comm_group) {
        MPI_Group custom_grp;
        MPI_Group_incl(world_group, (int) comm_group.size(), comm_group.data(), &custom_grp);

        MPI_Comm custom_comm;
        MPI_Comm_create_group(MPI_COMM_WORLD, custom_grp, 0, &custom_comm);
        return custom_comm;
    };

    for (auto& [step, stepCommRanks] : stepCommInstructions) {
        for(auto &stepCommRank : stepCommRanks) {
            assert(stepCommRank.size() > 1);
            MPI_Comm custom_comm = create_group(stepCommRank);
            stepCommGroups[step].emplace_back(custom_comm);
        }
    }
}

template<class T>
SmartAgg<T>::~SmartAgg() {
    for(auto const& [_, stepCommGroup] : stepCommGroups) {
        for (auto customComm: stepCommGroup)
            if (customComm != MPI_COMM_NULL)
                MPI_Comm_free(&customComm);
    }
}

template<class T>
void SmartAgg<T>::run(std::vector <T> &v) {
    run(v.data(), v.size());
}

template<class T>
void SmartAgg<T>::run(T *v, const size_t n) {
    const auto MPI_TYPE = GET_TYPE<T>;
    //const int rank = MPIHelper::get_rank();

    for(auto const& [step, stepCommGroup] : stepCommGroups) {
        for(auto customComm: stepCommGroup) {
            if(customComm == MPI_COMM_NULL) continue;
            int customCommRank, customCommSize;
            MPI_Comm_rank(customComm, &customCommRank);
            MPI_Comm_size(customComm, &customCommSize);
            if(customCommRank == 0) {
                MPI_Request reqs[100];
                std::vector<std::unique_ptr<T[]>> buf(customCommSize-1);
                for (auto& i: buf) { i = std::make_unique<T[]>(n); }
                for (int i = 1; i < customCommSize; i++) {
                    auto& bufJ = buf[i - 1];
                    MPI_Irecv(bufJ.get(), n, MPI_TYPE, i, step, customComm, reqs + i - 1);
//                std::vector<T*> buf(customCommSize-1);
                }
                MPI_Waitall(customCommSize - 1, reqs, MPI_STATUSES_IGNORE);
                for (auto &bufJ: buf) {
                    for (size_t i = 0; i < n; i++)
                        v[i] += bufJ[i];
                }
            } else {
                MPI_Send(v, n, MPI_TYPE, 0, step, customComm);
            }
        }
    }
}

template class SmartAgg<int>;
