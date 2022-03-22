#include "intersect.h"
#include "iostream"
#include <mpi.h>
#include <mpi_helper.h>
#include <vector>

std::vector<MPI_Comm> SmartIntersect::get_comm(std::vector<std::vector<int>> &comm_groups) {
    MPI_Group world_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    auto create_group = [&world_group](std::vector<int> &comm_group) {
        MPI_Group custom_grp;
        MPI_Group_incl(world_group, comm_group.size(), comm_group.data(), &custom_grp);

        MPI_Comm custom_comm;
        MPI_Comm_create_group(MPI_COMM_WORLD, custom_grp, 0, &custom_comm);
        return custom_comm;
    };

    std::vector <MPI_Comm> all_comm;
    for (auto &comm_group: comm_groups) {
        MPI_Comm temp = create_group(comm_group);
        all_comm.push_back(temp);
    }
    return all_comm;
}

template<class T>
void SmartIntersect::run(std::vector<T> &vR, std::vector<T> &vS,
    std::vector<std::vector<int>> &comm_groups, std::vector<std::vector<int>> &dist) {
    run(vR.data(), vS.data(), vR.size(), vS.size(), comm_groups, dist);
}

template<class T>
void SmartIntersect::run(T *vR, T *vS, const int nR, const int nS,
    std::vector<std::vector<int>> &comm_groups, std::vector<std::vector<int>> &dist) {
    const auto MPI_TYPE = GET_TYPE<T>;
    int s_meta_tag = 123;
    int s_data_tag = 12;
    int r_meta_tag = 234;
    int r_data_tag = 23;
    int finish_tag = 0;
    int send_tag = 1;
    std::vector<int> resultS;
    std::vector<int> resultR;
    // Form all communicators
    std::vector<MPI_Comm> comms = SmartIntersect::get_comm(comm_groups);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

//    std::cout << "here at rank " << rank << "\n";
//    std::cout << rank << " " << *vR << " " << *(vR+1) << "\n";
//    std::cout << rank << " " << *vS << " " << *(vS+1) << "\n";

    // send S from this node to hashed node in its communicator
    for (int comm_index = 0; comm_index < comms.size(); comm_index++) {
        if (comms[comm_index] != MPI_COMM_NULL) {
            MPI_Comm my_grp_comm = comms[comm_index];
            int comm_rank;
            int my_comm_size;
            MPI_Comm_rank(my_grp_comm, &comm_rank);
            MPI_Comm_size(my_grp_comm, &my_comm_size);

            MPI_Request meta_requestsS[nS];
            MPI_Request data_requestsS[nS];
            IntersectHash obj(dist[comm_index]);

            for (int i = 0; i < nS; i++) {
                int receiver = obj.hash(vS[i]);
                int partner = comm_groups[comm_index][receiver];
                std::cout << "sending " << vS[i] << " from " << world_rank << " "" to " << partner << "\n";
//                if (receiver != comm_rank) {
                    MPI_Issend(&send_tag, 1, MPI_TYPE, receiver, s_meta_tag, my_grp_comm, &meta_requestsS[i]);
                    MPI_Issend(&vS[i], 1, MPI_TYPE, receiver, s_data_tag, my_grp_comm, &data_requestsS[i]);
//                } else {
//                    resultS.push_back(vS[i]);
//                }
            }

            MPI_Request comm_meta_requestsS[my_comm_size];
            for (int i = 0; i < my_comm_size; i++) {
                MPI_Issend(&finish_tag, 1, MPI_TYPE, i, s_meta_tag, my_grp_comm, &comm_meta_requestsS[i]);
            }

            MPI_Request requestsS[my_comm_size];
            std::vector<int> s_recv_meta(world_size, 0);
            std::vector<int> s_recv_data(world_size, 0);
            for (int i = 0; i < my_comm_size; i++) {
                MPI_Irecv(&s_recv_meta[i], 1, MPI_INT, i, s_meta_tag, my_grp_comm, &requestsS[i]);
            }

            int finish = 0;
            while (finish < my_comm_size) {
                int index;
                MPI_Waitany(my_comm_size, requestsS, &index, MPI_STATUS_IGNORE);
                if (s_recv_meta[index] == 1) {
                    MPI_Recv(&s_recv_data[index], 1, MPI_INT, index, s_data_tag, my_grp_comm, MPI_STATUS_IGNORE);
                    resultS.push_back(s_recv_data[index]);
                    MPI_Irecv(&s_recv_meta[index], 1, MPI_INT, index, s_meta_tag, my_grp_comm, &requestsS[index]);
                } else {
                    finish++;
                }
            }
            MPI_Waitall(my_comm_size, requestsS, MPI_STATUSES_IGNORE);
        }
    }

    std::cout << "S here at rank " << world_rank << "\n";
    std::cout << world_rank << " ";
    for (int i = 0; i < resultS.size(); i++) {
        std::cout << resultS[i] << " ";
    }
    std::cout << "\n";

    MPI_Request meta_requestsR[nR][comm_groups.size()];
    MPI_Request data_requestsR[nR][comm_groups.size()];

    for (int i = 0; i < nR; i++) {
        for (int comm_index2 = 0; comm_index2 < comm_groups.size(); comm_index2++) {
            IntersectHash obj(dist[comm_index2]);
            int receiver = obj.hash(vR[i]);
            int partner = comm_groups[comm_index2][receiver];
            std::cout << "sending " << vR[i] << " from " << world_rank << " "" to "
                << partner << " in group " << comm_index2 << "\n";
            MPI_Issend(&send_tag, 1, MPI_TYPE, partner, r_meta_tag, MPI_COMM_WORLD, &meta_requestsR[i][comm_index2]);
            MPI_Issend(&vR[i], 1, MPI_TYPE, partner, r_data_tag, MPI_COMM_WORLD, &data_requestsR[i][comm_index2]);
        }
    }

    // signal finish sending
    MPI_Request world_meta_requestsR[world_size];
//            i != comm_rank &&
    for (int i = 0; i < world_size; i++) {
        MPI_Issend(&finish_tag, 1, MPI_TYPE, i, r_meta_tag, MPI_COMM_WORLD, &world_meta_requestsR[i]);
    }

    // start receiving R and S from other nodes
    MPI_Request requestsR[world_size];

    std::vector<int> r_recv_meta(world_size, 0);
    std::vector<int> r_recv_data(world_size, 0);

    for (int i = 0; i < world_size; i++) {
        MPI_Irecv(&r_recv_meta[i], 1, MPI_INT, i, r_meta_tag, MPI_COMM_WORLD, &requestsR[i]);
    }

    int finish = 0;
    while (finish < world_size) {
        int index;
        MPI_Waitany(world_size, requestsR, &index, MPI_STATUS_IGNORE);
        if (r_recv_meta[index] == 1) {
            MPI_Recv(&r_recv_data[index], 1, MPI_INT, index, r_data_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            resultR.push_back(r_recv_data[index]);
            MPI_Irecv(&r_recv_meta[index], 1, MPI_INT, index, r_meta_tag, MPI_COMM_WORLD, &requestsR[index]);
        } else {
            finish++;
        }
    }
    MPI_Waitall(world_size, requestsR, MPI_STATUSES_IGNORE);

    std::cout << "R here at rank " << world_rank << "\n";
    std::cout << world_rank << "  R  ";
    for (int i = 0; i < resultR.size(); i++) {
        std::cout << resultR[i] << " ";
    }
    std::cout << "\n";

    std::vector<int> v3;
    std::sort(resultR.begin(), resultR.end());
    std::sort(resultS.begin(), resultS.end());

    std::set_intersection(resultR.begin(),resultR.end(),
                          resultS.begin(),resultS.end(),
                          back_inserter(v3));

    std::cout << "intersection here at rank " << world_rank << "\n";
    std::cout << world_rank << "  final  ";
    for (int i = 0; i < v3.size(); i++) {
        std::cout << v3[i] << " ";
    }
    std::cout << "\n";
}

template void SmartIntersect::run<int>(int *, int *, const int, const int,
    std::vector<std::vector<int>> &, std::vector<std::vector<int>> &);
template void SmartIntersect::run<int>(std::vector<int> &, std::vector<int> &,
   std::vector<std::vector<int>> &, std::vector<std::vector<int>> &);

//            MPI_Waitall(nS, meta_requestsS, MPI_STATUSES_IGNORE);
//            MPI_Waitall(nS, meta_requestsS, MPI_STATUSES_IGNORE);
//            MPI_Waitall(nS, data_requestsS, MPI_STATUSES_IGNORE);