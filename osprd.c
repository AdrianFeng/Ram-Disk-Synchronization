#include <linux/version.h>
#include <linux/autoconf.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/kernel.h>  /* printk() */
#include <linux/errno.h>   /* error codes */
#include <linux/types.h>   /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/wait.h>
#include <linux/file.h>
#include "spinlock.h"
#include "osprd.h"
/////////////////////////////////////////////////////////

struct node 
{
       int pid;
       int status;
       struct Nodelist *Wait; //I wait other people
       struct Nodelist *Next; // point to who wait me.
};
struct Nodelist
{
  struct node * element;
  struct Nodelist * next;
  struct Nodelist * prev;
  int size; 
  int tempsize;    
};
/*void PrintNode(struct Nodelist *list)
{
     int n=0;
     struct Nodelist *it =list;
     while(it)
     {
              //printf("index %d pid is %d   he wait %d process and %d process wait him\n his status is %d \n",n++,it->element->pid,it->element->Wait->size,it->element->Next->size,it->element->status);
              struct Nodelist * waitlist=it->element->Wait;
              //printf("this is what he wait \n");
              while(waitlist->next)
              {
               //printf("%d ",waitlist->next->element->pid);                    
               waitlist=waitlist->next;                
              }
              //printf("\n");
              struct Nodelist * nextlist=it->element->Next;
              //printf("this is what pid need to wait him \n");
              while(nextlist->next)
              {
               //printf("%d ",nextlist->next->element->pid);
               nextlist=nextlist->next;
              }
              //printf("\n");
              it=it->next;
     }
     }*/
void insert(struct Nodelist **list, struct node **A)
{
    struct Nodelist *it =*list;
    while(it->next!=NULL)
    {
     it=it->next;
    }
    struct Nodelist *L=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
    L->element=*A;
    it->next=L;
    L->prev=it;
    it=it->next;
    it->next=NULL;
    (*list)->size++;  
}
int check(struct Nodelist *list)
{
    struct Nodelist *it =list;
    struct node *e;
    struct Nodelist *itNext;
    struct Nodelist *itWait;
    int size=list->size;
    while(it->next!=NULL)
    {
     it->next->element->status=1;
     it->next->element->Wait->tempsize=it->next->element->Wait->size;
     it=it->next;
    } 
    while(size>0)
    {
       it =list;
       while(it->next!=NULL)
       {
        e=it->next->element;
        if(e->status==1)
        {
         if(e->Wait->tempsize==0)
         {
          //printf(" inside e->Wait %d ",e->pid);
          e->status=0;
          itNext=e->Next;
          while(itNext->next!=NULL)
          {
           itNext->next->element->Wait->tempsize--;
           itNext=itNext->next;
          }
         }
        }
        it=it->next;
       }
       size--;
       //system("pause");
   } 
   it=list;
   //PrintNode(list->next);
    while(it->next!=NULL)
    {
     if(it->next->element->status==1)
     {
      //printf("dead");
      return 1;
      break;
     }
     it=it->next;
    }  
    return 0;
}
int Insert(struct Nodelist **list, int waitpid, int nextpid)
{
    struct Nodelist *it =*list;
    struct Nodelist *W;
    struct Nodelist *N;
    struct node *WW;
    struct node *NN;
    int nofindwait=1;
    int nofindnext=1;
    while(it->next!=NULL)
    {
     if(it->next->element->pid==waitpid)
     {
      W=it->next;
      nofindwait=0;
      //printf("find pid for wait pid \n");
      break;
     }     
     it=it->next;
    }
    if(nofindwait)
    {
      struct Nodelist *L=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
      struct node *A=kmalloc(sizeof(struct node),GFP_ATOMIC);
      A->pid=waitpid;
      A->status=1;
      A->Wait=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
      A->Wait->size=0;
      A->Wait->element=NULL;
      A->Wait->next=NULL;
      A->Next=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
      A->Next->size=0;
      A->Next->element=NULL;
      A->Next->next=NULL;
      L->element=A;
      it->next=L;
      L->prev=it;
      it=it->next;
      it->next=NULL;                 
      W=L;
      (*list)->size++;
    }
    it=*list;
    while(it->next!=NULL)
      {
	if(it->next->element->pid==nextpid)
	  {
	    N=it->next;
	    nofindnext=0;
	    //printf("find pid for nextpid \n");
	    break;
	  }         
	it=it->next;
      }
    if(nofindnext)
    {
      struct Nodelist *M=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
      struct node *B=kmalloc(sizeof(struct node),GFP_ATOMIC);
      B->pid=nextpid;
      B->status=1;
      B->Wait=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
      B->Wait->size=0;
      B->Wait->element=NULL;
      B->Wait->next=NULL;
      B->Next=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
      B->Next->size=0;
      B->Next->element=NULL;
      B->Next->next=NULL;
      M->element=B;
      it->next=M;
      M->prev=it;
      it=it->next;
      it->next=NULL;
      N=M;
      (*list)->size++; 
    }
    NN=N->element;
    WW=W->element;
    insert(&(WW->Next),&NN);
    insert(&(NN->Wait),&WW);
    return check(*list);
}
void Remove(struct Nodelist **list,int pid)
{
    struct Nodelist *it =*list;
    struct Nodelist *thenode;
    int find=0;
    //printk("start to remove \n");
    while(it->next!=NULL)
    {
     if(it->next->element->pid==pid)
     {
      thenode=it->next;
      find=1;
      break;
     }
     it=it->next;
    }
    if(!find)
    {
      //       printk("no this pid\n");
	     return;
    }
    else
    {
      //printk("start to do thing \n");
        (*list)->size--; //total node in graph --;
        struct Nodelist *before=thenode->prev;
        struct Nodelist *after=thenode->next;
        struct node *e =thenode->element;
        struct Nodelist *itNext =e->Next;
        struct node *b;
        struct Nodelist *itWait;
        struct Nodelist *here;
        struct Nodelist *Bhere;
        struct Nodelist *Ahere;
        while(itNext->next!=NULL)//go the oher node wait list detele this node;
        {
         b=itNext->next->element;
         //printk("%d \n",b->pid);
         itWait=b->Wait;
         //printk("wtf\n"); 
         while(itWait->next!=NULL) 
         {
           //printk("here1\n");
           //printk("%d ",itWait->next->element->pid);
           if(itWait->next->element->pid==pid)
           {
            itWait->size--;
            here=itWait->next;
            Bhere=here->prev;
            if(here->next!=NULL)
            {
             struct Nodelist *Ahere=here->next;
             Ahere->prev=Bhere;
            }
            Bhere->next=here->next;
            // printk("here2\n");
            kfree(here);
            break;
           }
           itWait=itWait->next;                  
         }
         itNext=itNext->next;
        }
	//printk("hellow word \n");
        itWait=e->Wait;
	//printk("hellow word \n");
        while(itWait->next!=NULL) // go the other node next list detele this node;
        {
	  //printk("hellow word \n");
         b=itWait->next->element;
         //printk("%d \n",b->pid);
         itNext=b->Next;
         //printk("wtf\n"); 
         while(itNext->next!=NULL)
         {
           //printk("here3\n");
           //printk("%d ",itNext->next->element->pid);
           if(itNext->next->element->pid==pid)
           {
            itNext->size--;
            here=itNext->next;
            Bhere=here->prev;
            if(here->next!=NULL)
            {
             Ahere=here->next;
             Ahere->prev=Bhere;
            }
            Bhere->next=here->next;
            //printf("here\n");
            kfree(here);
            break;
           }
           itNext=itNext->next;                  
         }
         itWait=itWait->next;
        }	
	//printk("hellow word \n");
        kfree(e);
        kfree(thenode);
        before->next=after;
	if(after!=NULL)
        after->prev=before;
    }
}
////////////////////////////////////////////////////////

/* The size of an OSPRD sector. */
#define SECTOR_SIZE	512

/* This flag is added to an OSPRD file's f_flags to indicate that the file
 * is locked. */
#define F_OSPRD_LOCKED	0x80000

/* eprintk() prints messages to the console.
 * (If working on a real Linux machine, change KERN_NOTICE to KERN_ALERT or
 * KERN_EMERG so that you are sure to see the messages.  By default, the
 * kernel does not print all messages to the console.  Levels like KERN_ALERT
 * and KERN_EMERG will make sure that you will see messages.) */
#define eprintk(format, ...) printk(KERN_NOTICE format, ## __VA_ARGS__)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("CS 111 RAM Disk");
// EXERCISE: Pass your names into the kernel as the module's authors.
MODULE_AUTHOR("Yi Wang and Zhen Feng");

#define OSPRD_MAJOR	222

/* This module parameter controls how big the disk will be.
 * You can specify module parameters when you load the module,
 * as an argument to insmod: "insmod osprd.ko nsectors=4096" */
static int nsectors = 32;
module_param(nsectors, int, 0);
/*********************************************************/
static struct Nodelist *myList;
/********************************************************/


/* The internal representation of our device. */
typedef struct osprd_info {
	uint8_t *data;                  // The data array. Its size is
	                                // (nsectors * SECTOR_SIZE) bytes.

	osp_spinlock_t mutex;           // Mutex for synchronizing access to
					// this block device

	unsigned ticket_head;		// Currently running ticket for
					// the device lock

	unsigned ticket_tail;		// Next available ticket for
					// the device lock

	wait_queue_head_t blockq;       // Wait queue for tasks blocked on
					// the device lock

	/* HINT: You may want to add additional fields to help
	         in detecting deadlock. */
        unsigned numberofwrites;
        unsigned numberofreads;
        unsigned pid[1000];
/**************************************************************/
    //pid_t current_holder; // store the current lock holder pid 
    //int current_type; // 1 means read and 0 means write 
    pid_t recent_write_pid; // the recent pid for write
    pid_t recent_read_pid; // the recent pid for read.
    int flag ; //  use for check the transition between the write and read; 
    			// 0 means no continue read 1 means read is continuing.
/**************************************************************/    
	// The following elements are used internally; you don't need
	// to understand them.
	struct request_queue *queue;    // The device request queue.
	spinlock_t qlock;		// Used internally for mutual
	                                //   exclusion in the 'queue'.
	struct gendisk *gd;             // The generic disk.
} osprd_info_t;

#define NOSPRD 4
static osprd_info_t osprds[NOSPRD];


// Declare useful helper functions

/*
 * file2osprd(filp)
 *   Given an open file, check whether that file corresponds to an OSP ramdisk.
 *   If so, return a pointer to the ramdisk's osprd_info_t.
 *   If not, return NULL.
 */
static osprd_info_t *file2osprd(struct file *filp);

/*
 * for_each_open_file(task, callback, user_data)
 *   Given a task, call the function 'callback' once for each of 'task's open
 *   files.  'callback' is called as 'callback(filp, user_data)'; 'filp' is
 *   the open file, and 'user_data' is copied from for_each_open_file's third
 *   argument.
 */
static void for_each_open_file(struct task_struct *task,
			       void (*callback)(struct file *filp,
						osprd_info_t *user_data),
			       osprd_info_t *user_data);


/*
 * osprd_process_request(d, req)
 *   Called when the user reads or writes a sector.
 *   Should perform the read or write, as appropriate.
 */
static void osprd_process_request(osprd_info_t *d, struct request *req)
{
	if (!blk_fs_request(req)) {
		end_request(req, 0);
		return;
	}

	// EXERCISE: Perform the read or write request by copying data between
	// our data array and the request's buffer.
	// Hint: The 'struct request' argument tells you what kind of request
	// this is, and which sectors are being read or written.
	// Read about 'struct request' in <linux/blkdev.h>.
	// Consider the 'req->sector', 'req->current_nr_sectors', and
	// 'req->buffer' members, and the rq_data_dir() function.

	// Your code here.
	unsigned numberofsector= req->current_nr_sectors;
        unsigned sector = req->sector;
	unsigned int length = numberofsector * SECTOR_SIZE;
	//int length=1;
	//printk(" length %d \n " ,length);
	//printk("sector %d \n",sector);
	//printk("numberofsecotr %d \n ",numberofsector);
      	uint8_t * actual_location = sector * SECTOR_SIZE + d->data;
	if ( (numberofsector+sector) > nsectors )
	  {
	    eprintk("sector location is unaccessible");
	    end_request(req, -1);
	  }
	if ( (rq_data_dir(req)) == READ)
	  {
	    memcpy((void *) req->buffer, (void *) actual_location, (unsigned int) length);
	  }
	if ((rq_data_dir(req)) == WRITE)
	  {
	    memcpy((void *) actual_location, (void *) req->buffer,(unsigned int) length);
	  }
	/*if ((rq_data_dir(req)) != WRITE && (rq_data_dir(req)) != READ )
	  {
	    eprintk(" can not execute command other than read or  write");
	    end_request(req, -1);
	    }*/
	end_request(req, 1);
}


// This function is called when a /dev/osprdX file is opened.
// You aren't likely to need to change this.
static int osprd_open(struct inode *inode, struct file *filp)
{
	// Always set the O_SYNC flag. That way, we will get writes immediately
	// instead of waiting for them to get through write-back caches.
	filp->f_flags |= O_SYNC;
	return 0;
}


// This function is called when a /dev/osprdX file is finally closed.
// (If the file descriptor was dup2ed, this function is called only when the
// last copy is closed.)
static int osprd_close_last(struct inode *inode, struct file *filp)
{
	if (filp) {
		osprd_info_t *d = file2osprd(filp);
		int filp_writable = (filp->f_mode & FMODE_WRITE)!=0;

		// EXERCISE: If the user closes a ramdisk file that holds
		// a lock, release the lock.  Also wake up blocked processes
		// as appropriate.

		// Your code here.
		//printk("close file %d \n",current->pid);
		if((filp->f_flags&F_OSPRD_LOCKED)!=0)
		  {
		    // printk("release clock %d \n",current->pid);
		    if(filp_writable)
		      {
			osp_spin_lock(&d->mutex);
			d->numberofwrites=0;
		      }
		    else
		      {
			osp_spin_lock(&d->mutex);
			d->numberofreads--;
		      }
		    if(d->numberofwrites==0||d->numberofreads==0)
		      {
			filp->f_flags&=~F_OSPRD_LOCKED;
		      }
		    unsigned i=0;
		    int run=1;
		    for(i=d->ticket_tail+1;i<1000;i++)
		      {
			if(d->pid[i]!=0)
			  {
			    d->ticket_tail=i;
			    run=0;
			    break;
			  }
		      }
		    if(run)
		      {
			d->ticket_tail=d->ticket_head;
		      }
		    osp_spin_unlock(&d->mutex);
		    // printk("finish relase lock %d  \n",current->pid);
		    //printk("%d %d ",d->ticket_tail,d->ticket_head);
/*******************************************************************************/
	            osp_spin_lock(&d->mutex);
		    Remove(&myList,current->pid);
	            osp_spin_unlock(&d->mutex);
/*******************************************************************************/
		    
		    wake_up_all(&d->blockq);
		  }
		// This line avoids compiler warnings; you may remove it.
		//(void) filp_writable, (void) d;
		//wake_up_all(&d->blockq);
		//printk("close file %d \n",current->pid);
	}
	return 0;
}


/*
 * osprd_lock
 */

/*
 * osprd_ioctl(inode, filp, cmd, arg)
 *   Called to perform an ioctl on the named file.
 */
int osprd_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	osprd_info_t *d = file2osprd(filp);	// device info
	int r = 0;			// return value: initially 0

	// is file open for writing?
	int filp_writable = (filp->f_mode & FMODE_WRITE) != 0;

	// This line avoids compiler warnings; you may remove it.
	//(void) filp_writable, (void) d;

	// Set 'r' to the ioctl's return value: 0 on success, negative on error
	unsigned local_ticket;
	if (cmd == OSPRDIOCACQUIRE) {

		// EXERCISE: Lock the ramdisk.
		//
		// If *filp is open for writing (filp_writable), then attempt
		// to write-lock the ramdisk; otherwise attempt to read-lock
		// the ramdisk.
		//
                // This lock request must block using 'd->blockq' until:
		// 1) no other process holds a write lock;
		// 2) either the request is for a read lock, or no other process
		//    holds a read lock; and
		// 3) lock requests should be serviced in order, so no process
		//    that blocked earlier is still blocked waiting for the
		//    lock.
		//
		// If a process acquires a lock, mark this fact by setting
		// 'filp->f_flags |= F_OSPRD_LOCKED'.  You also need to
		// keep track of how many read and write locks are held:
		// change the 'osprd_info_t' structure to do this.
		//
		// Also wake up processes waiting on 'd->blockq' as needed.
		//
		// If the lock request would cause a deadlock, return -EDEADLK.
		// If the lock request blocks and is awoken by a signal, then
		// return -ERESTARTSYS.
		// Otherwise, if we can grant the lock request, return 0.

		// 'd->ticket_head' and 'd->ticket_tail' should help you
		// service lock requests in order.  These implement a ticket
		// order: 'ticket_tail' is the next ticket, and 'ticket_head'
		// is the ticket currently being served.  You should set a local
		// variable to 'd->ticket_head' and increment 'd->ticket_head'.
		// Then, block at least until 'd->ticket_tail == local_ticket'.
		// (Some of these operations are in a critical section and must
		// be protected by a spinlock; which ones?)

		// Your code here (instead of the next two lines).
	  
	  osp_spin_lock(&d->mutex); //lock for get ticket;
	  local_ticket=d->ticket_head; // process get a ticket number
	  // printk("get lock %d  %d %d \n",current->pid,d->ticket_head,d->ticket_tail);
	  d->pid[local_ticket]=current->pid;
//////////////////////////////////////////////////////////
	  if ( local_ticket ==0 )
	  {
	  	//d->current_holder = current->pid;
	  	//d->current_type= (flip_writable) ?0:1; // 1 means read and 0 means write 
	    d->flag = (filp_writable)?0:1;
	    if (d->flag)
	      {
		d->recent_read_pid= current->pid;
		//printk("recent_read_pid is %d ",d->recent_read_pid );
	      }
	    else 
	      {
		d->recent_write_pid = current->pid;
		//printk("recent_write_read_pid is %d ",d->recent_write_pid );
	      }
	  }
	  else {
	    int now_type = (filp_writable) ?0:1 ;
	    //printk("111111111111111111111111111111111111111111111111111111\n");
	    if (d->flag)
	      {
		if (now_type )
		  {
		    // call you function here current-pid wait for recent_write_pid;
		    //printk(" read and read \n");
		    // printk("recent_read_pid is %d \n",d->recent_read_pid );
		    //printk("current pid is %d ",current->pid );
		    if(Insert(&myList,d->recent_write_pid,current->pid))
		      {
			//printf("deadlock \n");
			Remove(&myList, d->recent_write_pid);
			Remove(&myList,current->pid);
			osp_spin_unlock(&d->mutex);
			return -EDEADLK;
		      }
		    else
		      {
			//printk("first\n");
		      }
		    d->recent_read_pid= current->pid;
		  }
		else 
		  {
		    // call you function here current-pid wait for recent_read_pid;
		    // printk("pid is %d current ->pid is %d \n",d->recent_read_pid,current->pid);
		    // printk(" read and write \n");
		    // printk("recent_read_pid is %d \n",d->recent_read_pid );
		    //printk("current pid is %d ",current->pid );
		    if(Insert(&myList,d->recent_read_pid,current->pid))
		      {
			//printf("deadlock \n");
			Remove(&myList, d->recent_read_pid);
			Remove(&myList,current->pid);
			osp_spin_unlock(&d->mutex);
			return -EDEADLK;
		      }
		    else
		      {
			//printk("second \n");
		      }
		    d->recent_write_pid = current->pid;
		    d->flag =0;
		  }
	      }
	    else
	      {
		if (now_type )
		  {
		    // call you function here current-pid wait for recent_write_pid;
		    //printk("write and read \n");
		    //printk("recent_write_pid is %d \n",d->recent_write_pid );
		    // printk("current pid is %d ",current->pid );
		    if(Insert(&myList,d->recent_write_pid,current->pid))
		      {
			//printf("deadlock \n");
			Remove(&myList, d->recent_write_pid);
			Remove(&myList,current->pid);
			osp_spin_unlock(&d->mutex);
			return -EDEADLK;
		      }
		    else
		      {
			//printk("third\n");
		      }
		    
		    d->flag=1;
		    d->recent_read_pid=current->pid;
		  }
		else
		  {
		    // call you function here current-pid wait for recent_write_pid;
		    //printk("write and read write \n");
		    //printk("recent_write_pid is %d \n",d->recent_write_pid );
		    //printk("current pid is %d ",current->pid );
		    if(Insert(&myList,d->recent_write_pid,current->pid))
		      {
			//printk("deadlock \n");
			Remove(&myList, d->recent_write_pid);
			Remove(&myList,current->pid);
			osp_spin_unlock(&d->mutex);
			return -EDEADLK;
		      }
		    else
		      {
			//printk("last \n");
		      }
		    d->flag =0;
		    d->recent_write_pid = current->pid;
		  }
	      }
	  }
/*************************************************/
	  d->ticket_head++;   //ticket increase ;
	  osp_spin_unlock(&d->mutex); // unlock for get ticket;
	  if(filp_writable) //need to get a write lock
	    {
	      r=wait_event_interruptible(d->blockq,local_ticket==d->ticket_tail&&d->numberofwrites==0&&d->numberofreads==0);
	      /*********************************************************/
	      /*********************************************************/
	      if(r==-ERESTARTSYS)
	       {
		 int i=0;
		 osp_spin_lock(&d->mutex); 
		 for(i=d->ticket_tail;i<1000;i++)
		   {
		     if(d->pid[i]==current->pid)
		       {
			 	d->pid[i]=0;
		       }
		   }
		 osp_spin_unlock(&d->mutex); 
		 return r;
	       }
	      osp_spin_lock(&d->mutex);
	      d->numberofwrites=1;
	      // printk("get wlock %d \n",current->pid);
	    } 
	  else //need to get a read lock
	    {
	     r=wait_event_interruptible(d->blockq,local_ticket==d->ticket_tail&&d->numberofwrites==0);
	     /***************************************************************/
	     /***************************************************************/
	     if(r==-ERESTARTSYS)
	       {
		 //osp_spin_lock(&d->mutex); 
		 //d->ticket_tail++;
		 //osp_spin_unlock(&d->mutex); 
		int i=0;
		 osp_spin_lock(&d->mutex); 
		 for(i=d->ticket_tail;i<1000;i++)
		   {
		     if(d->pid[i]==current->pid)
		       {
			 d->pid[i]=0;
		       }
		   }
		 osp_spin_unlock(&d->mutex); 
		 return r;
	       }
	      osp_spin_lock(&d->mutex);
	      d->numberofreads++;
	      //printk("get rlock %d \n",current->pid);
	    }
	  filp->f_flags |= F_OSPRD_LOCKED; //lock the file
	  //d->ticket_tail++;
	  osp_spin_unlock(&d->mutex);
	  //printk("finish get lock %d \n",current->pid);
	} else if (cmd == OSPRDIOCTRYACQUIRE) {

		// EXERCISE: ATTEMPT to lock the ramdisk.
		//
		// This is just like OSPRDIOCACQUIRE, except it should never
		// block.  If OSPRDIOCACQUIRE would block or return deadlock,
		// OSPRDIOCTRYACQUIRE should return -EBUSY.
		// Otherwise, if we can grant the lock request, return 0.

		// Your code here (instead of the next two lines).
	  osp_spin_lock(&d->mutex); //lock for get ticket;
	  // local_ticket=d->ticket_head; // process get a ticket number 
	  //d->ticket_head++;   //ticket increase ;
	  osp_spin_unlock(&d->mutex); // unlock for get ticket;
	  if(filp_writable) //need to get a write lock
	    {
	      osp_spin_lock(&d->mutex);
	      if(d->numberofwrites!=0||d->numberofreads!=0)
		{
		  // d->ticket_head--;
		  osp_spin_unlock(&d->mutex);
		  return -EBUSY;
		}
	      //d->numberofwrites=1;
	    } 
	  else //need to get a read lock
	    {
	      osp_spin_lock(&d->mutex);
	      if(d->numberofwrites!=0)
		{
		  //d->ticket_head--;
		  osp_spin_unlock(&d->mutex);
		  return -EBUSY;
       		}
	      // d->numberofreads++;
	    }
	  //filp->f_flags |= F_OSPRD_LOCKED; //lock the file
	  //d->ticket_tail++;
	  osp_spin_unlock(&d->mutex);
	  // printk("finish get lock %d \n",current->pid);

	} else if (cmd == OSPRDIOCRELEASE) {

		// EXERCISE: Unlock the ramdisk.
		//
		// If the file hasn't locked the ramdisk, return -EINVAL.
		// Otherwise, clear the lock from filp->f_flags, wake up
		// the wait queue, perform any additional accounting steps
		// you need, and return 0.

		// Your code here (instead of the next line).
		eprintk("release clock \n");
		if((filp->f_flags&F_OSPRD_LOCKED)!=0)
		  {
		    if(filp_writable)
		      {
			osp_spin_lock(&d->mutex);
			d->numberofwrites=0;
		      }
		    else
		      {
			osp_spin_lock(&d->mutex);
			d->numberofreads--;
		      }
		    if(d->numberofwrites==0||d->numberofreads==0)
		      {
			filp->f_flags&=~F_OSPRD_LOCKED;
		      }
		    osp_spin_unlock(&d->mutex);
		  }
		else
		  {
		    return -EINVAL;
		  }
		eprintk("finish relase lock \n");
		// This line avoids compiler warnings; you may remove it.
		//(void) filp_writable, (void) d;
/**************************************************************/
		osp_spin_lock(&d->mutex);
		Remove(&myList,current->pid);
		osp_spin_unlock(&d->mutex);
/*************************************************************/
		wake_up_all(&d->blockq);
	} else
		r = -ENOTTY; /* unknown command */
	return r;
}


// Initialize internal fields for an osprd_info_t.

static void osprd_setup(osprd_info_t *d)
{
	/* Initialize the wait queue. */
	init_waitqueue_head(&d->blockq);
	osp_spin_lock_init(&d->mutex);
	d->ticket_head = d->ticket_tail = 0;
	d->numberofreads=d->numberofwrites=0;
/****************************************************/
	//current_holder =-1; // store the current lock holder pid 
    //current_type =-1; // 1 means read and 0 means write 
   	d->recent_write_pid = 0;
   	d->recent_read_pid = 0;
   	d->flag = 0;
	myList=kmalloc(sizeof(struct Nodelist),GFP_ATOMIC);
	myList->size=0;
	myList->element=NULL;
	myList->next=NULL;
/********************************************************/ 
	/* Add code here if you add fields to osprd_info_t. */
}


/*****************************************************************************/
/*         THERE IS NO NEED TO UNDERSTAND ANY CODE BELOW THIS LINE!          */
/*                                                                           */
/*****************************************************************************/

// Process a list of requests for a osprd_info_t.
// Calls osprd_process_request for each element of the queue.

static void osprd_process_request_queue(request_queue_t *q)
{
	osprd_info_t *d = (osprd_info_t *) q->queuedata;
	struct request *req;

	while ((req = elv_next_request(q)) != NULL)
		osprd_process_request(d, req);
}


// Some particularly horrible stuff to get around some Linux issues:
// the Linux block device interface doesn't let a block device find out
// which file has been closed.  We need this information.

static struct file_operations osprd_blk_fops;
static int (*blkdev_release)(struct inode *, struct file *);

static int _osprd_release(struct inode *inode, struct file *filp)
{
	if (file2osprd(filp))
		osprd_close_last(inode, filp);
	return (*blkdev_release)(inode, filp);
}

static int _osprd_open(struct inode *inode, struct file *filp)
{
	if (!osprd_blk_fops.open) {
		memcpy(&osprd_blk_fops, filp->f_op, sizeof(osprd_blk_fops));
		blkdev_release = osprd_blk_fops.release;
		osprd_blk_fops.release = _osprd_release;
	}
	filp->f_op = &osprd_blk_fops;
	return osprd_open(inode, filp);
}


// The device operations structure.

static struct block_device_operations osprd_ops = {
	.owner = THIS_MODULE,
	.open = _osprd_open,
	// .release = osprd_release, // we must call our own release
	.ioctl = osprd_ioctl
};


// Given an open file, check whether that file corresponds to an OSP ramdisk.
// If so, return a pointer to the ramdisk's osprd_info_t.
// If not, return NULL.

static osprd_info_t *file2osprd(struct file *filp)
{
	if (filp) {
		struct inode *ino = filp->f_dentry->d_inode;
		if (ino->i_bdev
		    && ino->i_bdev->bd_disk
		    && ino->i_bdev->bd_disk->major == OSPRD_MAJOR
		    && ino->i_bdev->bd_disk->fops == &osprd_ops)
			return (osprd_info_t *) ino->i_bdev->bd_disk->private_data;
	}
	return NULL;
}


// Call the function 'callback' with data 'user_data' for each of 'task's
// open files.

static void for_each_open_file(struct task_struct *task,
		  void (*callback)(struct file *filp, osprd_info_t *user_data),
		  osprd_info_t *user_data)
{
	int fd;
	task_lock(task);
	spin_lock(&task->files->file_lock);
	{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 13)
		struct files_struct *f = task->files;
#else
		struct fdtable *f = task->files->fdt;
#endif
		for (fd = 0; fd < f->max_fds; fd++)
			if (f->fd[fd])
				(*callback)(f->fd[fd], user_data);
	}
	spin_unlock(&task->files->file_lock);
	task_unlock(task);
}


// Destroy a osprd_info_t.

static void cleanup_device(osprd_info_t *d)
{
	wake_up_all(&d->blockq);
	if (d->gd) {
		del_gendisk(d->gd);
		put_disk(d->gd);
	}
	if (d->queue)
		blk_cleanup_queue(d->queue);
	if (d->data)
		vfree(d->data);
}


// Initialize a osprd_info_t.

static int setup_device(osprd_info_t *d, int which)
{
	memset(d, 0, sizeof(osprd_info_t));

	/* Get memory to store the actual block data. */
	if (!(d->data = vmalloc(nsectors * SECTOR_SIZE)))
		return -1;
	memset(d->data, 0, nsectors * SECTOR_SIZE);

	/* Set up the I/O queue. */
	spin_lock_init(&d->qlock);
	if (!(d->queue = blk_init_queue(osprd_process_request_queue, &d->qlock)))
		return -1;
	blk_queue_hardsect_size(d->queue, SECTOR_SIZE);
	d->queue->queuedata = d;

	/* The gendisk structure. */
	if (!(d->gd = alloc_disk(1)))
		return -1;
	d->gd->major = OSPRD_MAJOR;
	d->gd->first_minor = which;
	d->gd->fops = &osprd_ops;
	d->gd->queue = d->queue;
	d->gd->private_data = d;
	snprintf(d->gd->disk_name, 32, "osprd%c", which + 'a');
	set_capacity(d->gd, nsectors);
	add_disk(d->gd);

	/* Call the setup function. */
	osprd_setup(d);

	return 0;
}

static void osprd_exit(void);


// The kernel calls this function when the module is loaded.
// It initializes the 4 osprd block devices.

static int __init osprd_init(void)
{
	int i, r;

	// shut up the compiler
	(void) for_each_open_file;
#ifndef osp_spin_lock
	(void) osp_spin_lock;
	(void) osp_spin_unlock;
#endif

	/* Register the block device name. */
	if (register_blkdev(OSPRD_MAJOR, "osprd") < 0) {
		printk(KERN_WARNING "osprd: unable to get major number\n");
		return -EBUSY;
	}

	/* Initialize the device structures. */
	for (i = r = 0; i < NOSPRD; i++)
		if (setup_device(&osprds[i], i) < 0)
			r = -EINVAL;
	if (r < 0) {
		printk(KERN_EMERG "osprd: can't set up device structures\n");
		osprd_exit();
		return -EBUSY;
	} else
		return 0;
}


// The kernel calls this function to unload the osprd module.
// It destroys the osprd devices.

static void osprd_exit(void)
{
	int i;
	for (i = 0; i < NOSPRD; i++)
		cleanup_device(&osprds[i]);
	unregister_blkdev(OSPRD_MAJOR, "osprd");
}


// Tell Linux to call those functions at init and exit time.
module_init(osprd_init);
module_exit(osprd_exit);
