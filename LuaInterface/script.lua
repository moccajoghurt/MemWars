

injector = DLLInjector()
injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/DLLInjectionAttack/InjectedDLL.dll")
injector:SetTargetProcessByName("chrome.exe")
if injector:ExecuteAttack() then
    print("success")
end
print(injector:GetAttackResults())