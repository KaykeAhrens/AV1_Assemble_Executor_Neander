; Programa de teste: decrementa contador ate zero usando JZ e JMP
ORG 0
    LDA COUNT
LOOP: ADD NEG_ONE
    STA COUNT
    JZ END
    JMP LOOP
END: HLT

COUNT: DATA 3
NEG_ONE: DATA 255
