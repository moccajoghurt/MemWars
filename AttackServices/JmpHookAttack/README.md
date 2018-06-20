# JMP Hook Attack



This attack is based on a DLL injection.



The DLL has the function "FunctionToBeHooked()", which is executed by "FunctionCaller()".



First the memory of the DLL is read to find the function call of "FunctionToBeHooked()".



A trampoline is then created which first calls the "HookingFunc()" function and then the "FunctionToBeHooked()" function.



Now the function call of "FunctionToBeHooked()" in memory is replaced with the trampoline, so we have executed a JMP hook within our own DLL.