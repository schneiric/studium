#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>			//file operations structure - allows tu use open/close,read/write
#include <linux/cdev.h>			//helps to register the device to the kernel
#include <linux/semaphore.h>	
#include <asm/uaccess.h>		//copy_to_user; copy_from_user

//(1) Create a structure for our fake device
struct fake_device {
		char data[100];
		struct semaphore sem;
} virtual_device;

//(2) To later register our device we need a cdev object and some other variables
struct cdev *mcdev;		//m for 'my'
int major_number;		//will store our major number - extracted from dev_t using
						//macro - mknod /director/file c major minor
int ret;				//for return values of functions. kernel stack very small, outsoucing

dev_t dev_num;			//will hold major number that kernel gives us
						//name--> appears in /proc/devices

#define DEVICE_NAME		"exampledevice"

//(7) 	called on device_file open
//		inode reference to the (physical) file on disk
//		and contains information about that file
//		struct file represents an abstract open file; abstract is responsible for containing fops
int device_open(struct inode *inode, struct file *filp){

		if(down_interruptible(&virtual_device.sem) != 0){
				printk(KERN_ALERT "examplecode: could not lock during open");
				return -1;
			}
			
			printk(KERN_INFO "examplecode: opened device");
			return 0;	
} 

//(8) called when user wants information from the device
ssize_t device_read(struct file* filp, char* bufStoreData, size_t bufCount, loff_t* curOffset){
	//takes data from kernel space(device) to user space(process)
	//copy_to_user (destination, source, sizeToTransfer, Offset(currentPosition of openFile)
	printk(KERN_INFO "examplecode: Reading from device");
	ret = copy_to_user(bufStoreData,virtual_device.data,bufCount);
	return ret; //totally amounts of bytes written
}

//(9) called when user wants to send information to the device
ssize_t device_write(struct file* filp, const char* bufSourceData, size_t bufCount, loff_t* curOffset){
	//send data from user to kernel
	//copy_from_user (dest, source, count)
	
	printk(KERN_INFO "examplecode: writing to device");
	ret = copy_from_user(virtual_device.data, bufSourceData, bufCount);
	return ret;	
}

//(19) called upon user close
int device_close(struct inode *inode, struct file *filp){
	
		//by calling up, which is opposite of down for semaphore, we release the mutex that we obtained at device open
		//this has the effect of allowing other process to use the device now
		up(&virtual_device.sem);
		printk(KERN_INFO "examplecode: closed device");
		return 0;
}


//(6) Tell the kernel which functions to call when user operates on our device files
struct file_operations fops = {
		
		.owner = THIS_MODULE,		//prevent unloading of this modul when operations are in use
		.open = device_open,		//points to the method to call when opening the device
		.release = device_close,	//points to the method to call when clsing the device
		.write = device_write,		//points to the method to call when writing to the device
		.read = device_read,		//points to the method to call when reading from the device
};


static int driver_entry(void) {
		
		//(3) Register our device with the system: a 2 step process
		//step(a) use dynamic allocation to assign our device
		//	a major number-- alloc_chrdev_region(dev_t*, uint fminor, uint count, cha* name)
		ret = alloc_chrdev_region(&dev_num,0,1,DEVICE_NAME);
		if(ret < 0) { //at time kernel functions return negatives, there is an error
				printk(KERN_ALERT "exampledevice: failes to allocate a major number");
				return ret;	//propagate error
			}
		major_number = MAJOR(dev_num);
		printk(KERN_INFO "examplecode: major number is %d", major_number);
		printk(KERN_INFO "\tuse \"mknod /dev/%s c %d 0\" for device file",DEVICE_NAME,major_number);
		//Step(b)
		mcdev = cdev_alloc();			//create our cdev structure, initialized our cdev
		mcdev->ops = &fops;				////struct file operations
		mcdev->owner = THIS_MODULE;
		//cdev created and has to be add to the kernel
		//int cdev_add(struct cdev* dev, dev_t num, unsigned int count)
		ret = cdev_add(mcdev, dev_num,1);
		if(ret < 0 ) {
				printk(KERN_ALERT "examplecode: unable to add cdev to kernel");
				return ret;
		}
		//(4) Initialize our semaphore
		sema_init(&virtual_device.sem,1);
	return 0;
}

static void driver_exit(void){
		//(5) unregister everything in reverse order
		//(a)
		cdev_del(mcdev);
		
		//(b)
		unregister_chrdev_region(dev_num,1);
		printk(KERN_ALERT "examplecode: unload module");
}

//inform the kernel where to start an stop with our module/driver
module_init(driver_entry);
module_exit(driver_exit);
MODULE_LICENSE("GPL");
