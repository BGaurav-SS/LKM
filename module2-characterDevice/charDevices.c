/*  This program creates a LKM that can be 
    interacted with via a character device file in userspace.
*/


#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

#define CLASS_NAME "testDevice"
#define DEVICE_NAME "charDevice"

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Gaurav Bhattarai");
MODULE_DESCRIPTION ("Illustration of character devices.");
MODULE_VERSION ("One and Only");


int nOpenCount=0, messageLength=0;
char kBuffer[256];
float


//  Called every time the device is opened from user space.
//  inodep:  pointer to an inode object (defined in linux/fs.h)
//  filep: pointer to a file object (defined in linux/fs.h)
static int dev_open(struct inode *inodep, struct file *filep){
    nOpenCount++;
    printk(KERN_INFO "Times this file is opened = %d", nOpenCount);
    return 0;
}

//  Called when the device is closed from the user space.
dev_release(struct inode *pInode, struct file *pFile){
    printk(KERN_LOG "Device closed successfully.");
    return 0;
}

/*  Called when the data is sent from device to the user space.
//  pFile:  pointer to the file
//  buf:    buffer (in userspace) which stores the data read. 
            The device driver reads from the hardware into kernel 
            space buffer first. Then this buffer is copied into this userspace buffer. 
//  length: length of buffer
//  offset: sets the cursor position in the file to read into. 
*/
static ssize_t dev_read(struct file *pFile, char *uBuffer, size_t length, loff_t *offset){
    int nError; 

    //copies contents of kernel space buffer (kBuffer) to user-space buffer (uBuffer)
    //Returns the no. of bytes that could not be copied. returns 0 ==> success.
    nError = copy_to_user (uBuffer, kBuffer, messageLength);
    
    if (nError == 0){
        printk (KERN_INFO "Sent %d characters to user-space.\n", messageLength);
        return 0;
    }

    else{
        printk(KERN_INFO "Failed to send %d characters to user-space.\n", nError);
        return EFAULT;
    }


}

//  Called when the data is sent from user space to the device.
dev_write()

/*  file_operations is a structure defined in /linux/fs.h
    The structure contains function pointers related to different operations that
    can be performed in the file.
*/
static struct file_operations fops = {
    .open = dev_open;
    .release = dev_release;
    .read = dev_read;
    .write = dev_write; 
}



static int __init initFunction(void){
    
    return 0;
}

static void __exit exitFunction(void){

}


module_init(initFunction);
module_exit(exitFunction);