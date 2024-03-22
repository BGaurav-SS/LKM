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
#include <linux/gpio.h>

#define CLASS_NAME          "classGpioControl"
#define DRIVER_NAME         "driverGpioControl"
#define DEVICE_FILE_NAME    "deviceFileGpioControl"
#define DEVICE_NAME         "deviceGpioControl"
#define OUT_GPIO            4
#define IN_GPIO             17

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Gaurav Bhattarai");
MODULE_DESCRIPTION ("GPIO device driver.");
MODULE_VERSION ("One and Only");


// static char     kBuffer[64];
static dev_t    deviceNumber;
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
    char requestedLedState[requestedLength]; 
    int nCopy, nError;
    nError = copy_from_user(requestedLedState, uBuffer, (int)requestedLength);
    //Input 1 ==> ON, Input 0 ==> OFF
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

    printk (KERN_INFO "Initializing the GPIO device driver module.\n");
   
    //Allocate device number
    if ((alloc_chrdev_region(&deviceNumber, 12, 1, DEVICE_FILE_NAME)) < 0 ){
        printk("ERROR: Device number allocation failed.\n");
		return -1;
    }
    printk(KERN_INFO "SUCCESS: Device number allocation. Major:Minor=%d:%d", deviceNumber>>20, deviceNumber&0xfffff);

    //Create and register a class
    pClassGpioControl = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(pClassGpioControl)){
        printk(KERN_INFO"ERROR %ld: Device class creation.\n", PTR_ERR(pClassGpioControl));
        goto errorClassRegistration;
    }
    printk (KERN_INFO "SUCCESS: Device class creation: %s.\n", CLASS_NAME);

    //Create device entry belonging to created class.
    pDeviceGpioControl = device_create(pClassGpioControl, NULL, deviceNumber, NULL, DEVICE_NAME);
    if (IS_ERR(pDeviceGpioControl)){
        printk(KERN_INFO"ERROR %ld: Device file creation.\n", PTR_ERR(pDeviceGpioControl));
        goto errorDeviceCreation;
    }

    //Attach the cdev structure with file operations allowed.
    cdev_init(&cdevGpioControl, &fops);
    //Attach the cdev structure to the device node created.
    if ((cdev_add(&cdevGpioControl, deviceNumber, 1)) < 0){
        printk(KERN_INFO"Failed during the registration of device to the kernel.");
        goto errorDeviceRegistration;
    }
    printk (KERN_INFO "Success: Device file registration.\n");

    //GPIOs must be allocated before using. 
    /*
        Parameters: int gpio -  required GPIO
                    const char* label - associates a string with this gpio that can later appear in sysfs
    */
    if(gpio_request(OUT_GPIO, "RPi- Test Output GPIO") < 0){
        printk("ERROR: Allocation of output GPIO %d.\n", OUT_GPIO);
        goto errorGPIOAllocation
    };

    //Set gpio direction
    if (gpio_direction_output(OUT_GPIO, 1) < 0){
        printk("ERROR: Setting the direction of output GPIO %d.\n", OUT_GPIO);
        goto errorSetDirectionOutput
    }

    if(gpio_request (IN_GPIO, "RPi- Test Input GPIO") < 0){
        printk("ERROR: Allocation of input GPIO %d.\n", IN_GPIO);
        goto errorSetDirectionOutput;
    }

    if (gpio_direction_input(IN_GPIO) < 0){
        printk("ERROR: Setting the direction of input GPIO %d.\n", IN_GPIO);
        goto errorSetDirectionInput
    }


    return 0;

    errorSetDirectionInput:
        gpio_free(IN_GPIO);
    errorSetDirectionOutput:
        gpio_free(OUT_GPIO);
    errorGPIOAllocation:
    errorDeviceRegistration:
        device_destroy(pClassGpioControl, deviceNumber);
    errorDeviceCreation:
        class_unregister(pClassGpioControl);
        class_destroy(pClassGpioControl);
    errorClassRegistration:
        unregister_chrdev_region(deviceNumber, 1);
    
    return -1;
}

static void __exit exitFunction(void){
    gpio_set_value(OUT_GPIO, 0);
    gpio_free(IN_GPIO);
    gpio_free(OUT_GPIO);

    cdev_del(&cdevGpioControl);
    device_destroy(pClassGpioControl, deviceNumber);
    class_destroy(pClassGpioControl);
    unregister_chrdev_region(deviceNumber, 1);

    printk(KERN_LOG"GPIO driver exited.\n");

}

module_init(initFunction);
module_exit(exitFunction);