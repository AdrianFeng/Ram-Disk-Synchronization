#include <stdio.h>
#include <stdlib.h>
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
void PrintNode(struct Nodelist *list)
{
     int n=0;
     struct Nodelist *it =list;
     while(it)
     {
              printf("index %d pid is %d   he wait %d process and %d process wait him\n his status is %d \n",n++,it->element->pid,it->element->Wait->size,it->element->Next->size,it->element->status);
              struct Nodelist * waitlist=it->element->Wait;
              printf("this is what he wait \n");
              while(waitlist->next)
              {
               printf("%d ",waitlist->next->element->pid);                    
               waitlist=waitlist->next;                
              }
              printf("\n");
              struct Nodelist * nextlist=it->element->Next;
              printf("this is what pid need to wait him \n");
              while(nextlist->next)
              {
               printf("%d ",nextlist->next->element->pid);
               nextlist=nextlist->next;
              }
              printf("\n");
              it=it->next;
     }
}
void insert(struct Nodelist **list, struct node **A)
{
    struct Nodelist *it =*list;
    while(it->next!=NULL)
    {
     it=it->next;
    }
    struct Nodelist *L=malloc(sizeof(struct Nodelist),GFP_ATOMIC);
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
   PrintNode(list->next);
    while(it->next!=NULL)
    {
     if(it->next->element->status==1)
     {
      printf("dead");
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
     }
     if(it->next->element->pid==nextpid)
     {
      N=it->next;
  
      nofindnext=0;
      //printf("find pid for nextpid \n");
     }         
     if(!nofindnext&&!nofindwait)
     {
      break;
     }
     //printf("it %d pid\n",it->next->element->pid);
     it=it->next;
    }
    if(nofindwait)
    {
      struct Nodelist *L=malloc(sizeof(struct Nodelist));
      struct node *A=malloc(sizeof(struct node));
      A->pid=waitpid;
      A->status=1;
      A->Wait=malloc(sizeof(struct Nodelist));
      A->Wait->size=0;
      A->Wait->element=NULL;
      A->Wait->next=NULL;
      A->Next=malloc(sizeof(struct Nodelist));
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
    if(nofindnext)
    {
      struct Nodelist *M=malloc(sizeof(struct Nodelist));
      struct node *B=malloc(sizeof(struct node));
      B->pid=nextpid;
      B->status=1;
      B->Wait=malloc(sizeof(struct Nodelist));
      B->Wait->size=0;
      B->Wait->element=NULL;
      B->Wait->next=NULL;
      B->Next=malloc(sizeof(struct Nodelist));
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
    printf("start to remove \n");
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
             printf("no this pid\n");
    }
    else
    {
        //printf("start to do thing \n");
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
        // printf("%d \n",b->pid);
         itWait=b->Wait;
         //printf("wtf\n"); 
         while(itWait->next!=NULL) 
         {
          // printf("here\n");
           //printf("%d ",itWait->next->element->pid);
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
            //printf("here\n");
            free(here);
            break;
           }
           itWait=itWait->next;                  
         }
         itNext=itNext->next;
        }
        itWait=e->Wait;
        while(itWait->next!=NULL) // go the other node next list detele this node;
        {
         b=itWait->next->element;
         //printf("%d \n",b->pid);
         itNext=b->Next;
         //printf("wtf\n"); 
         while(itNext->next!=NULL)
         {
           //printf("here\n");
           //printf("%d ",itNext->next->element->pid);
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
            free(here);
            break;
           }
           itNext=itNext->next;                  
         }
         itWait=itWait->next;
        }
        free(e);
        free(thenode);
        before->next=after;
        after->prev=before;
    }
}
/*int main()
{   
    struct Nodelist *myList=malloc(sizeof(struct Nodelist));
    myList->size=0;
    myList->element=NULL;
    myList->next=NULL;
    if(Insert(&myList,3000,2000))
    {
     printf("deadlock \n");
    }
    else
    {
        printf("no deadlock \n");
    }
    if(Insert(&myList,2000,1000))
    {
     printf("deadlock \n");
    }
    else
    {
        printf("no deadlock \n");
    }
    if(Insert(&myList,1000,3000))
    {
     printf("deadlock \n");
    }
    else
    {
        printf("no deadlock \n");
    }
   
    //Insert(&myList,1000,2000);
    //PrintNode(myList->next);
    //Remove(&myList,3000);
    //PrintNode(myList->next);
    system("pause");
    return 0;
}*/
