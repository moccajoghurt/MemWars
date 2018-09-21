local colors = require("ConsoleColors")

function InjectDLL(dllPath, targetProcessName, requireConfirmationFile, timeout)
    injector = DLLInjector()
    injector:SetTargetDLL(dllPath)
    injector:SetTargetProcessByName(targetProcessName)
    injector:SetTimeout(timeout)
    if requireConfirmationFile then
        injector:RequireConfirmationFile()
    end
    local success = injector:InjectDLL()
    return success, injector:GetAttackResults()
end

function HijackThread()
    threadHijacker = ThreadHijacker()
    threadHijacker:SetTargetProcessByName(targetProcessName)
    threadHijacker:SetTimeout(10000)
    local success = threadHijacker:HijackThread()
    return success, threadHijacker:GetAttackResults()
end

function LsassAttack()
    lsassInstaller = LsassAttackInstaller()
    if lsassInstaller:Install() == false then
        return false, lsassInstaller:GetAttackResults()
    end

    lsassAttackClient = LsassAttackClient()
    if lsassAttackClient:SetTargetProcessByName(targetProcessName) == false then
        return false, lsassAttackClient:GetAttackResults()
    end
    local success = lsassAttackClient:StartAttack()
    return success, lsassAttackClient:GetAttackResults()
end

function HypervisorActivated()
    local script = [[if ($hyperv.State -eq \"Enabled\") {Write-Host "True"}else{Write-Host "False"}]]
    local pipe = io.popen("powershell -command " .. script)
    local result = pipe:read("*a")
    pipe:write("exit")
    pipe:close()
    if result == "False\n" then
        return false
    else
        return true
    end
end

function KernelDLLInject(dllPath)
    kernelInjector = HiddenKernelDLLInjector()
    kernelInjector:SetTargetDLL(dllPath)
    if kernelInjector:LoadDLLIntoKernel() == false then
        return false, kernelInjector:GetAttackResults()
    end
    -- After the DLL has been mapped into the kernel, the anti-cheat protected process can be started
    -- this ensures that the capcom driver cannot be detected since it has been unloaded
    -- start target process here
    local success = kernelInjector:InjectDLLIntoProcess(targetProcessName)
    return success, kernelInjector:GetAttackResults()
end

header = 
'===========================================\n' ..
'| MemWars Penetration Testing Framework   |\n' ..
'| https://github.com/moccajoghurt/MemWars |\n' ..
'===========================================\n'

colors.SetConsoleColor(colors.brightwhite)
io.write(header)
colors.SetConsoleColor(colors.black)
io.write("Enter the target process name: ")
targetProcessName = io.read()

localPath = "C:/Users/Marius/git/MemWars/AttackServices/"

colors.SetConsoleColor(colors.cyan)
io.write("\n[+] Testing for basic DLL-Injection...\n")
success, results = InjectDLL(localPath .. "DLLInjectionAttack/InjectedDLL.dll", targetProcessName, false, 10000)

if success then
    colors.SetConsoleColor(colors.brightgreen)
    print(results)

    colors.SetConsoleColor(colors.cyan)
    io.write("[+] Testing for Direct 3D 11 DLL-Injection...\n")
    success, results = InjectDLL(localPath .. "Direct3D11HookAttack/Direct3DHookDLL.dll", targetProcessName, true, 10000)
    if success then
        colors.SetConsoleColor(colors.brightgreen)
        io.write("[+] Successfully hooked the Direct 3D Function PresentHook()\n\n")
    else
        colors.SetConsoleColor(colors.brightred)
        io.write("[-] Could not hook the Direct 3D Function PresentHook()\n\n")
    end

    colors.SetConsoleColor(colors.cyan)
    io.write("[+] Testing for IAT Hook DLL-Injection...\n")
    success, results = InjectDLL(localPath .. "IATHookAttack/IATHookDLL.dll", targetProcessName, true, 10000)
    if success then
        colors.SetConsoleColor(colors.brightgreen)
        io.write("[+] Successfully hooked the GetCurrentThreadId() Function via IAT\n\n")
    else
        colors.SetConsoleColor(colors.brightred)
        io.write("[-] Could not hook the IAT of " .. targetProcessName .. "\n\n")
    end

    colors.SetConsoleColor(colors.cyan)
    io.write("[+] Testing for Socket Hook DLL-Injection...\n")
    success, results = InjectDLL(localPath .. "SocketHookAttack/SocketHookDLL.dll", targetProcessName, true, 10000)
    if success then
        colors.SetConsoleColor(colors.brightgreen)
        io.write("[+] Successfully read network packets of " .. targetProcessName .. "\n\n")
    else
        colors.SetConsoleColor(colors.brightred)
        io.write("[-] Could not read network packets of " .. targetProcessName .. "\n\n")
    end

else
    colors.SetConsoleColor(colors.brightred)
    print(results)
end


colors.SetConsoleColor(colors.cyan)
io.write("[+] Testing for Thread hijacking...\n")
success, results = HijackThread()
if success then
    colors.SetConsoleColor(colors.brightgreen)
    print(results)
else
    colors.SetConsoleColor(colors.brightred)
    print(results)
end

colors.SetConsoleColor(colors.cyan)
io.write("[+] Testing for Lsass attack...\n")
colors.SetConsoleColor(colors.black)
io.write("[+] Note: Installing the Lsass-Attack can take up to 3 minutes since we hijack an idling thread\n")
success, results = LsassAttack()
if success then
    colors.SetConsoleColor(colors.brightgreen)
    print(results)
else
    colors.SetConsoleColor(colors.brightred)
    print(results)
end

if HypervisorActivated() then
    colors.SetConsoleColor(colors.red)
    io.write("[-] Cannot test for Hidden Kernel DLL Injection since Hyperisor is activated.\n")
    io.write("[-] Deactivate the Hyper-V Hypervisor Service if you want to test for this attack.\n")
    colors.SetConsoleColor(colors.white)
    return
end

colors.SetConsoleColor(colors.cyan)
io.write("[+] Testing for Hidden Kernel DLL Injection...\n")
success, results = KernelDLLInject(localPath .. "DLLInjectionAttack/InjectedDLL.dll")
if success then
    colors.SetConsoleColor(colors.brightgreen)
    print(results)
else
    colors.SetConsoleColor(colors.brightred)
    print(results)
end

colors.SetConsoleColor(colors.white)