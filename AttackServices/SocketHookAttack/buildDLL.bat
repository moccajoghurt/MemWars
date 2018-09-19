@ECHO OFF
cl.exe /EHsc /LD SocketHookDLL.cpp /link /LTCG /LIBPATH:"C:\Users\marius\git\MemWars\libs\Capstone\msvc\x64\Release" user32.lib Ws2_32.lib