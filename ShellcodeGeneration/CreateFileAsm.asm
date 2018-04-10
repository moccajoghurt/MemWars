;---ASM Hello World Win64 MessageBox

extrn CreateFileA: PROC
extrn ExitProcess: PROC

.data
mytitle db 'Win64', 0
msg db 'testfile1234567.txt', 0

.code
main proc

    mov rdx, 40000000h
    lea rcx, msg
    mov rdx, 40000000h
    xor r8d,  r8d
    xor r9d, r9d

    push 0
    push 80h
    push 1

    mov rax, CreateFileA
    sub rsp, 20h
    call rax
    add rsp, 20h
    mov ecx, eax
    call ExitProcess
main endp

End