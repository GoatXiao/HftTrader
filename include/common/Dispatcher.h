
#pragma once
#include <atomic>

template<class T, uint32_t CNT>
class Dispatcher
{
public:
    static_assert(CNT && !(CNT & (CNT - 1)), "CNT must be a power of 2");
    
    struct Reader
    {
        T* read() {
            auto& blk = q->blks[read_idx % CNT]; // friend
            uint32_t blk_idx = ((std::atomic<uint32_t>*)&blk.idx)->load(std::memory_order_acquire);
            if (blk_idx >= read_idx) {
                ++read_idx; // next read idx
                return &blk.data;
            } else {
                return nullptr;
            }
        };

        Dispatcher<T, CNT>* q = nullptr;
        uint32_t read_idx; // used only by reading thread
    };

    Reader getReader() {
        Reader reader;
        reader.q = this;
        reader.read_idx = write_idx + 1; // read from 1
        return reader;
    };

    template<typename Writer>
    void write(Writer writer) {
        auto& blk = blks[++write_idx % CNT]; // write from 1
        writer(blk.data);
        ((std::atomic<uint32_t>*)&blk.idx)->store(write_idx, std::memory_order_release);
    };

private:
    friend class Reader;

    struct alignas(64) Block
    {
        uint32_t idx = 0; // the only atomic barrier
        T data;
    } blks[CNT] = {};

    alignas(128) uint32_t write_idx = 0; // used only by writing thread
};
