


print(">> Testing DLL Injection")
injector = DLLInjector()
injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/SocketHookAttack/SocketHookDLL.dll")
-- injector:SetTargetDLL("C:/Users/Marius/git/MemWars/AttackServices/NetworkEncryptionDetector/DetectEncryptionDLL.dll")
injector:SetTargetProcessByName("SocketTestApp.exe")
injector:RequireConfirmationFile()
injector:SetTimeout(1000)
if injector:InjectDLL() then
    print("Successfully injected the DLL in the target process")
end
print(injector:GetAttackResults())