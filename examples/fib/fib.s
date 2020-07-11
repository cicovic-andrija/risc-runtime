; fib.s - fibonacci calculator, recursive

.text

.global fib
fib:
                push r1
                push r2

                mov r1, r6[6]
                cmp r1, 0
                jmpgt $valid
                mov r0, 0
                jmp $return

valid:          cmp r1, 2
                jmpgt $rec
                mov r0, 1
                jmp $return
rec:
                sub r1, 1
                push r1
                call fib
                mov r2, r0
                add r6, 2

                sub r1, 1
                push r1
                call fib
                add r0, r2
                add r6, 2

return:         pop r2
                pop r1
                ret

.end

