#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
class SimpleAgg {
    template <class T>
    void run(T* v, const int n, const int root, MPI_Comm comm) {
        const int WORLD_SIZE = MPIHelper::get_world_size();
        const int rank = MPIHelper::get_rank();
        using MPI_DataType = typename GetMPIDataType<T>::type;
        MPI_Request reqs[100];
        if (rank == 0) {
            std::vector<T*> buf(WORLD_SIZE-1);
            for (auto& i: buf) i = new T[n];
            for (int i = 1; i < WORLD_SIZE; i++) {
                MPI_Irecv(buf[i-1], n, MPI_DataType(), i, 1996, comm, reqs + i - 1);
            }
            MPI_Waitall(WORLD_SIZE - 1, reqs, MPI_STATUSES_IGNORE);
            for (int j = 1; j < WORLD_SIZE; j++) {
                const auto& bufj = buf[j];
                for (int i = 0; i < n; i++) {
                    v[i] += bufj[i];
                }
            }
            for (auto& i: buf) delete[] i;
        } else {
            MPI_Send(v, n, MPI_DataType(), root, 1996, comm);
        }
    }
};
template void SimpleAgg::run<int>(int*, const int, const int, MPI_Comm);