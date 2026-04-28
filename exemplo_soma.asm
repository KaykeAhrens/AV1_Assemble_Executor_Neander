; Programa de teste: soma dois valores e grava em RESULT
ORG 0
    LDA A
    ADD B
    STA RESULT
    HLT

A: DATA 7
B: DATA 5
RESULT: SPACE 1
