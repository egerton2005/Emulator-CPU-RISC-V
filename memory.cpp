#include "memory.hpp"

void Memory::load_fragment(std::ifstream& file) {
    while (file.peek() != EOF) {
        uint32_t address, fragment_size;

        if (!file.read(reinterpret_cast<char*>(&address), 4)) break;
        if (!file.read(reinterpret_cast<char*>(&fragment_size), 4)) break;

        file.read(reinterpret_cast<char*>(&data_[address]), fragment_size);

    }
}

void Memory::read_block(uint32_t address, uint8_t* buffer, uint32_t size) const {
    memcpy(buffer, &data_[address], size);
}

void Memory::write_block(uint32_t address, const uint8_t* buffer, uint32_t size) {
    memcpy(&data_[address], buffer, size);
}

uint8_t* Memory::output(uint64_t address) {
    return &data_[address];
}