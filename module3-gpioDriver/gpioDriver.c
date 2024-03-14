/*  
    This program creates a GPIO device driver.
    Controlling the LED and reading the status of a GPIO pin.
*/

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/cdev.h>

#define CLASS_NAME          "classGpioControl"
#define DRIVER_NAME         "driverGpioControl"
#define DEVICE_FILE_NAME    "deviceFileGpioControl"

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Gaurav Bhattarai");
MODULE_DESCRIPTION ("GPIO device driver.");
MODULE_VERSION ("One and Only");


static dev_t    deviceNumber;
// static char     kBuffer[64];
static int      times = 0;

static struct class*    pClassGpioControl;
static struct device*   pDeviceGpioControl;
static struct cdev      cdevGpioControl;

static struct file_operations fops ={
    .owner = THIS_MODULE,
    .open = deviceFileOpen,
    .release = deviceFileRelease,
    .write = deviceFileWrite,
    .read = deviceFileRead,
};

static int deviceFileOpen(struct inode* pInode, struct file* pFile){
    printk(KERN_INFO "GPIO device file opened. Count = %d.\n", ++times);
    return 0;
}

static int deviceFileRelease(struct inode* pInode, struct file* pFile){
    printk(KERN_INFO "GPIO device file closed/released.");
    return 0;
}

static int deviceFileWrite(struct file* pFile, const char* uBuffer, size_t requestedLength, loff_t* pOffset){
    char requestedLedState[requestedLength]; //1 ==> ON, 0 ==> OFF
    int nCopy, nError;
    nError = copy_from_user(requestedLedState, uBuffer, (int)requestedLength);
    switch (requestedLedState[0]){
        case '1':
            //Turn LED on
        case '0':
            //Turn LED off
        default:
            printk(KERN_INFO "Invalid input provided by user.\n");
    }
    return ((int)requestedLength - nError);
}

static int deviceFileRead(struct file* pFile, const char* uBuffer, size_t requestedLength, loff_t* pOffset){
    return ((int)requestedLength);
}



static int __init initFunction(void){
    return 0;
}

static void __exit exitFunction(void){
}

module_init(initFunction);
module_exit(exitFunction);