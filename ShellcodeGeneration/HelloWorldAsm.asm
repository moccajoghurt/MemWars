;---ASM Hello World Win64 MessageBox

extrn MessageBoxA: PROC
extrn ExitProcess: PROC

.data
mytitle db 'Win64', 0
msg db 'Hello World!', 0

.code
main proc
    sub rsp, 28h  
    mov rcx, 0       ; hWnd = HWND_DESKTOP
    lea rdx, msg     ; LPCSTR lpText
    lea r8,  mytitle   ; LPCSTR lpCaption
    mov r9d, 0       ; uType = MB_OK
    call MessageBoxA
    add rsp, 28h  
    mov ecx, eax     ; uExitCode = MessageBox(...)
    call ExitProcess
main endp

End