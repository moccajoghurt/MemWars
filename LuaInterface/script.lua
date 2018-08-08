

useDllInjector = true
useDirect3DInjector = true

if useDllInjector then
    injector = DLLInjector()
    injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll")
    injector:SetTargetProcessByName("notepad.exe")
    if injector:InjectDLL() then
        print("Successfully injected the DLL in the target process")
    end
    print(injector:GetAttackResults())
end

if useDirect3DInjector then
    direct3dinjector = DLLInjector()
    direct3dinjector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/Direct3D11HookAttack/Direct3DHookDLL.dll")
    direct3dinjector:SetTargetProcessByName("Direct3DTestApp.exe")
    if direct3dinjector:InjectDLL() then
        print("Successfully hooked the Direct 3D Function PresentHook()")
    end
    print(direct3dinjector:GetAttackResults())
end