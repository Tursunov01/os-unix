#include <linux/kernel.h>
#include <linux/module.h> 
#include <linux/interrupt.h> 
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

#define IRQ_NUM 1 
#define PROC_FILE_NAME "tasklet"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tursunov Jasurbek");

static int my_dev_id;
const unsigned long my_tasklet_data = 123456; 
static struct proc_dir_entry *proc_file;

struct tasklet_struct my_tasklet;

static int __init my_tasklet_init(void);
static int my_tasklet_open(struct inode *inode, struct file *file);
static int my_tasklet_release(struct inode *inode, struct file *file);
static int my_tasklet_show(struct seq_file *m, void *v);
static void __exit my_tasklet_exit(void);

void my_tasklet_handler(unsigned long data);
static irqreturn_t my_interrupt_handler(int irq, void *dev_id);
DECLARE_TASKLET(my_tasklet, my_tasklet_handler, my_tasklet_data);

static const struct proc_ops my_tasklet_props = 
{
    .proc_open = my_tasklet_open,
    .proc_release = my_tasklet_release,
    .proc_read = seq_read
};

static inline void printk_tasklet_info(const char* prefix)
{
    printk(KERN_INFO "/my_tasklet/ %s -state: %ld, count: %d, data: %ld\n",prefix, my_tasklet.state, my_tasklet.count, my_tasklet.data);
}

static int my_tasklet_show(struct seq_file *m, void *v)
{
    printk(KERN_INFO "/my_tasklet/ Called my_tasklet_show\n"); 
    seq_printf(m, "state: %ld\ncount: %d\ndata: %ld\n", my_tasklet.state, my_tasklet.count, my_tasklet.data);
    return 0;
}

static int my_tasklet_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "/my_tasklet/ Called my_tasklet_open\n"); 
    return single_open(file, my_tasklet_show, NULL);
}

void my_tasklet_handler(unsigned long data) 
{
    printk(KERN_INFO "/my_tasklet/ Called my_tasklet_handler\n"); 
    printk_tasklet_info("");
}

static irqreturn_t my_interrupt_handler(int irq, void *dev_id) 
{
    printk(KERN_INFO "/my_tasklet/ Called my_interrupt_handler\n"); 
    if (irq == IRQ_NUM) 
    {
        tasklet_schedule(&my_tasklet); 
        printk_tasklet_info("In interrupt handler");
        printk(KERN_INFO "/my_tasklet/ Tasklet scheduled\n"); 
        return IRQ_HANDLED;
    } 
    else return IRQ_NONE;
}

static int my_tasklet_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "/my_tasklet/ Called my_tasklet_release\n"); 
    return single_release(inode, file);
}

static int __init my_tasklet_init(void)
{
    proc_file = proc_create(PROC_FILE_NAME, S_IRUGO, NULL, &my_tasklet_props);
    if (!proc_file)
    {
        printk(KERN_ERR "/my_tasklet/ Error: can't create seq file\n");
        return -ENOMEM; 
    }
    if (request_irq(IRQ_NUM, my_interrupt_handler, IRQF_SHARED, "my_interrupt_tasklet", &my_dev_id))
    {
        printk(KERN_ERR "/my_tasklet/ Error: can't register irq handler\n"); 
        return -1;
    }
    printk(KERN_INFO "/my_tasklet/ Module loaded\n");
    return 0;
}

static void __exit my_tasklet_exit(void) 
{
    tasklet_kill(&my_tasklet);
    free_irq(IRQ_NUM, &my_dev_id);
    if (proc_file)
        remove_proc_entry(PROC_FILE_NAME, NULL);
    printk(KERN_INFO "/my_tasklet/ Module unloaded\n");
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);