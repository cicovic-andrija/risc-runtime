; main.s - Print 0-9th Fibonacci number.

.text

.global fib
.global error_msg
.global input_msg
.global output_msg
.global N
.global read
.global write
.global print_hex
.global println
.global prints

.global START
START:
loop:           mov r1, read
                cmp r1, 0
                jmpeq $loop

                mov r1, N
                mov r2, 0
                mov read, r2
                mov r2, 1
                mov write, r2

                cmp r1, 57
                jmpgt $skip
                cmp r1, 47
                jmpgt $calc
                jmp $skip
calc:
                push &input_msg
                call $prints
                add r6, 2

                sub r1, 48
                push r1
                call $print_hex
                add r6, 2
                call $println

                push &output_msg
                call $prints
                add r6, 2

                push r1
                call $fib
                add r6, 2
                push r0
                call $print_hex
                add r6, 2
                call $println
                jmp $loop

skip:           push &error_msg
                call $prints
                add r6, 2

                cmp r1, 113
                jmpeq $quit
                jmp $loop

quit:           halt

.end

