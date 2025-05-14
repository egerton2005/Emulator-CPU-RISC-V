.globl _start

_start:
    li x3, 7            # x3 = счётчик циклов

    addi x5, x0, 0   
    lw x4, 32(x5)     
    addi x5, x5, 512   
    lw x4, 32(x5)
    addi x5, x5, 512
    lw x4, 32(x5)
    addi x5, x5, 512
    lw x4, 32(x5)
    addi x5, x5, 512
    lw x4, 32(x5)

    lw x4, 32(x5)
    lw x4, 32(x5)
    lw x4, 32(x5)
    lw x4, 32(x5)
    lw x4, 32(x5)

loop:
    addi x3, x3, -1
    beq x3, x2, exit
    j loop   

exit:
    ecall