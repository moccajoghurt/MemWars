# Kernel DLL Injector

This method of attack is still under construction. Here is a rough summary of how it works:

- Load a DLL into kernel memory using a vulnerable kernel driver (in this case the Capcom driver)

- Allow access to the kernel memory from the user mode by manipulating the virtual address-pointers of the kernel memory

- Hook the physical memory of ntdll.dll and the function "TlsGetValue" and check if the target process calls the function

- When the target process calls the function, execute the DLL located in the kernel memory in the target process

- Remove the physical hooks and vulnerable drivers right after they have been used to keep the detection vector small
