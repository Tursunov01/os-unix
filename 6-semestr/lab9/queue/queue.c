#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/slab.h>

#define IRQ_NUM 1

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tursunov Jasurbek");

static int my_dev_id;
static int irq_call_count= 0;
struct workqueue_struct *wq;

typedef struct
{
    struct work_struct my_work;
    int id;
    int data;
} 
my_work_t;
my_work_t *work1, *work2;


void printk_time(void)
{
    struct timespec64 ts;
    ktime_get_ts64(&ts);
    printk(KERN_INFO "/my_wq/ System work time -%02lld:%02lld:%02lld\n", ts.tv_sec / 3600 % 24, ts.tv_sec / 60 % 60, ts.tv_sec % 60);
}
    
void my_wq_function(struct work_struct *work) 
{
    int delay = 1234;
    my_work_t *my_work = (my_work_t *) work;
    printk(KERN_INFO "/my_wq/\n/my_wq/ my_work%d.data = %d\n", my_work->id, ++my_work->data);
    printk(KERN_INFO "/my_wq/ counter = %d\n", ++irq_call_n);
    printk_time();
    msleep(delay);
    printk(KERN_INFO "/my_wq/ After %d milliseconds\n", delay);
    printk_time();
}

static irqreturn_t my_interrupt_handler(int irq, void *dev_id) 
{
    if (irq == SHARED_IRQ) 
    {
        printk(KERN_INFO "/my_wq/ Called my_interrupt_handler\n");
        if (work1)
            queue_work(wq, &work1->my_work);
        if (work2)
            queue_work(wq, &work2->my_work);
            return IRQ_HANDLED;
    } 
    else 
        return IRQ_NONE;
}

static my_work_t* alloc_work(int id)
{
    my_work_t*work = (my_work_t *) kmalloc(sizeof(my_work_t), GFP_KERNEL);
    if (work)
    {
        work->id = id;
        work->data = 0;
    }
    return work;
}

static int __init my_wq_init(void) 
{
    if (request_irq(IRQ_NUM, my_interrupt_handler, IRQF_SHARED, "my_interrupt_wq", &my_dev_id))
    {
        printk(KERN_ERR "/my_wq/Error: can't register interrupt handler\n");
        return -1;
    }
    wq= create_workqueue("workqueue");
    if (wq) 
    {
        printk(KERN_INFO "/my_wq/ Workqueue created\n"); 
        if ((work1 = alloc_work(1)))
            INIT_WORK(&work1->my_work, my_wq_function);
        else 
            printk(KERN_ERR "/my_wq/ Error: can't initwork1\n");
        if ((work2 = alloc_work(2)))
            INIT_WORK(&work2->my_work, my_wq_function);
        else 
            printk(KERN_ERR "/my_wq/ Error: can't init work2\n");
    }
    else
    {
        free_irq(IRQ_NUM, &my_dev_id);
        printk(KERN_ERR "/my_wq/ Error: can't create workqueue\n");
        return -ENOMEM;
    }
    printk(KERN_INFO "/my_wq/ Module loaded\n");
    return 0; 
}

static void __exit my_wq_exit(void) 
{
    flush_workqueue(wq);
    destroy_workqueue(wq);
    free_irq(IRQ_NUM, &my_dev_id);
    if (work1)
        kfree((void *)work1);
    if (work2)
        kfree((void *)work2);
    printk(KERN_INFO "/my_wq/ Module unloaded\n");
}

module_init(my_wq_init);
module_exit(my_wq_exit);