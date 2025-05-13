#include <iostream>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include <fstream>
#include <cstring>
#include <bitset>
#include <cmath> 
#include <new>    

#include "mycache.hpp"

void ProcessingR(uint32_t instr, std::vector<uint32_t>& regs) {
    uint8_t rd = (instr >> 7) & ((1 << SIZE_RD) - 1);
    uint8_t funct3 = (instr >> 12) & ((1 << SIZE_FUNC3) - 1);
    uint8_t rs1 = (instr >> 15) & ((1 << SIZE_RS1) - 1);
    uint8_t rs2 = (instr >> 20) & ((1 << SIZE_RS2) - 1);
    uint8_t funct7 = (instr >> 25) & ((1 << SIZE_FUNC7) - 1);
    if(funct7 == 0x01) {
        if(funct3 == FMT_R::mul) {
            int64_t value = static_cast<int64_t>(static_cast<int32_t>(regs[rs1])) * static_cast<int64_t>(static_cast<int32_t>(regs[rs2]));
            regs[rd] = value & 0xFFFFFFFF;
        }
        else if(funct3 == FMT_R::mulh) {
            int64_t value = static_cast<int64_t>(static_cast<int32_t>(regs[rs1])) * static_cast<int64_t>(static_cast<int32_t>(regs[rs2]));
            regs[rd] = (value >> 32) & 0xFFFFFFFF;
        }
        else if(funct3 == FMT_R::mulhsu) {
            int64_t value = static_cast<int64_t>(static_cast<int32_t>(regs[rs1])) * static_cast<uint64_t>(regs[rs2]);
            regs[rd] = (value >> 32) & 0xFFFFFFFF;
        }
        else if(funct3 == FMT_R::mulhu) {
            int64_t value = static_cast<uint64_t>(regs[rs1]) * static_cast<uint64_t>(regs[rs2]);
            regs[rd] = (value >> 32) & 0xFFFFFFFF;
        }
        else if(funct3 == FMT_R::divv) {
            regs[rd] = static_cast<int32_t>(regs[rs1]) / static_cast<int32_t>(regs[rs2]);
        }
        else if(funct3 == FMT_R::divu) {
            regs[rd] = (regs[rs1] / regs[rs2]);
        }
        else if(funct3 == FMT_R::rem) {
            regs[rd] = static_cast<int32_t>(regs[rs1]) % static_cast<int32_t>(regs[rs2]);
        }
        else if(funct3 == FMT_R::remu) {
            regs[rd] = (regs[rs1] % regs[rs2]);
        }
    }
    else if(funct3 == FMT_R::add && funct7 == 0x0) {
        regs[rd] = regs[rs1] + regs[rs2];
    }
    else if (funct3 == FMT_R::sub) {
        regs[rd] = regs[rs1] - regs[rs2]; 
    }
    else if(funct3 == FMT_R::xorr) {
        regs[rd] = regs[rs1] ^ regs[rs2]; 
    }
    else if(funct3 == FMT_R::orr) {
        regs[rd] = regs[rs1] | regs[rs2]; 
    }
    else if(funct3 == FMT_R::andd) {
        regs[rd] = regs[rs1] & regs[rs2]; 
    }
    else if(funct3 == FMT_R::sll) {
        regs[rd] = regs[rs1] << regs[rs2]; 
    }
    else if(funct3 == FMT_R::srl && funct7 == 0x0) {
        regs[rd] = regs[rs1] >> (regs[rs2] & 0x1F); 
    }
    else if(funct3 == FMT_R::sra) {
        int32_t value = static_cast<int32_t>(regs[rs1]);
        regs[rd] = value >> (regs[rs2] & 0x1F); 
    }
    else if(funct3 == FMT_R::slt) {
        regs[rd] = (static_cast<int32_t>(regs[rs1]) < static_cast<int32_t>(regs[rs2])) ? 1 : 0; 
    }
    else if(funct3 == FMT_R::sltu) {
        regs[rd] = (regs[rs1] < regs[rs2]) ? 1 : 0; 
    }
}

template<typename Cache>
void ProcessingII(uint32_t instr, std::vector<uint32_t>& regs, Cache& cache) {
    uint8_t rd = (instr >> 7) & ((1 << SIZE_RD) - 1);
    uint8_t funct3 = (instr >> 12) & ((1 << SIZE_FUNC3) - 1);
    uint8_t rs1 = (instr >> 15) & ((1 << SIZE_RS1) - 1);
    int32_t imm = static_cast<int32_t>(((instr >> 20) & ((1 << (SIZE_FUNC7 + SIZE_RS2)) - 1)) << 20) >> 20;
    uint32_t address = regs[rs1] + imm;
    uint32_t data  = cache.access(address, false);
    if(funct3 == FMT_I::lb) {
        regs[rd] = static_cast<int32_t>((static_cast<int8_t>(data & 0xFF)));
    }
    else if (funct3 == FMT_I::lh) {
        regs[rd] = static_cast<int32_t>((static_cast<int16_t>(data & 0xFFFF)));
    }
    else if (funct3 == FMT_I::lw) {
        regs[rd] = data;
    }
    else if (funct3 == FMT_I::lbu) {
        regs[rd] = data & 0xFF; 
    }
    else if (funct3 == FMT_I::lhu) {
        regs[rd] = data & 0xFFFF; 
    }
}

template<typename Cache>
void ProcessingS(uint32_t instr, std::vector<uint32_t>& regs, Cache& cache) {
    uint32_t imm1 = (instr >> 7) & ((1 << SIZE_RD) - 1);
    uint8_t funct3 = (instr >> 12) & ((1 << SIZE_FUNC3) - 1);
    uint8_t rs1 = (instr >> 15) & ((1 << SIZE_RS1) - 1);
    uint8_t rs2 = (instr >> 20) & ((1 << SIZE_RS2) - 1);
    int32_t imm = static_cast<int32_t>(((((instr >> 25) & ((1 << SIZE_FUNC7) - 1)) << SIZE_RD) | imm1) << 20) >> 20;
    uint32_t address = regs[rs1] + imm;
    // uint32_t data  = cache.access(address, false);
    if(funct3 == FMT_S::sb) {
        cache.write(address, (regs[rs2] & 0xFF), 1);
    }
    else if (funct3 == FMT_S::sh) {
        cache.write(address, (regs[rs2] & 0xFFFF), 2);
    }
    else if (funct3 == FMT_S::sw) {
        cache.write(address, regs[rs2], 4);
    }   
}

void ProcessingI(uint32_t instr, std::vector<uint32_t>& regs, uint32_t& pc) {
    uint8_t rd = (instr >> 7) & ((1 << SIZE_RD) - 1);
    uint8_t funct3 = (instr >> 12) & ((1 << SIZE_FUNC3) - 1);
    uint8_t rs1 = (instr >> 15) & ((1 << SIZE_RS1) - 1);
    int32_t imm = static_cast<int32_t>(((instr >> 20) & ((1 << (SIZE_FUNC7 + SIZE_RS2)) - 1)) << 20) >> 20;
    if(funct3 == FMT_I::jalr && (instr & 0x7F) == 0b1100111) {
        regs[rd] = pc;
        pc += regs[rs1] + imm - 4;
    }
    else if(funct3 == FMT_I::addi) {
        regs[rd] = regs[rs1] + imm;
    }
    else if (funct3 == FMT_I::xori) {
        regs[rd] = regs[rs1] ^ imm;
    }
    else if (funct3 == FMT_I::ori) {
        regs[rd] = regs[rs1] | imm;
    }
    else if (funct3 == FMT_I::andi) {
        regs[rd] = regs[rs1] & imm; 
    }
    else if (funct3 == FMT_I::slli) {
        uint8_t shift = imm & 0x1F; 
        regs[rd] = regs[rs1] << shift;
    }
    else if (funct3 == FMT_I::srli) {
        uint8_t shift = imm & 0x1F; 
        regs[rd] = regs[rs1] >> shift;
    }
    else if (funct3 == FMT_I::srai) {
        int32_t value = static_cast<int32_t>(regs[rs1]);
        uint8_t shift = imm & 0x1F; 
        regs[rd] = value >> shift; 
    }
    else if (funct3 == FMT_I::slti) {
        regs[rd] = (static_cast<int32_t>(regs[rs1]) < imm) ? 1 : 0;
    }
    else if (funct3 == FMT_I::sltiu) {
        regs[rd] = (regs[rs1] > ((instr >> 20) & ((1 << (SIZE_FUNC7 + SIZE_RS2)) - 1))) ? 1 : 0;
    }
}

void ProcessingB(uint32_t instr, std::vector<uint32_t>& regs, uint32_t& pc) {
    uint8_t funct3 = (instr >> 12) & ((1 << SIZE_FUNC3) - 1);
    uint8_t rs1 = (instr >> 15) & ((1 << SIZE_RS1) - 1);
    uint8_t rs2 = (instr >> 20) & ((1 << SIZE_RS2) - 1);
    uint32_t imm_prom = (((instr >> 31) & 0x1) << 12) |
                (((instr >> 7) & 0x1) << 11) |
                (((instr >> 25) & 0x3F) << 5) |
                (((instr >> 8) & 0xF) << 1);
    int32_t imm = static_cast<int32_t>(imm_prom << 19) >> 19;
    if(funct3 == FMT_B::beq) {
        if(regs[rs1] == regs[rs2]) pc += imm - 4;
    }
    else if (funct3 == FMT_B::bne) {
        if(regs[rs1] != regs[rs2]) pc += imm - 4;
    }
    else if (funct3 == FMT_B::blt) {
        if(static_cast<int32_t>(regs[rs1]) < static_cast<int32_t>(regs[rs2])) pc += imm - 4;
    }
    else if (funct3 == FMT_B::bge) {
        if(static_cast<int32_t>(regs[rs1]) >= static_cast<int32_t>(regs[rs2])) pc += imm - 4; 
    }
    else if (funct3 == FMT_B::bltu) {
        if(regs[rs1] < regs[rs2]) pc += imm - 4;
    }
    else if (funct3 == FMT_B::bgeu) {
        if(regs[rs1] >= regs[rs2]) pc += imm - 4;
    }
}

void ProcessingJU(uint32_t instr, std::vector<uint32_t>& regs, uint32_t& pc) {
    uint8_t rd = (instr >> 7) & ((1 << SIZE_RD) - 1);
    int32_t imm_u = static_cast<int32_t>(instr) >> 12;
    uint32_t imm_prom_j = (((instr >> 31) & 1) << 20) | (((instr >> 12) & 0xFF) << 12);
    imm_prom_j |= (((instr >> 20) & 1) << 11) | (((instr >> 21) & 0x3FF) << 1);
    int32_t imm_j = static_cast<int32_t>(imm_prom_j << 11) >> 11; 
    if((instr & 0x7F) == FMT::J) {
        regs[rd] = pc;
        pc += imm_j - 4;
    }
    else if ((instr & 0x7F) == FMT::U) {
        regs[rd] = imm_u << 12;
    }
    else if ((instr & 0x7F) == FMT::UU) {
        regs[rd] = pc - 4 + (imm_u << 12);
    }
}

template<typename CacheType>
uint32_t run_simulation(CacheType& cache, std::vector<uint32_t>& registers) {
    uint32_t pc = registers[0];
    uint32_t stop_signal = registers[1];
    registers[0] = 0;

    while (true) {
        if(stop_signal == pc) break;
        uint32_t instr = cache.access(pc, true);
        uint32_t opcode = instr & 0x7F;

        pc += 4;
        if(opcode == FMT::R) {
            // std::cout<<1<<'\n';
            ProcessingR(instr, registers);  
        }
        else if(opcode == FMT::II) {
            // std::cout<<2<<'\n';
            ProcessingII(instr, registers, cache);
        }
        else if(opcode == FMT::S) {
            // std::cout<<3<<'\n';
            ProcessingS(instr, registers, cache);
        }
        else if(opcode == FMT::I || opcode == FMT::III) { // +jalr
            // std::cout<<4<<'\n';
            ProcessingI(instr, registers, pc);
        }
        else if(opcode == FMT::B) {
            // std::cout<<5<<'\n';
            ProcessingB(instr, registers, pc);
        }
        else if(opcode == FMT::J || opcode == FMT::U || opcode == FMT::UU) {
            // std::cout<<6<<'\n';
            ProcessingJU(instr, registers, pc);
        }
        else if(opcode == FMT::FENCE) {
            continue;
        }
        else if(opcode == FMT::I_END) break;
        else {
            std::cerr << "Undefind opcode " << opcode << '\n';
            std::cerr << "Inst: " << instr << '\n';
            std::cerr << "pc: " << pc << '\n';
            return pc;
        }
    }
    return pc;
}

void print_cache_stats(const CacheLRU& lru, const CachePLRU& plru) {
    auto print_stats = [](const auto& cache, const char* name) {
        if (cache.total_accesses_inst_ + cache.total_accesses_data_ == 0) {
            printf("%s\tnan%%\tnan%%\tnan%%\n", name);
            return;
        }
        std::cout<< "total accesses inst: " << cache.total_accesses_inst_ << " total accesses data: " << cache.total_accesses_data_ << '\n';
        std::cout<< "hits inst: " << cache.inst_hits_ << "hits data: " << cache.data_hits_ << '\n';
        double total_rate = ((cache.inst_hits_ + cache.data_hits_) * (double)100.0) / (cache.total_accesses_inst_ + cache.total_accesses_data_);
        double inst_rate = (cache.inst_hits_ * (double)100.0) / (cache.total_accesses_inst_);
        double data_rate = (cache.data_hits_ * (double)100.0) / (cache.total_accesses_data_);
        
        printf("%s\t%3.5f%%\t%3.5f%%\t%3.5f%%\n", 
               name, total_rate, inst_rate, data_rate);
    };
    
    printf("replacement\thit rate\thit rate (inst)\thit rate (data)\n");
    print_stats(lru, "        LRU");
    print_stats(plru, "      bpLRU");
}


int main(int argc, char* argv[]) {
    std::string input_name, output_name;
    uint64_t start_fragment, size_fragment;
    for(int i = 1; i < argc; ++i) {
        if(!std::strcmp(argv[i], "-i") && i + 1 < argc) {
            input_name = argv[++i];
        }
        else if(!std::strcmp(argv[i], "-o") && i + 3 < argc) {
            output_name = argv[++i];
            start_fragment = strtoull(argv[++i], nullptr, 10);
            size_fragment = strtoull(argv[++i], nullptr, 10);
        }
        else {
            std::cerr << "Uncorrected args\n"; 
            return 0;
        }
    }

    std::ifstream fin(input_name, std::ios::binary);
    if(!fin.is_open()) {
        std::cerr << "Uncorrected input file name\n"; 
        return 0;
    }

    std::vector<uint32_t> registers(32, 0);
    
    fin.read(reinterpret_cast<char*>(&registers[0]), 32 * sizeof(uint32_t));
    std::vector<uint32_t> registers2 = registers;

    Memory memory;
    memory.load_fragment(fin);
    CacheLRU cache_lru(memory);
    CachePLRU cache_bit_plru(memory);

    run_simulation<CacheLRU>(cache_lru, registers);
    uint32_t pc_res = run_simulation<CachePLRU>(cache_bit_plru, registers2);
    
    cache_lru.flush();
    cache_bit_plru.flush();

    if(!output_name.empty()) {
        std::ofstream fuot(output_name, std::ios::binary);
        fuot.write(reinterpret_cast<char*>(&pc_res), sizeof(uint32_t));
        fuot.write(reinterpret_cast<char*>(&registers[0]), 31 * sizeof(uint32_t));

        fuot.write(reinterpret_cast<char*>(cache_bit_plru.output(start_fragment)), size_fragment);
    }

    print_cache_stats(cache_lru, cache_bit_plru);
    return 0;
}