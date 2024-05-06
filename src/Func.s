#this file just for linux/macos
    .section    __TEXT,__text,regular,pure_instructions
    .intel_syntax noprefix
    
    .globl      _func1
    .globl      _printf
    .globl      _printf2
    .globl      _func2
    .p2align    8, 0x90
    
_func2:
#    push    rdx     # for align rsp with 16 bytes
    push    rbp
    mov     rdi, [rsp + 0x18)
    call    _func1
    pop     rbp
#    pop     rdx
    ret

_printf2:
    mov     rdi, [rsp + 0x8)
    mov     rsi, [rsp + 0x10)
    mov     rcx, [rsp + 0x18)
    mov     rdx, [rsp + 0x20)
    lea     rax, [rip + .return]
    push    rax
    xor     rax, rax
    #jmp     _func1
    jmp     _printf
.return:
#    add     rsp, 0x8
    ret

#if 0
#gcc -S -masm=intel t.c
    .section    __TEXT,__text,regular,pure_instructions
    .build_version macos, 14, 0 sdk_version 14, 4
    .intel_syntax noprefix
    .globl  _block                          ## -- Begin function block
    .p2align    4, 0x90
_block:                                 ## @block
    .cfi_startproc
## %bb.0:
    push    rbp
    .cfi_def_cfa_offset 16
    .cfi_offset rbp, -16
    mov rbp, rsp
    .cfi_def_cfa_register rbp
    pop rbp
    ret
    .cfi_endproc
                                        ## -- End function
    .globl  _func1                          ## -- Begin function func1
    .p2align    4, 0x90
_func1:                                 ## @func1
    .cfi_startproc
## %bb.0:
    push    rbp
    .cfi_def_cfa_offset 16
    .cfi_offset rbp, -16
    mov rbp, rsp
    .cfi_def_cfa_register rbp
    sub rsp, 16
    mov dword ptr [rbp - 4], edi
    mov esi, dword ptr [rbp - 4]
    lea rdi, [rip + L_.str]
    mov al, 0
    call    _printf
    add rsp, 16
    pop rbp
    ret
    .cfi_endproc
                                        ## -- End function
    .globl  _test2                          ## -- Begin function test2
    .p2align    4, 0x90
_test2:                                 ## @test2
    .cfi_startproc
## %bb.0:
    push    rbp
    .cfi_def_cfa_offset 16
    .cfi_offset rbp, -16
    mov rbp, rsp
    .cfi_def_cfa_register rbp
    mov qword ptr [rbp - 8], rdi
    mov dword ptr [rbp - 12], esi
    pop rbp
    ret
    .cfi_endproc
                                        ## -- End function
    .section    __TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
    .asciz  "func1, a=%d\n"

.subsections_via_symbols

#endif
