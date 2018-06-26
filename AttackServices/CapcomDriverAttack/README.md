# Capcom Driver Attack

This attack installs the vulnerable Capcom driver.

After the driver is installed, user mode functions can be executed in kernel mode. In this case, any process is opened from kernel mode and the corresponding HANDLE is returned.
