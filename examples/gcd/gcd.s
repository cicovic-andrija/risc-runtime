; gcd.s - Greatest Common Divisor (GCD)

.text

.global print_hex

.global gcd
gcd:
            push r1
            push r2

            mov r1, r6[6]
            mov r2, r6[8]

loop:       cmp r2, r1
            jmpeq stop
            jmpgt less
            sub r1, r2
            jmp loop
less:       sub r2, r1
            jmp loop
stop:       mov r0, r1

            pop r2
            pop r1
            ret

.end

