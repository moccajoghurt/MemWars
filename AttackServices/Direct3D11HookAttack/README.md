# Direct3D 11 Hook Attack



This attack is based on a DLL injection to hook the Direct3D functions of a process.



First, within the DLL [D3D11CreateDeviceAndSwapChain](https://msdn.microsoft.com/de-de/library/windows/desktop/ff476083%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396) creates a Direct3D device. 

This device has a virtual address table. 

The game process already has its own Direct3D device, which is why our device now shares the virtual address table with this device. 


So we can now change the pointers in the virtual address table of the game and thus hook popular Direct3D functions.



In this attack the Direct3D method "Present()" is hooked.



The library[PolyHook](https://github.com/stevemk14ebr/PolyHook) is used to change the pointers in the Virtual Address Table.

Translated with www.DeepL.com/Translator