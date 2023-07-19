; hi
mov 45,A
mov 45,@r6
mov ARR1,A
mov ARR1,@r6
mov @r3,A
mov @r3,@r6
A: cmp 45,123
cmp 45,A
cmp 45,@r6
cmp ARR1,123
cmp ARR1,A
cmp ARR1,@r6
cmp @r3,123
cmp @r3,A
cmp @r3,@r6
B: add 45,A
add 45,@r6
add ARR1,A
add ARR1,@r6
add @r3,A
add @r3,@r6
C: sub 45,A
sub 45,@r6
sub ARR1,A
sub ARR1,@r6
sub @r3,A
sub @r3,@r6
D: not B
not @r3
clr B
clr @r3
lea ARR2,C
lea ARR2,@r6
inc D
inc @r2
dec D
dec @r2
jmp D
jmp @r2
bne D
bne @r2
red D
red @r2
prn -13
prn E
prn @r2
jsr E
jsr @r2
rts
stop
ARR1: .data 12,13,14,15,16
ARR2: .string "Example String"
.extern E
.entry A

