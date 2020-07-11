;intr.s

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

intr_0:     push r0
            mov r0, 97
            mov *65534, r0
            mov r0, 99
            mov *65534, r0
            mov r0, 13
            mov *65534, r0
            pop r0
            iret

intr_1:     iret

intr_2:     iret

intr_3:     push r0
            mov r0, *65532
            mov *65534, r0
            pop r0
            iret
.end

