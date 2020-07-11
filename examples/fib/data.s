; data.s - storage

.rodata

.global error_msg
error_msg:      .char 78, 97, 78, 13, 00 ; "NaN\n"

.global input_msg
input_msg:      .char 32, 105, 110, 58, 32, 00 ; " in: "

.global output_msg
output_msg:     .char 102, 105, 98, 58, 32, 00 ; "fib: "

.data

.global read
read:           .word 0

.global write
write:          .word 1

.bss

.global N
N:              .word

.end

