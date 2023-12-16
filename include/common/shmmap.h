#ifndef  __SHMMAP_H__
#define  __SHMMAP_H__
//使用共享内存大页设置
//挂载Hugetlb文件系统
// mkdir /mnt/ huge
// mount none /mnt/huge -t hugetlbfs
// cat /proc/meminfo
// 需要修改机器的hugepage相关配置 
//END
#include <bits/stdc++.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#define MAP_LENGTH (10*1024*1024) //10MB

template<class T>
T* shmmap(const char * filename) {
    int shm_fd = open(filename, O_CREAT | O_RDWR, 0666);
    //int shm_fd = shm_open(filename, O_CREAT | O_RDWR, 0666);
    if(shm_fd == -1) {
        std::cerr << "shm_open failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    //if(ftruncate(shm_fd, MAP_LENGTH)) {
    //    std::cerr << "ftruncate failed: " << strerror(errno) << std::endl;
    //    close(shm_fd);
    //    return nullptr;
    //}
    T* ret = (T*)mmap(0, MAP_LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    if(ret == MAP_FAILED) {
        std::cerr << "mmap failed: " << strerror(errno) << std::endl;
        return nullptr;
    }
    return ret;
}

#endif
