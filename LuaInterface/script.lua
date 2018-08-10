

useDllInjector = true
useDirect3DInjector = true
useIATHookInjector = true
useThreadHijacker = true

targetProcessName = "memoryTestApp.exe"

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
        print("Successfully hijacked the main thread of the target process")
    end
    print(threadHijacker:GetAttackResults())
end


