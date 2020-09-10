#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <proc/switch.h>
#include <interrupt.h>

extern struct list plist;
extern struct list rlist;
extern struct list slist;
extern struct list runq[RQ_NQS];

extern struct process procs[PROC_NUM_MAX];
extern struct process *idle_process;
struct process *latest;

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);
int scheduling; 					// interrupt.c
int newLine = 0;

struct process* get_next_proc(void) 
{
	bool found = false;
	struct process *next = NULL;
	struct list_elem *elem;

	/* 
	   You shoud modify this function...
	   Browse the 'runq' array 
	*/

	/* runq에서 우선순위 순대로 process 선택 */
	for(int i = 0; i < RQ_NQS; i++){
		for(elem = list_begin(&runq[i]); elem != list_end(&runq[i]); elem = list_next(elem)){
			next = list_entry(elem, struct process, elem_stat);

			if(next->state == PROC_RUN){
				return next;	// next process return
			}
		}
	}

	return next;
}

void schedule(void)
{
	struct process *cur;
	struct process *next;

	/* You shoud modify this function.... */
	
	if(cur_process != idle_process){	// idle process가 아닐 경우 idle process로 스케줄링
		scheduling = 1;	// timer handler tick 보정
		cur = cur_process;
		cur_process = idle_process;
		next = idle_process;
		next->time_slice = 0;

		scheduling = 0;

		intr_disable();	// 인터럽트 disable
		switch_process(cur, next);
		intr_enable();	// 인터럽트 enable
		return;
	}

	proc_wake();

	struct list_elem *elem;
	
	newLine = 0;

	/* 프로세스 schedule정보(실행결과) 출력 */
	for(elem = list_begin(&plist); elem != list_end(&plist); elem = list_next(elem)){
		struct process *p = list_entry(elem, struct process, elem_all);

		if(p->pid != 0 && p->state == PROC_RUN){
			printk("#=%2d p=%4d c=%4d u=%4d", p->pid, p->priority, p->time_slice, p->time_used);
			newLine++;
		}

		if(newLine > 0 && newLine < list_size(&plist)-list_size(&slist)- 1 && p->state == PROC_RUN){
			printk(", ");
		}
	}

	if(newLine){
		printk("\n");
		newLine = 0;
	}

	next = get_next_proc();

	if(next->state == PROC_RUN && next != idle_process){	// schedule된 함수 출력
		printk("Selected # = %d\n", next->pid);
	}

	cur = cur_process;
	cur_process = next;
	cur_process->time_slice = 0;

	intr_disable();	// 인터럽트 disable
	switch_process(cur, next);
	intr_enable();	// 인터럽트 enable
}
