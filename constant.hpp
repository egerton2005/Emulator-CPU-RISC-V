#pragma once

#include <cstring> 
#include <iostream>  
#include <fstream> 
#include <new>    

constexpr inline int MEMORY_SIZE = 512 * 1024 * 1024;
constexpr inline int ADDRESS_LEN = 19;
// Конфигурация кэша look-through write-back write-allocate
// Политика вытеснения кэша LRU и bit-pLRU
constexpr inline int CACHE_WAY = 4;
constexpr inline int CACHE_TAG_LEN = 10;
constexpr inline int CACHE_INDEX_LEN = 4;
constexpr inline int CACHE_OFFSET_LEN = 5;
constexpr inline int CACHE_SIZE = 2 * 1024;
constexpr inline int CACHE_LINE_SIZE = 32;
constexpr inline int CACHE_LINE_COUNT = 64;
constexpr inline int CACHE_SET_COUNT = 16;


constexpr inline int SIZE_RD = 5;
constexpr inline int SIZE_FUNC3 = 3;
constexpr inline int SIZE_RS1 = 5;
constexpr inline int SIZE_RS2 = 5;
constexpr inline int SIZE_FUNC7 = 7;


enum FMT {
    R = 0b0110011,
    I = 0b0010011,
    II = 0b0000011,
    S = 0b0100011,
    B = 0b1100011,
    J = 0b1101111,
    III = 0b1100111,
    U = 0b0110111, 
    UU = 0b0010111,
    I_END = 0b1110011,
    FENCE = 0b0001111
}; // lui, auipc, fence

enum FMT_I {
    addi = 0x0,
    xori = 0x4,
    ori = 0x6,
    andi = 0x7,
    slli = 0x1,
    srli = 0x5,
    srai = 0x5, 
    slti = 0x2, 
    sltiu = 0x3, 
    lb = 0x0, 
    lh = 0x1, 
    lw = 0x2, 
    lbu = 0x4, 
    lhu = 0x5, 
    jalr = 0x0, 
    ecall = 0x0, 
    ebreak = 0x0
}; //jal - J

enum FMT_S {
    sb = 0x0,
    sh = 0x1,
    sw = 0x2,
};

enum FMT_B {
    beq = 0x0,
    bne = 0x1,
    blt = 0x4,
    bge = 0x5,
    bltu = 0x6,
    bgeu = 0x7
};

enum FMT_R {
    add = 0x0,
    sub = 0x0,
    xorr = 0x4,
    orr = 0x6,
    andd = 0x7,
    sll = 0x1,
    srl = 0x5, 
    sra = 0x5, 
    slt = 0x2, 
    sltu = 0x3, 
    mul = 0x0, 
    mulh = 0x1, 
    mulhsu = 0x2, 
    mulhu = 0x3, 
    divv = 0x4, 
    divu = 0x5, 
    rem = 0x6, 
    remu = 0x7, 
};