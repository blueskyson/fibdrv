# fibdrv

`Fibdrv` is a Linux kernel module that calculates the Fibonacci sequence. It is an assignment of *CSIE3018 - Linux Kernel Internals* by [Ching-Chun (Jim) Huang](https://www.csie.ncku.edu.tw/en/members/5) at National Cheng Kung University. In this assignment, I made `fibdrv` theoretically possible to calculate $F_{188795}$ and, using the relationship between the Fibonacci sequence and the **golden ratio**, made memory allocation deterministic. Additionally, I used **Fast Doubling** to reduce the amount of computations, and compared the computation times of the **Karatsuba** and **Schönhage–Strassen** algorithms.

Programs usually use previous generated values as the basis for the next calculation when computing random numbers. See [Pseudo-Random Number Generators](https://en.wikipedia.org/wiki/Pseudorandom_number_generator) (PRNG). Fibonacci, after modifying the initial value or performing mod calculations, can produce numbers that are not easily predictable. This method of generating random numbers is called [Lagged Fibonacci generator](https://en.wikipedia.org/wiki/Lagged_Fibonacci_generator) (LFG). See also: [For The Love of Computing: The Lagged Fibonacci Generator — Where Nature Meet Random Numbers](https://medium.com/asecuritysite-when-bob-met-alice/for-the-love-of-computing-the-lagged-fibonacci-generator-where-nature-meet-random-numbers-f9fb5bd6c237).

The expected goals are:
- Writing programs for the Linux kernel level
- Learning core APIs such as `ktimer` and `copy_to_user`
- Reviewing C language numerical systems and bitwise operations
- improving numerical analysis and computation strategies
- Exploring Linux VFS
- Implementing automatic testing mechanisms
- Conducting performance analysis through `perf`

Homework instructions (in Chinese): [https://hackmd.io/@sysprog/linux2022-fibdrv](https://hackmd.io/@sysprog/linux2022-fibdrv)  
Old version dev blog: [https://jacklinweb.github.io/posts/fibdrv/](https://jacklinweb.github.io/posts/fibdrv/)

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

You will see `Passed [-]` if it works properly. and The result will be dumped in `out` file.

```
cat out
Reading from /dev/fibonacci at offset 0, returned the sequence 0.
Reading from /dev/fibonacci at offset 1, returned the sequence 1.
Reading from /dev/fibonacci at offset 2, returned the sequence 1.
Reading from /dev/fibonacci at offset 3, returned the sequence 2.
... (skip)
Reading from /dev/fibonacci at offset 300, returned the sequence 222232244629420445529739893461909967206666939096499764990979600.
```

To change algorighm, please uncomment one library in [fibdrv.c](./fibdrv.c).

```c
/** 
 * Only include one calculation method at a time.
 * Method 1: Use unsigned long long array to store big number. 
 * Method 2: Introduce fast-doubling.
 * Method 3: Optimize multiplication using Schonhange Strassen.
 * Method 4: Optimize multiplication using Karatsuba.
 */
// #include "lib/adding.h"
// #include "lib/fast_doubling.h"
// #include "lib/schonhange_strassen.h"
#include "lib/karatsuba.h"
```

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
