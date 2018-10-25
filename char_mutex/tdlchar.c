/**
 * @file   tdlchar.c
 * @author Todd Leonhardt
 * @date   10 May 2017
 * @version 1.0
 * @brief   An introductory character driver Linux loadable kernel module (LDM)
 * This module maps to /dev/tdlchar and comes with a helper C program that can be
 * run in Linux user space to communicate with this the LKM.
 *
 * This code is based on the tdlchar.c code written by Derek Molloy
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
 */

// FIX For Synchronization problem:
// --------
// The original code wasn't process or thread safe.  This code adds a mutex to fix the problem.

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exitf
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>        // Required for the copy to user function
#include <linux/mutex.h>          // Required for the mutex functionality

#define  DEVICE_NAME "tdlchar"    ///< The device will appear at /dev/tdlchar using this value
#define  CLASS_NAME  "tdl"        ///< The device class -- this is a character device driver

// The license type provides information (via modinfo) but it also affects kernel behavior.
// You can choose "Proprietary" for non-GPL code, but the kernel will be marked as "tainted".
// If you want anyone online to help you with an issue, your kernel better not be tainted.
MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality

// The author, description, and version of the module visible with modinfo
MODULE_AUTHOR("Todd Leonhardt");  ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver");  ///< The description -- see modinfo
MODULE_VERSION("1.0");            ///< A version number to inform users

// Device drivers have an associated major and minor number.  The major number is used by the kernel
// to identify the correct device driver when the device is accessed.
static int    majorNumber;                  ///< Stores the device number -- determined automatically

static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened

// Drivers have a class name and a device name. "tdl" is used as the class name, and "tdlchar" as the
// device name. This results in the creation of a device that appears on the file system at
// /dev/tdlchar in the device tree and at /sys/class/tdl/tdlchar in the sysfs virtual file system.
static struct class*  tdlcharClass  = NULL; ///< The device-driver class struct pointer
static struct device* tdlcharDevice = NULL; ///< The device-driver device struct pointer

static DEFINE_MUTEX(tdlchar_mutex);     ///< Macro to declare a new mutex

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
   .open = dev_open,       // Called each time the device is opened from user space
   .read = dev_read,       // Called when data is sent from the device to user space
   .write = dev_write,     // Called when data is sent from user space to the device
   .release = dev_release, // Called when the device is closed in user space
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init tdlchar_init(void)
{
   printk(KERN_INFO "TDLChar: Initializing the TDLChar LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0)
   {
      printk(KERN_ALERT "TDLChar failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "TDLChar: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   tdlcharClass = class_create(THIS_MODULE, CLASS_NAME);

   // Check for error and clean up if there is
   if (IS_ERR(tdlcharClass))
   {
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(tdlcharClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "TDLChar: device class registered correctly\n");

   // Register the device driver, hard-code minor number of zero
   tdlcharDevice = device_create(tdlcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

   // Clean up if there is an error
   if (IS_ERR(tdlcharDevice))
   {
      class_destroy(tdlcharClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(tdlcharDevice);
   }
   printk(KERN_INFO "TDLChar: device class created correctly\n"); // Made it! device was initialized

   // Initialize the mutex dynamically
   mutex_init(&tdlchar_mutex);

   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit tdlchar_exit(void)
{
   mutex_destroy(&tdlchar_mutex);                           // destroy the dynamically-allocated mutex
   device_destroy(tdlcharClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(tdlcharClass);                          // unregister the device class
   class_destroy(tdlcharClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "TDLChar: Goodbye from the LKM!\n");
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep)
{
   // Try to acquire the mutex (returns 0 on fail)
   if(!mutex_trylock(&tdlchar_mutex))
   {
      printk(KERN_ALERT "TDLChar: Device in use by another process");
      return -EBUSY;
   }

   numberOpens++;
   printk(KERN_INFO "TDLChar: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);

   // if true then have success
   if (error_count==0)
   {
      printk(KERN_INFO "TDLChar: Sent %d characters to the user\n", size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else
   {
      printk(KERN_INFO "TDLChar: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }
}

/** @brief Convert a character to upper case
 * Straight forward implementation of toupper() in C since not available in kernel libs
 * @param inChar A character
 * @return returns the upper-case version of the input character
 */
static char toupper(const char inChar)
{
   char outChar = inChar;
   if (inChar >= 'a' && inChar <= 'z')
   {
      outChar = inChar - ('a' - 'A');
   }
   return outChar;
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM but it is converted to all uppercase first.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
   int i;
   // Copy the buffer contents to our message buffer one byte at a time, converting to upper case
   for( i = 0; i < len; i++)
   {
      message[i] = toupper(buffer[i]);
   }
   message[len] = '\0'; // ensure null terminated
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "TDLChar: Received %zu characters from the user\n", len);
   return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep)
{
   // release the mutex (i.e., lock goes up)
   mutex_unlock(&tdlchar_mutex);

   printk(KERN_INFO "TDLChar: Device successfully closed\n");
   return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 * identify the initialization function at insertion time and the cleanup function.
 *
 * The functions in the module can have whatever names you like; however, the same names must be
 * passed to the special macros module_init() and module_exit() at the very end of the module.
 */

// When this module is loaded, the tdlchar_init() function executes
module_init(tdlchar_init);

// When this module is unloaded, the tdlchar_exit() function executes
module_exit(tdlchar_exit);
