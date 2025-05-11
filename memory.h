#pragma once
#include "constant.h"

#include <iostream>
#include <vector>
#include <cstdint>
#include <memory>
#include <fstream>
#include <cstring>



class Memory {
private:
    std::vector<uint8_t> data_ = std::vector<uint8_t>(MEMORY_SIZE);

public:
    Memory() = default;

    Memory(const Memory& mem) {
        std::copy(mem.data_.begin(), mem.data_.end(), data_.begin());
    }

    void load_fragment(std::ifstream& file);

    void read_block(uint32_t address, uint8_t* buffer, uint32_t size) const;

    void write_block(uint32_t address, const uint8_t* buffer, uint32_t size);

    uint8_t* output(uint64_t address);
};