/**
 * @file   testtdlchar.c
 * @author Todd Leonhardt
 * @date   10 May 2017
 * @version 1.0
 * @brief  A Linux user space program that communicates with the tdlchar.c LKM. It passes a
 * string to the LKM and reads the response from the LKM. For this example to work the device
 * must be called /dev/tdlchar.
 *
 * Based on the testtdlchar.c example written by Derek Molloy
 * @see http://www.derekmolloy.ie/ for a full description and follow-up descriptions.
*/
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256               ///< The buffer length (crude but fine)
static char receive[BUFFER_LENGTH];     ///< The receive buffer from the LKM

int main()
{
   int ret, fd;
   char stringToSend[BUFFER_LENGTH];
   printf("Starting device test code example...\n");

   // Open the device with read/write access
   fd = open("/dev/tdlchar", O_RDWR);
   if (fd < 0)
   {
      perror("Failed to open the device...");
      return errno;
   }
   printf("Type in a short string to send to the kernel module:\n");

   // Read in a string (with spaces).  The %[^\n]%*c uses the scanset specifiers, which are
   // represented by %[] to use the ^ character to stop reading after the first occurrence of the \n character.
   scanf("%[^\n]%*c", stringToSend);
   printf("Writing message to the device [%s].\n", stringToSend);

   // Send the string to the LKM
   ret = write(fd, stringToSend, strlen(stringToSend));
   if (ret < 0)
   {
      perror("Failed to write the message to the device.");
      return errno;
   }

   printf("Press ENTER to read back from the device...\n");
   // The getchar() allows the program to pause at that point until the ENTER key is pressed. This
   // is necessary to demonstrate a problem with the current code formulation.
   getchar();

   printf("Reading from the device...\n");

   // Read the response from the LKM
   ret = read(fd, receive, BUFFER_LENGTH);
   if (ret < 0)
   {
      perror("Failed to read the message from the device.");
      return errno;
   }

   // Display the response from the LKM in the terminal window
   printf("The received message is: [%s]\n", receive);
   printf("End of the program\n");
   return 0;
}
