; print.s - print library

.rodata

chr_a:          .word 97
chr_0:          .word 48

.text

.global print_hex
print_hex:
                push r1
                push r2

                mov r1, r6[6]
                mov r2, 12

loop:           mov r1, r6[6]
                shr r1, r2
                and r1, 15
                cmp r1, 9
                jmpgt $letter
                add r1, chr_0
                jmp out
letter:         sub r1, 10
                add r1, chr_a
out:            mov *65534, r1
                sub r2, 4
                jmpgt $loop
                jmpeq $loop

                pop r2
                pop r1
                ret

.global println
println:
                push r1
                mov r1, 13
                mov *65534, r1
                pop r1
                ret

.global prints
prints:
                push r1
                push r2
                mov r1, r6[6]
putchar:        mov r2, r1[0]
                and r2, 255
                jmpeq $return
                mov *65534, r2
                add r1, 1
                jmp $putchar
return:         pop r2
                pop r1
                ret

.end

