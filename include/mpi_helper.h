#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream>
#include <fstream>

class MPIHelper;
constexpr bool enable_debug = false;
class Buf {
    void* ptr;
    size_t len;
    bool in_use;
public:
    Buf(): ptr(nullptr), len(0), in_use(false){}
    friend MPIHelper;
};

template <class>
constexpr MPI_Datatype GET_TYPE = MPI_DATATYPE_NULL;

template <>
constexpr MPI_Datatype GET_TYPE<int> = MPI_INT;


class MPIHelper {
    int rank = -1;
    int world_size = -1;
    MPIHelper() = default;
    constexpr static int BUF_LEN = 3;
    std::array<Buf, BUF_LEN> buf;
    std::vector<Buf> temp_buf;
    constexpr static bool log_sep_files = false;
    std::vector<std::ofstream> log_files;
public:
    static MPIHelper& getInstance() {
        static MPIHelper instance;
        return instance;
    }
    static void init(const MPI_Comm& comm) {
        auto& in = getInstance();
        MPI_Comm_rank(comm, &in.rank);
        MPI_Comm_size(comm, &in.world_size);
        if (log_sep_files) {
            for (int i = 0; i < in.world_size; i++) {
                in.log_files.push_back(std::ofstream("/tmp/log/" + std::to_string(i)));
            }
        }
    }
    template<class T>
    T* get_buf(size_t sz) {
        for (auto& i: buf) {
            if (i.len >= sz && !i.in_use) {
                i.in_use = true;
                return (T*)i.ptr;
            }
        }
        for (auto& i: buf) {
            if (!i.in_use) {
                if (i.len) {
                    delete[] i.ptr;
                }
                i.ptr = new char[sz];
                i.len = sz;
                i.in_use = true;
                return (T*)i.ptr;
            }
        }
        // allocate to temporary buffer
        Buf b;
        b.ptr = new char[sz];
        bzero(b.ptr, sz);
        b.len = sz;
        b.in_use = true;
        temp_buf.push_back(b);
        return (T*)b.ptr;
    }
    void clear_memory() {
        while (temp_buf.size()) {
            delete[] (char*)(temp_buf.back().ptr);
            temp_buf.pop_back();
        }
        for (auto& i: buf) {
            i.in_use = false;
        }
    }
    static int get_rank() {return getInstance().rank;}
    static int get_world_size() {return getInstance().world_size;}
//    template <class T1, class... T>
    template <class... Ts>
    static void Log(Ts&&... args) {
        if (!enable_debug) return;
        const int rank = get_rank();
        std::ostream& o = (log_sep_files) ? getInstance().log_files[rank] : std::cerr;
        o << "[" << rank << "/" << get_world_size << "] ";
        log(o, args...);
    }

private:
    static void log(std::ostream& o) {
        o << "\n";
    }
    template <class T, class... Ts>
    static void log(std::ostream& o, T&& arg, Ts&&... args) {
        o << arg;
        log(o, args...);
    }
};
