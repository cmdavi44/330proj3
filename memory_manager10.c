#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched/signal.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/hrtimer.h>
#include <linux/jiffies.h>


MODULE_AUTHOR("Group 37");

// Setting up the 4 module parameters that will take input and change
// the variables accordingly.
static int pid = 0;
module_param(pid, int, 0644);
MODULE_PARM_DESC(pid, "proccess id");

struct task_struct* task;
int totalPages = 0;
int check = -1;
int totalSwap = 0;
int totalPhys = 0;
int accessedCnt = 0;
int stopTimer = 0;
int stopThirtyTimer = 0;

unsigned long timer_interval_ns = 10e9; // 10-second timer
//HRTimer
static struct hrtimer  my_hrtimer;
//static struct hrtimer thirtySecTimer;
u64 start_t;



// Check if page in physical memory or swap && test if page was accessed
int checkPage(struct mm_struct* mm,unsigned long address, struct vm_area_struct *vma) {
    pgd_t *pgd;
    p4d_t *p4d;
    pmd_t *pmd;
    pud_t *pud;
    pte_t *ptep, pte;

    pgd = pgd_offset(mm, address);                    // get pgd from mm and the page address
    if (pgd_none(*pgd) || pgd_bad(*pgd)){           // check if pgd is bad or does not exist
        return 0;}

    p4d = p4d_offset(pgd, address);                   // get p4d from from pgd and the page address
    if (p4d_none(*p4d) || p4d_bad(*p4d)){          // check if p4d is bad or does not exist
        return 0;}

    pud = pud_offset(p4d, address);                   // get pud from from p4d and the page address
    if (pud_none(*pud) || pud_bad(*pud)){          // check if pud is bad or does not exist
        return 0;}

    pmd = pmd_offset(pud, address);               // get pmd from from pud and the page address
    if (pmd_none(*pmd) || pmd_bad(*pmd)){       // check if pmd is bad or does not exist
        return 0;}

    ptep = pte_offset_map(pmd, address);      // get pte from pmd and the page address
    if (!ptep){return 0;}                                         // check if pte does not exist
    pte = *ptep;

//    printk("Test: %d", );
    ptep_test_and_clear_young(vma, address, ptep);
    return 1;
}

// Test if page was accessed function
int ptep_test_and_clear_young(struct vm_area_struct *vma, unsigned long addr, pte_t *ptep) {
    int ret = 0;
    if (pte_young(*ptep))
        ret = test_and_clear_bit(_PAGE_BIT_ACCESSED,
                                 (unsigned long *) &ptep->pte);
    if (ret == 1)
        accessedCnt++;

    return ret;
}

// Timer Function
//enum hrtimer_restart thirty_no_restart_callback(struct hrtimer *timer)
//{
//    u64 now_t = jiffies;
//    printk("Accessed Count: %d", accessedCnt);
//    printk("start_t - now_t = %u\n", jiffies_to_msecs(now_t - start_t));
//    stopThirtyTimer = 1;
//    return HRTIMER_NORESTART;
//}

enum hrtimer_restart no_restart_callback(struct hrtimer *timer)
{
    u64 now_t = jiffies;
    int totalPhysSize = totalPhys / 4096;
    int totalSwapSize = totalSwap / 4096;
    int accessedSize = accessedCnt / 4096;
    printk("PID [%d]: RSS=[%d] KB, SWAP=[%d] KB, WSS=[%d] KB", pid, totalPhysSize, totalSwapSize, accessedSize);
//    printk("start_t - now_t = %u\n", jiffies_to_msecs(now_t - start_t));
    stopTimer = 1;
    return HRTIMER_RESTART;
}

// Page Walk Function
static int __init initialize(void) {
//    int process_counter = 0;
//    printk("Proccess ID: %d\n", Intparameter);
//    printk("---Hello World---");

//    hrtimer_init(&thirtySecTimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
//    thirtySecTimer.function = &thirty_no_restart_callback;
//    start_t = jiffies;
//    hrtimer_start(&thirtySecTimer, ms_to_ktime(30000), HRTIMER_MODE_REL);


    // 10 Second Timer
//    int i = 2;
//    while (i >= 0) {
        hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
        my_hrtimer.function = &no_restart_callback;
        start_t = jiffies;
        hrtimer_start(&my_hrtimer, ms_to_ktime(10000), HRTIMER_MODE_REL);
//        while (stopTimer == 0) {
            for_each_process(task) {
                struct vm_area_struct *vma = 0;
                unsigned long vpage;
                if (task->mm && task->mm->mmap) {
                    for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
//                printk("Start: %lu", vma->vm_start);
//                printk("End: %lu", vma->vm_end);
                        for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
                            totalPages++;

                            check = checkPage(task->mm, vpage, vma);

                            if(check == 0) {
                                totalSwap++;
                            }else {
                                totalPhys++;
                            }
                        }
                    }
                }

            }
//        }
//        i--;
//    }


    return 0;
}

static void __exit clean_exit(void) {
//    printk("---Goodbye World---");
    hrtimer_cancel(&my_hrtimer);
}

module_init(initialize);
module_exit(clean_exit);
MODULE_LICENSE("GPL");