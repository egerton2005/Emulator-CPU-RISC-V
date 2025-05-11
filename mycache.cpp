#include "mycache.hpp"

uint32_t CacheLRU::access(uint32_t address, bool is_instruction) {
    is_instruction ? ++total_accesses_inst_ : ++total_accesses_data_;
    uint32_t tag = address >> (CACHE_OFFSET_LEN + CACHE_INDEX_LEN);
    uint32_t index = (address >> CACHE_OFFSET_LEN) & (CACHE_SET_COUNT - 1);
    uint32_t offset = address & (CACHE_LINE_SIZE - 1);

    auto& set = sets_[index];
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (set[way].valid && set[way].tag == tag) {
            update_lru(set, way);
            if (is_instruction) inst_hits_++;
            else data_hits_++;
            
            return *reinterpret_cast<uint32_t*>(&set[way].data[offset]);
        }
    }

    return handle_miss(set, tag, index, address);
}

void CacheLRU::write(uint32_t address, uint32_t value, uint32_t size) {
    ++total_accesses_data_;
    uint32_t tag = address >> (CACHE_OFFSET_LEN + CACHE_INDEX_LEN);
    uint32_t index = (address >> CACHE_OFFSET_LEN) & (CACHE_SET_COUNT - 1);
    uint32_t offset = address & (CACHE_LINE_SIZE - 1);

    auto& set = sets_[index];
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (set[way].valid && set[way].tag == tag) {
            uint8_t* data_ptr = &set[way].data[offset];
            memcpy(data_ptr, &value, size);
            set[way].dirty = true;
            data_hits_++;
            update_lru(set, way);
            return;
        }
    }

    handle_write_miss(set, tag, index, address, value, size);
}

void CacheLRU::update_lru(std::vector<CacheLine>& set, int used_way) {
    for (auto& line : set) {
        if (line.valid) line.lru_counter++;
    }
    set[used_way].lru_counter = 0;
}

uint32_t CacheLRU::handle_miss(std::vector<CacheLine>& set, uint32_t tag, uint32_t index, uint32_t address) {
    int victim_way = find_lru_victim(set);
    
    if (set[victim_way].valid && set[victim_way].dirty) {
        uint32_t victim_addr = (set[victim_way].tag << (CACHE_OFFSET_LEN + CACHE_INDEX_LEN)) |
                                (index << CACHE_OFFSET_LEN);
        memory_.write_block(victim_addr, set[victim_way].data, CACHE_LINE_SIZE);
    }

    memory_.read_block(address & ~(CACHE_LINE_SIZE - 1), set[victim_way].data, CACHE_LINE_SIZE);
    
    set[victim_way].valid = true;
    set[victim_way].dirty = false;
    set[victim_way].tag = tag;
    set[victim_way].lru_counter = 0;

    return *reinterpret_cast<uint32_t*>(&set[victim_way].data[address & (CACHE_LINE_SIZE - 1)]);
}

void CacheLRU::handle_write_miss(std::vector<CacheLine>& set, uint32_t tag, uint32_t index, uint32_t address, uint32_t value, uint32_t size) {
    handle_miss(set, tag, index, address);
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (set[way].valid && set[way].tag == tag) {
            uint32_t offset = address & (CACHE_LINE_SIZE - 1);
            uint8_t* data_ptr = &set[way].data[offset];
            memcpy(data_ptr, &value, size);
            set[way].dirty = true;
            update_lru(set, way);
            break;
        }
    }
}

int CacheLRU::find_lru_victim(const std::vector<CacheLine>& set) {
    int victim_way = 0;
    uint32_t max_counter = 0;
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (!set[way].valid) return way;
        if (set[way].lru_counter > max_counter) {
            max_counter = set[way].lru_counter;
            victim_way = way;
        }
    }
    // std::cout<<"displacement\n";
    return victim_way;
}

uint8_t* CacheLRU::output(uint64_t address) {
    return memory_.output(address);
}



uint32_t CachePLRU::access(uint32_t address, bool is_instruction) {
    is_instruction ? ++total_accesses_inst_ : ++total_accesses_data_;

    uint32_t tag = address >> (CACHE_OFFSET_LEN + CACHE_INDEX_LEN);
    uint32_t index = (address >> CACHE_OFFSET_LEN) & (CACHE_SET_COUNT - 1);
    uint32_t offset = address & (CACHE_LINE_SIZE - 1);

    auto& set = sets_[index];
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (set[way].valid && set[way].tag == tag) {
            update_plru_bits(set, way);
            if (is_instruction) inst_hits_++;
            else data_hits_++;
            
            return *reinterpret_cast<uint32_t*>(&set[way].data[offset]);
        }
    }

    return handle_miss(set, tag, index, address);
}

void CachePLRU::write(uint32_t address, uint32_t value, uint32_t size) {
    ++total_accesses_data_;
    uint32_t tag = address >> (CACHE_OFFSET_LEN + CACHE_INDEX_LEN);
    uint32_t index = (address >> CACHE_OFFSET_LEN) & (CACHE_SET_COUNT - 1);
    uint32_t offset = address & (CACHE_LINE_SIZE - 1);

    auto& set = sets_[index];
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (set[way].valid && set[way].tag == tag) {
            uint8_t* data_ptr = &set[way].data[offset];
            memcpy(data_ptr, &value, size);
            set[way].dirty = true;
            data_hits_++;
            update_plru_bits(set, way);
            return;
        }
    }

    handle_write_miss(set, tag, index, address, value, size);
}

void CachePLRU::update_plru_bits(CacheSet& set, int used_way) {
    set.plru_bits |= (1 << used_way);
    if ((set.plru_bits & 0xF) == 0xF) {
        set.plru_bits = (1 << used_way);
    }
}

uint32_t CachePLRU::handle_miss(CacheSet& set, uint32_t tag, uint32_t index, uint32_t address) {
    int victim_way = find_lru_victim(set);
    
    if (set[victim_way].valid && set[victim_way].dirty) {
        uint32_t victim_addr = (set[victim_way].tag << (CACHE_OFFSET_LEN + CACHE_INDEX_LEN)) |
                                (index << CACHE_OFFSET_LEN);
        memory_.write_block(victim_addr, set[victim_way].data, CACHE_LINE_SIZE);
    }

    memory_.read_block(address & ~(CACHE_LINE_SIZE - 1), set[victim_way].data, CACHE_LINE_SIZE);
    
    set[victim_way].valid = true;
    set[victim_way].dirty = false;
    set[victim_way].tag = tag;
    update_plru_bits(set, victim_way);

    return *reinterpret_cast<uint32_t*>(&set[victim_way].data[address & (CACHE_LINE_SIZE - 1)]);
}

void CachePLRU::handle_write_miss(CacheSet& set, uint32_t tag, uint32_t index, uint32_t address, uint32_t value, uint32_t size) {
    handle_miss(set, tag, index, address);
    
    for (int way = 0; way < CACHE_WAY; way++) {
        if (set[way].valid && set[way].tag == tag) {
            uint32_t offset = address & (CACHE_LINE_SIZE - 1);
            uint8_t* data_ptr = &set[way].data[offset];
            memcpy(data_ptr, &value, size);
            set[way].dirty = true;
            update_plru_bits(set, way);
            break;
        }
    }
}

int CachePLRU::find_lru_victim(CacheSet& set) {
    for (int way = 0; way < CACHE_WAY; way++) {
        if ((set.plru_bits & (1 << way)) == 0) {
            return way;
        }
    }
    return 0;
}

uint8_t* CachePLRU::output(uint64_t address) {
    return memory_.output(address);
}