# Linux Kernel Programming
This repository contains a basic introduction to Linux kernel programming.  It
focuses on using an embedded device such as a Raspberry Pi, but a Linux VM can
also be used.

A few simple example Loadable Kernel Modules (LKMs) are presented for learning
purposes.

The first example is a straightforward "Hello World" module that can be used to
make sure your setup for LKM development is working.


## Intro
A [loadable kernel module](https://en.wikipedia.org/wiki/Loadable_kernel_module)
(or LKM) is an object file that contains code to extend the running kernel, or
so-called base kernel, of an operating system. LKMs are typically used to add
support for new hardware (as device drivers) and/or filesystems, or for adding
system calls. When the functionality provided by a LKM is no longer required,
it can be unloaded in order to free memory and other resources.

Without this modular capability of extending the kernel at run time, the Linux
kernel would be huge because it would need to support every driver in existence
out of the box.

Even though LKMs are loaded at run time, they are executed in kernel space and not
in user space.  The OS treats the memory available in kernel space as completely
separate from the memory available in user space.  User space applications can
access kernel services in a controlled way by making system calls via the Linux
C library.

Kernel modules are written in the C programming language, but they are not are
a lot different from *normal* user-space C programs.  Some of the key differences
include:

* LKMs do not have a **main()** function
* LKMs do not execute sequentially.  Similar to event-driven programming
* Any resources that are allocated to the module must be manually released when
the module is unloaded (no automatic cleanup)
* Do not have **printf()** and similar functions.  Though there is a **printk()**
* LKMs must have a consistent and valid behavior when they are interrupted
* LKMs have a higher level of execution privilege
* LKMs do not have floating-point support

> It is very easy to crash the OS when you are writing and testing LKMs.  It is
possible that you you can crash it in a way that could corrupt your file system
and make it unbootable.
I strongly recommend you use of of two basic setups for
learning kernel development:
1. Use a Linux virtual machine (VM) that you have taken a snapshot of
  * That way you can revert to the snapshot if it gets hosed
2. Use an embedded Linux system which boots of an SD card such as a Raspberry Pi
  * Then if unbootable you can put the SD card in a card reader and fix the file
  system

## Prerequisites
In order to build LKMs, the Linux kernel headers need to be installed on your build machine.  The Linux kernel headers are C header files that define the
interfaces between the different kernel modules, the kernel, and user space.  The header files present must be the exact same version as the kernel for which you want to build a module.

### Installing the Linux Kernel headers
On a Debian, Ubuntu, or Mint Linux OS, you can install the kernel headers like so:

```bash
sudo apt install linux-headers-$(uname -r)
```

This should install the headers in */lib/modules/$(uname -r)/build/* which should likely be a symbolic link to the location */usr/src/linux/$(uname -r)/*.

## Building the LKM
Once the Linux kernel headers are in place, you can build the *hello_world* LKM
using the Makefile by typing ``make``:

```bash
cd hello_world
make
```

This creates the LKM with the name **hello_world.ko**.  Note that this LKM can
only be executed on a machine running the exact same kernel version.

## Testing the LKM
The "Hello World" LKM can be tested by loading it into the kernel.  This requires superuser permission.  The LKM can be loading using the ``insmod`` program to insert a module into the Linux kernel:

```bash
cd hello_world
sudo insmod hello_world.ko
```

You can list loaded kernel modules with the ``lsmod`` command:

```bash
lsmod | grep hello
```

You can get information about a LKM file using the ``modinfo`` command, which identifies the description, author, and any module parameters that are defined by the LKM souce code:

```bash
/sbin/modinfo hello_world.ko
```

The module can be removed from the kernel using the ``rmmod`` program:

```bash
sudo rmmod hello_world
```

You can view the output live in the kernel log as a result of the use of
the ``printk()`` function.  I recommend that you use a second terminal window
and view the live output as your LKM is loaded and unloaded, as follows:

```bash
sudo tail -f /var/log/kern.log
```

### Testing the LKM Parameter
The **hello_world.ko** LKM contains a custom paramter that can be set when the module is being loaded, for example:

```bash
sudo insmod hello_world.ko name=Todd
```

If you view */var/log/kern.log* at this point, the message "Hello Todd" appears
in place of "Hello World".

You can also see information about the kernel module that is loaded within the */proc* virtual files system:

```bash
ct /proc/modules | grep hello
```
This is the same info as provided by the ``lsmod`` command, but it also provides the current kernel memory offset for the loaded module, which is useful for debugging.

The */sys* virtual file system provides you with direct access to the custom parameter state as well as other info such as version, whehter it is tainted or not, etc.:

```bash
pushd /sys/module/hello_world
cat version
cat taint
cd parameters
cat name
popd
```

> It is important that you leave any virtual filesystem directory associated with the LKM before you unload it, otherwise you can cause a kernel panic with something as simple as a call to ``ls``.

## Attribution
Much of the material here comes from the excellent book
[Exploring Raspberry PI](http://exploringrpi.com/) by Derek Molloy.  If you want
to learn more, I recommend you use that book as a reference to get started.
[Chapter 16](http://exploringrpi.com/chapter16/) covers Kernel Programming.

A more detailed book specific to Linux kernel programming is
[Linux Kernel Development, 3rd Ed](https://www.amazon.com/Linux-Kernel-Development-Robert-Love/dp/0672329468) by Robert Love.
