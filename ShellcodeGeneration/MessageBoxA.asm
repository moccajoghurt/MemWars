;---ASM Hello World Win64 MessageBox

extrn MessageBoxA: PROC
extrn ExitProcess: PROC

.data
mytitle db 'Win64', 0
;msg db 'Hello World!1337', 0

.code
main proc
    sub rsp, 28h  
    xor rcx, rcx        ; hWnd = NULL
    xor rdx, rdx        ; LPCSTR lpText
    xor r8, r8          ; LPCSTR lpCaption
    xor r9d, r9d        ; uType = MB_OK
    mov rax, MessageBoxA
    call rax

    add rsp, 28h  
    mov ecx, eax     ; uExitCode = MessageBox(...)
    call ExitProcess
main endp

End