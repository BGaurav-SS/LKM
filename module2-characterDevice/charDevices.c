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
int majNumber, minNumber;
char kBuffer[256];



//  Called every time the device is opened from user space.
//  inodep:  pointer to an inode object (defined in linux/fs.h)
//  filep: pointer to a file object (defined in linux/fs.h)
static int dev_open(struct inode *inodep, struct file *filep){
    nOpenCount++;
    printk(KERN_INFO "Times this file is opened = %d", nOpenCount);
    return 0;
}

//  Called when the device is closed from the user space.
static int dev_release(struct inode *pInode, struct file *pFile){
    printk(KERN_LOG "Device closed successfully.");
    return 0;
}

/*  Called when the data is sent from device to the user space.
    @param pFile:  pointer to the file
    @param uBuffer: buffer (in userspace) which stores the data read. 
                The device driver reads from the hardware into kernel 
                space buffer first. Then this buffer is copied into this userspace buffer. 
                It is a pointer to a buffer that is already allocated in userspace
                by the application that initiated the read operation.  
    @param length of user-space buffer
    @param offset: sets the cursor position in the file to read into. 
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
        return -EFAULT;
    }
}

/*  Called when the data is sent from user-space to kernel space.
    @param pFile:  pointer to the file
    @param uBuffer: buffer (in userspace) which has the data to be sent. 
                sprintf function copies the contents of uBuffer into kBuffer.  
    @param length: length of user space buffer
    @param offset: sets the cursor position in the file to read into. 
*/
static ssize_t dev_write(struct file *pFile, char *uBuffer, size_t length, loff_t *offset){    
    sprintf(kBuffer, "%s", uBuffer);
    messageLength = strlen (kBuffer);
    printk (KERN_INFO "Received %zu characters from user-space buffer.", length);
    return 0;
}

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
    printk (KERN _INFO "Initializing the character device driver module\n");
    
    //int register_chrdev(uint major, uint baseminor, uint count, 
    //                    char *name, struct file_operations *fops)
    //To allocate minor numbers for @count devices starting from @baseminor

    //int register_chrdev(uint major, char *name, struct file_operations *fops) 
    //For single device.
    majNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majNumber < 0){
        printk(KERN_INFO "Failed during registration of major number.\n");
        return majNumber;
    }
    printk(KERN_INFO "Successfully registered the device. Major:Minor=%d:%d", majNumber>>20, majNumber&0xfffff);
    
    else
    
    return 0;
}

static void __exit exitFunction(void){

}


module_init(initFunction);
module_exit(exitFunction);