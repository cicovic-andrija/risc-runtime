; main.s - Greatest Common Divisor.

.data
M:              .word 36
N:              .word 27

.text

.global gcd
.global print_hex
.global println

.global START
START:
                push N
                push M
                call gcd
                add r6, 4
                push r0
                call print_hex
                add r6, 2
                call println
                halt
.end

