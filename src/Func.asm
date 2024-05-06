;just for windows masm
;
EXTERN func1:PROC
.DATA

.CODE


func2  PROC
;    push    rdx     # for align rsp with 16 bytes
    push    rbp
    mov     rdi, [rsp + 18h ]
    call    func1
    pop     rbp
;    pop     rdx
	ret
func2  ENDP


printf2  PROC
    ret
printf2  ENDP

END
