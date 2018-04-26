.code
 
SpinLockByte proc
SpinLock:
	pause ; tells the CPU we're spinning
	cmp dl, [rcx]
	jnz SpinLock
	ret
SpinLockByte endp
 
end