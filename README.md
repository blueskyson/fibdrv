# fibdrv

Linux kernel module that creates device /dev/fibonacci.  Writing to this device
should have no effect, however reading at offset k should return the kth
fibonacci number.

## Run Test

Environment: Ubuntu with Linux kernel version >= 5.4.0. For example:

```
uname -r
6.8.0-38-generic
```

Install linux headers and tools:

```bash
sudo apt install linux-headers-`uname -r`
sudo apt install util-linux strace gnuplot-nox
```

Since Linux kernel version 4.4, Ubuntu Linux has enabled `EFI_SECURE_BOOT_SIG_ENFORCE` by default. This requires kernel modules to have appropriate signatures to be loaded into the Linux kernel. For the convenience of subsequent testing, we need to disable the UEFI Secure Boot feature. Please refer to [Why do I get 'Required key not available' when installing 3rd party kernel modules or after a kernel upgrade?](https://askubuntu.com/questions/762254/why-do-i-get-required-key-not-available-when-install-3rd-party-kernel-modules)

Compile and Test Functionality:

```bash
make check
```

You will see `Passed [-]` if it works properly.

## References
* [The Linux Kernel Module Programming Guide](https://sysprog21.github.io/lkmpg/)
* [Writing a simple device driver](https://www.apriorit.com/dev-blog/195-simple-driver-for-linux-os)
* [Character device drivers](https://linux-kernel-labs.github.io/refs/heads/master/labs/device_drivers.html)
* [cdev interface](https://lwn.net/Articles/195805/)
* [Character device files](https://sysplay.in/blog/linux-device-drivers/2013/06/character-device-files-creation-operations/)

## License

`fibdrv`is released under the MIT license. Use of this source code is governed by
a MIT-style license that can be found in the LICENSE file.

External source code:
* [git-good-commit](https://github.com/tommarshall/git-good-commit): MIT License
