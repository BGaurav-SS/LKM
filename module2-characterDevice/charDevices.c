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
#include <linux/cdev.h>

#define CLASS_NAME "testClass"
#define DRIVER_NAME "testDriver"
#define DEVICE_NAME "testDevice"

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Gaurav Bhattarai");
MODULE_DESCRIPTION ("Illustration of character devices.");
MODULE_VERSION ("One and Only");


int nOpenCount=0, messageLength=0;
//dev_t is a 32-bit datatype. MSB-20 bit ==> Major number, LSB 12-bits ==> MinorNumber
static dev_t deviceNumber;      
static char kBuffer[256];
static struct class *charDevClass = NULL;
static struct device *pDevice;
static struct cdev charDevice;



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
    printk(KERN_INFO"Device closed successfully.");
    return 0;
}

/*  Called when the data is sent from device to the user space.
    @param pFile:  pointer to the file
    @param uBuffer: buffer (in userspace) which stores the data read. 
                The device driver reads from the hardware into kernel 
                space buffer first. Then this buffer is copied into this userspace buffer. 
                It is a pointer to a buffer that is already allocated in userspace
                by the application that initiated the read operation.  
    @param requested_length of data to read.
    @param offset: sets the cursor position in the file to read into. 
*/
static ssize_t dev_read(struct file *pFile, char *uBuffer, size_t requested_length, loff_t *offset){
    int nError, nCopy;

    //copies contents of kernel space buffer (kBuffer) to user-space buffer (uBuffer)
    //Returns the no. of bytes that could not be copied.
    printk (KERN_INFO "Kernel requested to write %zu characters.", requested_length);
    nCopy = min(requested_length, messageLength);
    nError = copy_to_user (uBuffer, kBuffer, nCopy);
    if (nError == 0){
        printk (KERN_INFO "Sent %d characters to user-space.\n", nCopy);
        return (nCopy - nError);
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
    @param requested_length: length of requested data transfer.
    @param offset: sets the cursor position in the file to read into.
    @return_val == length ==> copied requested no. of bytes
    @ 0 < return_val < length ==> part of data is copied. System retries to write rest of the data. Requested length is decreased automatically.
    @return_val == 0 ==> Nothing was written. System retries call to write function.
    @return_val < 0 ==> Error occured.
*/
static ssize_t dev_write(struct file *pFile, const char *uBuffer, size_t requested_length, loff_t *offset){    
    int nError, nCopy = 0;
    printk (KERN_INFO "User requested to write %zu characters.", requested_length); 

    nCopy = min(requested_length, sizeof(kBuffer));
    nError = copy_from_user(kBuffer, uBuffer, nCopy);
    messageLength = nCopy;

    printk (KERN_INFO "Wrote %d characters from user-space buffer.", nCopy);
    return (nCopy - nError);
}

/*  file_operations is a structure defined in /linux/fs.h
    The structure contains function pointers related to different operations that
    can be performed in the file.
*/
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

static int __init initFunction(void){
    printk (KERN_INFO "Initializing the character device driver module.\n");
    
    // int alloc_chrdev_region(dev_t * dev, unsigned baseminor, unsigned count, const char * name);
	if( alloc_chrdev_region(&deviceNumber, 0, 1, DEVICE_NAME) < 0) {
		printk("ERROR: Device number allocation.\n");
		return -1;
	}
    printk(KERN_INFO "SUCCESS: Device number allocation. Major:Minor=%d:%d", deviceNumber>>20, deviceNumber&0xfffff);

    //Registration of device class
    //Create a struct class pointer (used in calls to device_create)
    //Returns pointer on success, ERR_PTR on error.
    charDevClass = class_create(THIS_MODULE, CLASS_NAME);
    //IS_ERR checks if the pointer is an error pointer. Returns non-zero if the passed pointer is error pointer.
    //Reference: https://www.bhanage.com/2019/10/how-to-use-iserr-and-ptrerr-what-do.html
    if (IS_ERR(charDevClass)){
        printk(KERN_INFO"ERROR %ld: Device class creation.\n", PTR_ERR(charDevClass));
        goto errorClassRegistration;
    }
    printk (KERN_INFO "SUCCESS: Device class creation: %s.\n", CLASS_NAME);


    //creates a device file and registers it with sysfs
    //Given two integers, MKDEV combines them to create a dev_t variable that stores both Maj and Min number. Use it if you have can determine two integers.
    //Reference: https://tuxthink.blogspot.com/2012/05/working-of-macros-majorminor-and-mkdev.html
    pDevice = device_create(charDevClass, NULL, /*MKDEV(majNumber, minNumber)*/deviceNumber, NULL, DEVICE_NAME);
    if (IS_ERR(pDevice)){
        printk(KERN_INFO"ERROR %ld: Device file creation.\n", PTR_ERR(pDevice));
        goto errorDeviceFileCreation;
    }
    printk (KERN_INFO "SUCCESS: Device file creation: %s.\n", DEVICE_NAME);


    //Initialize device file
    cdev_init(&charDevice, &fops);
    //Registration of device to the kernel
    //int cdev_add(struct cdev * p, dev_t dev, unsigned count);
    if ((cdev_add(&charDevice, deviceNumber, 1)) < 0){
        printk(KERN_INFO"Failed during the registration of device to the kernel.");
        goto errorDeviceRegistration;
    }
    printk (KERN_INFO "Success: Device file registration.\n");


    return 0;


errorDeviceRegistration:
    device_destroy(charDevClass, deviceNumber);
errorDeviceFileCreation:
    class_unregister(charDevClass);
    class_destroy(charDevClass);
errorClassRegistration:
    unregister_chrdev_region(deviceNumber, 1);
    return -1;
}



static void __exit exitFunction(void){

    device_destroy(charDevClass, deviceNumber);
    class_unregister(charDevClass);
    class_destroy(charDevClass);
    unregister_chrdev_region(deviceNumber, 1);
    printk(KERN_INFO"LKM Exited.\n");

}

module_init(initFunction);
module_exit(exitFunction);