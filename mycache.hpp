#pragma once

#include "memory.hpp"

#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>


struct CacheLine {
    bool valid = false;
    bool dirty = false;
    uint32_t tag = 0;
    uint32_t lru_counter = 0;
    uint8_t data[CACHE_LINE_SIZE];
};

class CacheLRU {
private:
    std::vector<std::vector<CacheLine>> sets_ = std::vector<std::vector<CacheLine>>(CACHE_SET_COUNT, std::vector<CacheLine>(CACHE_WAY));
    Memory memory_;
    //statistic
    
public:
    uint32_t total_accesses_inst_ = 0;
    uint32_t total_accesses_data_ = 0;
    uint32_t inst_hits_ = 0;
    uint32_t data_hits_ = 0;

    CacheLRU(const Memory& mem) 
        : memory_(mem)
    { }

    uint32_t access(uint32_t address, bool is_instruction);

    void write(uint32_t address, uint32_t value, uint32_t size);

    uint8_t* output(uint64_t address);
private:
    void update_lru(std::vector<CacheLine>& set, int used_way);

    uint32_t handle_miss(std::vector<CacheLine>& set, uint32_t tag, uint32_t index, uint32_t address);

    void handle_write_miss(std::vector<CacheLine>& set, uint32_t tag, uint32_t index, uint32_t address, uint32_t value, uint32_t size);

    int find_lru_victim(const std::vector<CacheLine>& set);

};

struct CacheSet {
    std::vector<CacheLine> lines = std::vector<CacheLine>(CACHE_WAY);
    uint32_t plru_bits = 0;
    CacheLine& operator[](int i) {
        return lines[i];
    }
};

class CachePLRU {
private:
    std::vector<CacheSet> sets_ = std::vector<CacheSet>(CACHE_SET_COUNT);
    Memory memory_;
    //statistic
    
public:
    uint32_t total_accesses_inst_ = 0;
    uint32_t total_accesses_data_ = 0;
    uint32_t inst_hits_ = 0;
    uint32_t data_hits_ = 0;
    
    CachePLRU(const Memory& mem) 
        : memory_(mem)
    { }

    uint32_t access(uint32_t address, bool is_instruction);

    void write(uint32_t address, uint32_t value, uint32_t size);

    uint8_t* output(uint64_t address);
private:
    void update_plru_bits(CacheSet& set, int used_way);

    uint32_t handle_miss(CacheSet& set, uint32_t tag, uint32_t index, uint32_t address);

    void handle_write_miss(CacheSet& set, uint32_t tag, uint32_t index, uint32_t address, uint32_t value, uint32_t size);

    int find_lru_victim(CacheSet& set);

};