# An LKM Makefile
#
# A Makefile is required to build a loadable kernel module (LKM).  In fact, it is a special kbuild
# Makefile.
#
# Warning: Makefiles require Tab characters in front of rules (in front of the calls to "make" below.
#

# goal definition, defines the module to be built (tdlchar.o)
# obj-m defines a loadable module goal, whereas obj-y indicates a built-in object goal
obj-m+=tdlchar.o

# The remaineder of this Makefile is similar to a regular Makefile

# The "$(shell uname -r)" is a useful call to return the current kernel build version
# The -C option switches the directory to the kernel direcotry before performing any make tasks
# The "M=$(PWD)" variable assignment tells the make command where the actual project files exist

# The "modules" target is the default target for external kernel modules.
# An alternative target is "modules_install", which would install the module (needs sudo)

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) testtdlchar.c -o test
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm test
