
useDllInjector = true
useDirect3DInjector = false
useIATHookInjector = false
useThreadHijacker = true
useLsassAttack = false
useKernelDLLInector = false

targetProcessName = "LoaderLock.exe"

if useDllInjector then
    print("#### Testing DLL Injection")
    injector = DLLInjector()
    injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll")
    injector:SetTargetProcessByName(targetProcessName)
    if injector:InjectDLL() then
        print("Successfully injected the DLL in the target process")
    end
    print(injector:GetAttackResults())
end

if useDirect3DInjector then
    print("#### Testing Direct3D 11 DLL Injection")
    direct3dinjector = DLLInjector()
    direct3dinjector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/Direct3D11HookAttack/Direct3DHookDLL.dll")
    direct3dinjector:SetTargetProcessByName(targetProcessName)
    if direct3dinjector:InjectDLL() then
        print("Successfully hooked the Direct 3D Function PresentHook()")
    end
    print(direct3dinjector:GetAttackResults())
end

if useIATHookInjector then
    print("#### Testing IAT Hook Injection")
    IATHookInjector = DLLInjector()
    IATHookInjector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/IATHookAttack/IATHookDLL.dll")
    IATHookInjector:SetTargetProcessByName(targetProcessName)
    if IATHookInjector:InjectDLL() then
        print("Successfully hooked the GetCurrentThreadId() Function")
    end
    print(IATHookInjector:GetAttackResults())
end

if useThreadHijacker then
    print("#### Testing thread hijacking")
    threadHijacker = ThreadHijacker()
    threadHijacker:SetTargetProcessByName(targetProcessName)
    if threadHijacker:HijackThread() then
        print("Successfully hijacked the main thread of " .. targetProcessName)
    end
    print(threadHijacker:GetAttackResults())
end

function toLittleEndian(hexString)
    hexString = hexString:gsub("0x","", 1)
    local buf = ""
    for i = #hexString, 1, -1 do
        local c = hexString:sub(i,i)
        buf = buf .. c
    end
    buf = buf:gsub("(.)(.)","%2%1")
    return buf
end

if useLsassAttack then
    print("#### Installing lsass attack \n(this can take a few mintues since we wait for idling threads in lsass.exe)...")
    lsassInstaller = LsassAttackInstaller()
    if lsassInstaller:Install() then
        print("Successfully injected shellcode into lsass.exe and prepared communication via file mapping")
    end
    print(lsassInstaller:GetAttackResults())
    print("starting client...")
    lsassAttackClient = LsassAttackClient()
    if lsassAttackClient:SetTargetProcessByName(targetProcessName) then
        print("Successfully setup communication with lsass.exe")
    end
    
    readVal = lsassAttackClient:ReadProcessMemory("0x1c921120000", 1)
    print(readVal)
    lsassAttackClient:WriteProcessMemory("0x1c921130000", toLittleEndian("0xBE"))
    readVal = lsassAttackClient:ReadProcessMemory("0x1c921130000", 1)
    print(readVal)
    print(lsassAttackClient:GetAttackResults())
end

if useKernelDLLInector then
    print("### Testing Hidden Kernel DLL Injection")
    kernelInjector = HiddenKernelDLLInjector()
    kernelInjector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll")
    if kernelInjector:LoadDLLIntoKernel() then
        print ("Successfully loaded DLL into kernel")
    end
    -- After the DLL has been mapped into the kernel, the anti-cheat protected procress can be started
    -- this ensures that the capcom driver cannot be detected since it has been unloaded
    -- start target process here
    if kernelInjector:InjectDLLIntoProcess(targetProcessName) then
        print ("Successfully injected DLL into process")
    end
    print(kernelInjector:GetAttackResults())
end