#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/uaccess.h>
// #include <asm/uaccess.h>
// #include <linux/init.h>
// #include <linux/moduleparam.h>

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Fortune Cookie Kernel Module");
MODULE_AUTHOR("Tursunov Jasur");

#define MAX_COOKIE_LENGTH PAGE_SIZE

static struct proc_dir_entry *proc_entry, *proc_directory, *proc_link;
static char *cookie_pot; // Space for fortune strings
static int cookie_index; // Index to write next fortune
static int next_fortune; // Index to read next fortune

ssize_t fortune_write(struct file *file, const char __user *buf,
                             size_t len, loff_t *ppos)
{
    printk(KERN_INFO "+ Call fortune_write\n");
    int space_available = (MAX_COOKIE_LENGTH - cookie_index) + 1;
    if (len > space_available)
    {
        printk(KERN_INFO "Fortune: cookie pot is full!\n");
        return -ENOSPC;
    }
    if (copy_from_user(&cookie_pot[cookie_index], buf, len))
    {
        return -EFAULT;
    }
    cookie_index += len;
    cookie_pot[cookie_index-1] = 0;
    return len;
}

ssize_t fortune_read(struct file *file, char __user *buf,
                            size_t len, loff_t *ppos)
{
    printk(KERN_INFO "+ Call fortune_read\n");
    if (*ppos > 0)
        return 0;

    if (next_fortune >= cookie_index)
        next_fortune = 0;

    int l = strlen(&cookie_pot[next_fortune]) + 1;
    if (l > len) l = len;

    if (copy_to_user(buf, &cookie_pot[next_fortune], l))
        l = 0;

    next_fortune += l;
    *ppos = 1;

    return len;
}


 struct proc_ops fortune_fops = {
     // .owner = THIS_MODULE,
    .proc_read = fortune_read,
    .proc_write = fortune_write,
 };


int fortune_init(void)
{
    
    printk(KERN_INFO "+ Call fortune_init\n");
    int ret = 0;
    cookie_pot = (char *)vmalloc(MAX_COOKIE_LENGTH);
    if (!cookie_pot)
    {
        ret = -ENOMEM;
    }
    else
    {
        memset(cookie_pot, 0, MAX_COOKIE_LENGTH);

        proc_entry = proc_create("fortune", 0666, NULL, &fortune_fops);
        if (proc_entry == NULL)
        {
            ret = -ENOMEM;
            printk(KERN_INFO "Fortune: Couldn't create proc entry\n");
        }

        proc_directory = proc_mkdir("fortune_dir", NULL);
        if (proc_directory == NULL)
        {
            ret = -ENOMEM;
            printk(KERN_ERR "Fortune: Couldn't create proc directory");
        }

        proc_link = proc_symlink("fortune_link", NULL, "fortune");
        if (proc_link == NULL)
        {
            ret = -ENOMEM;
            printk(KERN_ERR "Fortune: Couldn't create proc symlink");
        }

        if (ret == -ENOMEM)
        {
            vfree(cookie_pot);
        }
        else
        {
            cookie_index = 0;
            next_fortune = 0;
            printk(KERN_INFO "Fortune: Module loaded.\n");
        }
    }
    return ret;
}

void fortune_exit(void)
{
    printk(KERN_INFO "+ Call fortune_exit\n");
    proc_remove(proc_entry);
    proc_remove(proc_directory);
    proc_remove(proc_link);
    vfree(cookie_pot);
    printk(KERN_INFO "Fortune: Module unloaded.\n");
}

module_init(fortune_init);
module_exit(fortune_exit);



// len = sprintf(buf, "%s\n", &cookie_pot[next_fortune]);
// next_fortune += len;
// *ppos = 1;

// https://stackoverflow.com/questions/64931555/how-to-fix-error-passing-argument-4-of-proc-create-from-incompatible-pointer