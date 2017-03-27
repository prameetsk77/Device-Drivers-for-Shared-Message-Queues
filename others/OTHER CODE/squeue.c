/* ----------------------------------------------- DRIVER squeue --------------------------------------------------

 Basic driver example to show skelton methods for several file operations.

 ----------------------------------------------------------------------------------------------------------------*/

#include <linux/module.h>  // Module Defines and Macros (THIS_MODULE)
#include <linux/kernel.h>  //
#include <linux/fs.h>	   // Inode and File types
#include <linux/cdev.h>    // Character Device Types and functions.
#include <linux/types.h>
#include <linux/slab.h>	   // Kmalloc/Kfree
#include <asm/uaccess.h>   // Copy to/from user space
#include <linux/string.h>
#include <linux/device.h>  // Device Creation / Destruction functions
#include <linux/jiffies.h> // Timing Library
#include "common_data.h"
#include<linux/init.h>
#include<linux/moduleparam.h> // Passing parameters to modules through insmod


#define DEVICE_NAME_1                 "squeue1"  // device name to be created and registered
#define DEVICE_NAME_2                 "squeue2"  // device name to be created and registered
#define DEVICE_NAME_3                 "squeue3"  // device name to be created and registered
#define DEVICE_NAME_4                 "squeue4"  // device name to be created and registered

#define QUEUE_LENGTH				10
/*
static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
    __asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
    return x;
}
*/

static __inline__ unsigned long long rdtsc(void)
{
    unsigned long long int x;
__asm__ __volatile__("rdtsc; "          // read of tsc
                     "shl $32,%%rdx; "  // shift higher 32 bits stored in rdx up
                     "or %%rdx,%%rax"   // and or onto rax
                     : "=a"(x)        // output to tsc
                     :
                     : "%rcx", "%rdx", "memory"); // rcx and rdx are clobbered
                                                  // memory to prevent reordering
    return x;
}

/* per device structure */
struct squeue_dev {
	struct cdev cdev;               /* The cdev structure */
	char name[20];                  /* Name of device*/	
	struct message message_queue[QUEUE_LENGTH];	/* The message */
	int head;
	int tail;
	int size;
} *squeue_devp1, *squeue_devp2, *squeue_devp3, *squeue_devp4;

static dev_t squeue_dev_number1;      /* Allotted device number */
static dev_t squeue_dev_number2;      /* Allotted device number */
static dev_t squeue_dev_number3;      /* Allotted device number */
static dev_t squeue_dev_number4;      /* Allotted device number */
int a=0;
struct class *squeue_dev_class1;
struct class *squeue_dev_class2;
struct class *squeue_dev_class3;
struct class *squeue_dev_class4;		/* Tie with the device model */

static struct device *squeue_dev_device1;
static struct device *squeue_dev_device2;
static struct device *squeue_dev_device3;
static struct device *squeue_dev_device4;

/*
* Open squeue driver
*/
int squeue_driver_open(struct inode *inode, struct file *file)
{
	struct squeue_dev *squeue_devp;
	

	/* Get the per-device structure that contains this cdev */
	squeue_devp = container_of(inode->i_cdev, struct squeue_dev, cdev);


	/* Easy access to cmos_devp from rest of the entry points */
	file->private_data = squeue_devp;
	printk("\n%s is openning \n", squeue_devp->name);
	return 0;
}

/*
 * Release squeue driver
 */
int squeue_driver_release(struct inode *inode, struct file *file)
{
	struct squeue_dev *squeue_devp = file->private_data;
	
	squeue_devp->head = 0;

	squeue_devp->tail = 0;

	squeue_devp->size = 0;

	printk("\nRelease: %s is closing\n", squeue_devp->name);
	
	return 0;
}

/*
 * Write to squeue driver
 */
ssize_t squeue_driver_write(struct file *file, const char *buf,
           size_t count, loff_t *ppos)
{	
	
	struct squeue_dev *squeue_devp = file->private_data;
	if (squeue_devp->size==QUEUE_LENGTH-1)
		return -1;
	squeue_devp->message_queue[squeue_devp->tail].tsc = squeue_devp->message_queue [squeue_devp->tail].tsc +rdtsc();
	
	if (copy_from_user((void*) &squeue_devp->message_queue [squeue_devp->tail], buf, count)) 
		return -EFAULT;

	printk("\nSize: %d\nTail Position %d\nHead Position %d \n", squeue_devp->size, squeue_devp->tail, squeue_devp->head);

	squeue_devp->tail++;
	squeue_devp->size++;

	if(squeue_devp->tail==QUEUE_LENGTH){
		squeue_devp->tail=0;
	}

	return 0;
}

/*
 * Read to squeue driver
 */
ssize_t squeue_driver_read(struct file *file, char *buf,
           size_t count, loff_t *ppos)
{
	
	struct squeue_dev *squeue_devp = file->private_data;

	if (squeue_devp->size==0)
		return -1;	
	squeue_devp->message_queue [squeue_devp->head].tsc = rdtsc() - squeue_devp->message_queue [squeue_devp->head].tsc;
	
	copy_to_user( buf, (void*) &squeue_devp->message_queue[squeue_devp->head], count);
	

	printk("\nSize: %d\nTail Position %d\nHead Position %d \n", squeue_devp->size, squeue_devp->tail, squeue_devp->head);
	squeue_devp->head++;
	squeue_devp->size--;

	if(squeue_devp->head==QUEUE_LENGTH){ // 
		squeue_devp->head=0;
	}
	
	return 0;
}

/* File operations structure. Defined in linux/fs.h */
static struct file_operations squeue_fops = {
	.read		= squeue_driver_read,        /* Read method */
    .owner		= THIS_MODULE,           /* Owner */
    .open		= squeue_driver_open,        /* Open method */
    .release		= squeue_driver_release,     /* Release method */
    .write		= squeue_driver_write,       /* Write method */
};

/*
 * Driver Initialization
 */
int __init squeue_driver_init(void)
{
	int ret1;
	int ret2;
	int ret3;
	int ret4;
	

	/* Request dynamic allocation of a device major number */
	if (alloc_chrdev_region(&squeue_dev_number1, 0, 1, DEVICE_NAME_1) < 0) {
			printk(KERN_DEBUG "Can't register device\n"); return -1;
	}
	if (alloc_chrdev_region(&squeue_dev_number2, 0, 1, DEVICE_NAME_2) < 0) {
				printk(KERN_DEBUG "Can't register device\n"); return -1;
	}
	if (alloc_chrdev_region(&squeue_dev_number3, 0, 1, DEVICE_NAME_3) < 0) {
				printk(KERN_DEBUG "Can't register device\n"); return -1;
	}
	if (alloc_chrdev_region(&squeue_dev_number4, 0, 1, DEVICE_NAME_4) < 0) {
				printk(KERN_DEBUG "Can't register device\n"); return -1;
	}

	/* Populate sysfs entries */
	squeue_dev_class1 = class_create(THIS_MODULE, DEVICE_NAME_1);
	squeue_dev_class2 = class_create(THIS_MODULE, DEVICE_NAME_2);
	squeue_dev_class3 = class_create(THIS_MODULE, DEVICE_NAME_3);
	squeue_dev_class4 = class_create(THIS_MODULE, DEVICE_NAME_4);

	/* Allocate memory for the per-device structure */
	squeue_devp1 = kmalloc(sizeof(struct squeue_dev), GFP_KERNEL);
	squeue_devp2 = kmalloc(sizeof(struct squeue_dev), GFP_KERNEL);
	squeue_devp3 = kmalloc(sizeof(struct squeue_dev), GFP_KERNEL);
	squeue_devp4 = kmalloc(sizeof(struct squeue_dev), GFP_KERNEL);

	if (!(squeue_devp1 || squeue_devp2 || squeue_devp3 || squeue_devp4)) {
		printk("Bad Kmalloc\n"); return -ENOMEM;
	}

	/* Request I/O region */
	sprintf(squeue_devp1->name, DEVICE_NAME_1);
	sprintf(squeue_devp2->name, DEVICE_NAME_2);
	sprintf(squeue_devp3->name, DEVICE_NAME_3);
	sprintf(squeue_devp4->name, DEVICE_NAME_4);

	/* Connect the file operations with the cdev */
	cdev_init(&squeue_devp1->cdev, &squeue_fops);
	cdev_init(&squeue_devp2->cdev, &squeue_fops);
	cdev_init(&squeue_devp3->cdev, &squeue_fops);
	cdev_init(&squeue_devp4->cdev, &squeue_fops);


	squeue_devp1->cdev.owner = THIS_MODULE;
	squeue_devp2->cdev.owner = THIS_MODULE;
	squeue_devp3->cdev.owner = THIS_MODULE;
	squeue_devp4->cdev.owner = THIS_MODULE;
	
	squeue_devp1->head = 0;
	squeue_devp2->head = 0;
	squeue_devp3->head = 0;
	squeue_devp4->head = 0;

	squeue_devp1->tail = 0;
	squeue_devp2->tail = 0;
	squeue_devp3->tail = 0;
	squeue_devp4->tail = 0;

	squeue_devp1->size = 0;
	squeue_devp2->size = 0;
	squeue_devp3->size = 0;
	squeue_devp4->size = 0;
	
	/* Connect the major/minor number to the cdev */
	ret1 = cdev_add(&squeue_devp1->cdev, (squeue_dev_number1), 1);
	ret2 = cdev_add(&squeue_devp2->cdev, (squeue_dev_number2), 1);
	ret3 = cdev_add(&squeue_devp3->cdev, (squeue_dev_number3), 1);
	ret4 = cdev_add(&squeue_devp4->cdev, (squeue_dev_number4), 1);

	if (ret1 || ret2 || ret3 || ret4) {
		printk("Bad cdev\n");
		return -1;
	}

	/* Send uevents to udev, so it'll create /dev nodes */
	squeue_dev_device1 = device_create(squeue_dev_class1, NULL, MKDEV(MAJOR(squeue_dev_number1), 0), NULL, DEVICE_NAME_1);
	squeue_dev_device2 = device_create(squeue_dev_class2, NULL, MKDEV(MAJOR(squeue_dev_number2), 0), NULL, DEVICE_NAME_2);
	squeue_dev_device3 = device_create(squeue_dev_class3, NULL, MKDEV(MAJOR(squeue_dev_number3), 0), NULL, DEVICE_NAME_3);
	squeue_dev_device4 = device_create(squeue_dev_class4, NULL, MKDEV(MAJOR(squeue_dev_number4), 0), NULL, DEVICE_NAME_4);
	// device_create_file(squeue_dev_device, &dev_attr_xxx);

	//memset(squeue_devp->in_string, 0, 256);

	//time_since_boot=(jiffies-INITIAL_JIFFIES)/HZ;//since on some systems jiffies is a very huge uninitialized value at boot and saved.
	//sprintf(squeue_devp->in_string, "Hi %s, this machine has been on for %d seconds", user_name, time_since_boot);

	//squeue_devp->current_write_pointer = 0;

	printk("squeue driver initialized.\n");// '%s'\n",squeue_devp->in_string);
	return 0;
}


/* Driver Exit */
void __exit squeue_driver_exit(void)
{
	// device_remove_file(squeue_dev_device, &dev_attr_xxx);
	/* Release the major number */
	unregister_chrdev_region((squeue_dev_number1), 1);
	unregister_chrdev_region((squeue_dev_number2), 1);
	unregister_chrdev_region((squeue_dev_number3), 1);
	unregister_chrdev_region((squeue_dev_number4), 1);

	/* Destroy device */
	device_destroy (squeue_dev_class1, MKDEV(MAJOR(squeue_dev_number1), 0));
	device_destroy (squeue_dev_class2, MKDEV(MAJOR(squeue_dev_number2), 0));
	device_destroy (squeue_dev_class3, MKDEV(MAJOR(squeue_dev_number3), 0));
	device_destroy (squeue_dev_class4, MKDEV(MAJOR(squeue_dev_number4), 0));

	cdev_del(&squeue_devp1->cdev);
	cdev_del(&squeue_devp2->cdev);
	cdev_del(&squeue_devp3->cdev);
	cdev_del(&squeue_devp4->cdev);

	kfree(squeue_devp1);
	kfree(squeue_devp2);
	kfree(squeue_devp3);
	kfree(squeue_devp4);

	/* Destroy driver_class */
	class_destroy(squeue_dev_class1);
	class_destroy(squeue_dev_class2);
	class_destroy(squeue_dev_class3);
	class_destroy(squeue_dev_class4);


	printk("squeue driver removed.\n");
}

module_init(squeue_driver_init);
module_exit(squeue_driver_exit);
MODULE_LICENSE("GPL v2");


