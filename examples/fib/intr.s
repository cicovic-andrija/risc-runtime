; intr.s - interrupt routines

.data       ; interrupt vector table

.word       intr_0 ; entry 0
.word       intr_1 ; entry 1
.word       intr_2 ; entry 2
.word       intr_3 ; entry 3
.word       0      ; entry 4
.word       0      ; entry 5
.word       0      ; entry 6
.word       0      ; entry 7

.text       ; interrupt routines

intr_0:     iret

intr_1:     iret

intr_2:     iret

.global N
.global read
.global write

intr_3:     push r0
            mov r0, write
            cmp r0, 0
            jmpeq $skip
            mov r0, *65532
            mov N, r0
            mov r0, 0
            mov write, r0
            mov r0, 1
            mov read, r0
            pop r0
skip:       iret

.end

