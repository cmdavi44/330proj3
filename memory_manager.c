#include <linux/slab.h>
#include <linux/time.h>
#include <asm/string.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/sched/signal.h>
#include <linux/mm_types.h>
#include <linux/mm.h>

#define NSEC_PER_MSEC   1000000L

static int pid = 0;
module_param(pid, int, 0644);
MODULE_PARM_DESC(pid, "proccess id");

static struct hrtimer hr_timer;
struct task_struct* task;
int totalPages = 0;
int check = -1;
int totalSwap = 0;
int totalPhys = 0;
int accessedCnt = 0;
int stopTimer = 0;
int stopThirtyTimer = 0;
int firstCnt = 0;
int totalPhysSize;
int totalSwapSize;
int accessedSize;

enum hrtimer_restart enHRTimer=HRTIMER_NORESTART;
s64 i64TimeInNsec = 10000 * NSEC_PER_MSEC;
unsigned long timer_interval_ns = 10e9; // 10-second timer

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

enum hrtimer_restart my_hrtimer_callback( struct hrtimer *timer )
{

    ktime_t currtime , interval;
    currtime  = ktime_get();
    interval = ktime_set(0,timer_interval_ns); 
    hrtimer_forward(timer, currtime , interval);

    for_each_process(task) {
        if (task->pid == pid) {
            struct vm_area_struct *vma = 0;
            unsigned long vpage;
            if (task->mm && task->mm->mmap) {
                for (vma = task->mm->mmap; vma; vma = vma->vm_next) {
//                printk("Start: %lu", vma->vm_start);
//                printk("End: %lu", vma->vm_end);
                    for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE) {
                        // totalPages++;

                        check = checkPage(task->mm, vpage, vma);

                        if(check == 0) {
                            totalSwap++;
                        }else {
                            totalPhys++;
                        }
                    }
                }
//                printk("PID: %d", task->pid);
            }
        }


    }
    totalPhysSize = totalPhys /*/ 4096*/;
    totalSwapSize = totalSwap /*/ 4096*/;
    accessedSize = accessedCnt /*/ 4096*/;
//    if (firstCnt > 0) {
        printk("[PID-%d]: RSS=[%d] KB, SWAP=[%d] KB, WSS=[%d] KB", pid, totalPhysSize, totalSwapSize, accessedSize);
//    }

    // totalPages = 0;
    // totalSwap = 0;
    // totalPhys = 0;
    // accessedCnt = 0;
    firstCnt++;
    return enHRTimer;
}

static int hrtimer_event_init_module(void)
{
    ktime_t kt;

    enHRTimer = HRTIMER_RESTART;
//    printk("hello world");
    //HRT init
    kt = ktime_set(0, timer_interval_ns/*i64TimeInNsec*/);
    hrtimer_init( &hr_timer, CLOCK_REALTIME, HRTIMER_MODE_REL); // Change to rel instead of ABS
    hrtimer_set_expires(&hr_timer, kt);
    hr_timer.function = &my_hrtimer_callback;
    hrtimer_start( &hr_timer, kt, HRTIMER_MODE_ABS);

    return 0;
}

void hrtimer_event_cleanup_module( void )
{
    //Reset
    hrtimer_cancel(&hr_timer);
    enHRTimer = HRTIMER_NORESTART;
//    printk("HR-Timer module uninstalized\n");
}

module_init(hrtimer_event_init_module);
module_exit(hrtimer_event_cleanup_module);
MODULE_LICENSE("GPL");