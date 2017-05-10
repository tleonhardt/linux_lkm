# Character Device Drivers
A *character device* typically transfers data to and from a user application —
they behave like pipes or serial ports, reading or writing the byte data in a
character-by-character stream.  They provide the framework for many typical
drivers, such as those that are required for interfacing to serial communications,
video capture, and audio devices. The main alternative to a character device is
a *block device*. Block devices behave in a similar fashion to regular files,
allowing a buffered array of cached data to be viewed or manipulated with
operations such as reads, writes, and seeks. Both device types can be accessed
through device files that are attached to the file system tree. For example, the
program code that is presented in this article builds to become a device
``/dev/tdlchar``, which appears on your Linux system under the **/dev** directory:

```bash
lsmod | grep tdl
ls -hal /dev/tdl*
```

## Purpose of this example
The main purpose of this example is to demonstrate how a kernel-space LKM (**tdlchar**)
can communicate with a user-space C/C++ application (**testtdlchar.c**).

In this example, a C user-space application sends a string to the LKM. The LKM
then responds with the message that was sent, but with everything converted to
uppercase.


## Where to learn more
Derek Molloy has an excellent blog post [Writing a Linux Kernel Module — Part 2: A Character Device](http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/)
The example here is very closely based on this blog post.

The [Linux Kernel Module Programming Guide](http://www.tldp.org/LDP/lkmpg/2.6/html/index.html)
has a chapter on [Character Device Drivers](http://www.tldp.org/LDP/lkmpg/2.6/html/x569.html).
This provides detailed coverage.
