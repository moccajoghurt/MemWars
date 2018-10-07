

print("#### Testing DLL Injection")
injector = DLLInjector()
-- injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll")
injector:SetTargetDLL("C:/Users/Marius/Desktop/ESO Hack/FW1FontWrapper.dll")
injector:SetTargetProcessByName("eso64.exe")
if injector:InjectDLL() then
    print("Successfully injected the DLL in the target process")
end
injector:SetTargetDLL("C:/Users/Marius/Desktop/ESO Hack/EsoDLL.dll")
injector:SetTargetProcessByName("eso64.exe")
if injector:InjectDLL() then
    print("Successfully injected the DLL in the target process")
end
print(injector:GetAttackResults())