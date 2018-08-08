

useDllInjector = true
useDirect3DInjector = true

if useDllInjector then
    injector = DLLInjector()
    injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll")
    injector:SetTargetProcessByName("notepad.exe")
    if injector:InjectDLL() then
        print("success")
    end
    print(injector:GetAttackResults())
end

