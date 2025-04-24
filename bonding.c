//Lab A: Bonding
//Madelyn Gross
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bonding.h"
#include "dllist.h"

/* This solution compiles and hangs. */
typedef struct global_info{
  pthread_mutex_t *lock;
  Dllist hydrogen;//lists of thread_infos NOT bonding_args
  Dllist oxygen;
} Global_Info;

//Each oxygen and hydrogen will have one of these
typedef struct thread_info{
  int id;
  pthread_cond_t *condition;
  int h1Id;
  int h2Id;
  int oId;
} Thread_Info;

//just initializing the global info because ig they thought this was important
void *initialize_v(char *verbosity)
{
  Global_Info *info = malloc(sizeof(struct global_info));

  info->lock = new_mutex();
  info->hydrogen = new_dllist();
  info->oxygen = new_dllist();

  return (void *)info;
}

void *hydrogen(void *arg)
{
  struct bonding_arg *a = (struct bonding_arg *)arg;
  struct global_info *g = (struct global_info *)a->v;
  Dllist ptr;

  pthread_mutex_lock(g->lock);

  Thread_Info *thisThread = malloc(sizeof(Thread_Info));
  thisThread->id = a->id;
  thisThread->condition = new_cond();
  thisThread->h1Id = -1;
  thisThread->h2Id = -1;
  thisThread->oId = -1;

  //do both list have something in them? if not add to the list and wait
  if(dll_empty(g->hydrogen) == 1 || dll_empty(g->oxygen) == 1){
    dll_append(g->hydrogen, new_jval_v(thisThread));
    pthread_cond_wait(thisThread->condition, g->lock);
  }else{
    //we have what we need so we can bond
    //getting the first hydrogen and oxygen in each list
    ptr = dll_first(g->hydrogen);
    Thread_Info *h2 = (Thread_Info *)ptr->val.v;
    dll_delete_node(ptr);
    
    ptr = dll_first(g->oxygen);
    Thread_Info *o = (Thread_Info *)ptr->val.v;
    dll_delete_node(ptr);

    //assigning the correct ids for this thread;
    thisThread->h1Id = thisThread->id;
    thisThread->h2Id = h2->id;
    thisThread->oId = o->id;

    //for the second hydrogen
    h2->h1Id = thisThread->id;
    h2->h2Id = h2->id;
    h2->oId = o->id;

    //for the oxygen
    o->h1Id = thisThread->id;
    o->h2Id = h2->id;
    o->oId = o->id;

    //signal the other threads conditions to wake up
    pthread_cond_signal(h2->condition);
    pthread_cond_signal(o->condition);
  }

  //unlock must happen first but then call 
  pthread_mutex_unlock(g->lock);
  char *result = Bond(thisThread->h1Id, thisThread->h2Id, thisThread->oId);
  free(thisThread);

  return result;
}

//basically the same as hydrogen just a little bit different logic
void *oxygen(void *arg)
{
  struct bonding_arg *a = (struct bonding_arg *)arg;
  struct global_info *g = (struct global_info *)a->v;

  Dllist ptr;
  pthread_mutex_lock(g->lock);

  Thread_Info *thisThread = malloc(sizeof(Thread_Info));
  thisThread->id = a->id;
  thisThread->condition = new_cond();
  thisThread->h1Id = -1;
  thisThread->h2Id = -1;
  thisThread->oId = -1;

  //now we check if the hydrogen list has 2 elements in it? if not append to the oxygen list
  if(dll_empty(g->hydrogen) || dll_next(dll_first(g->hydrogen)) == g->hydrogen){
    dll_append(g->oxygen, new_jval_v(thisThread));//append it to oxygen list silly
    pthread_cond_wait(thisThread->condition, g->lock);
  }else{
    ptr = dll_first(g->hydrogen);
    Thread_Info *h1 = (Thread_Info *)ptr->val.v;
    dll_delete_node(ptr);

    ptr = dll_first(g->hydrogen);
    Thread_Info *h2 = (Thread_Info *)ptr->val.v;
    dll_delete_node(ptr);
    
    //assigning the correct ids for this thread;
    //we are in the oxygen now!!!
    thisThread->h1Id = h1->id;
    thisThread->h2Id = h2->id;
    thisThread->oId = thisThread->id;

    //for the first hydrogen
    h1->h1Id = h1->id;
    h1->h2Id = h2->id;
    h1->oId = thisThread->id;

    //for the second hydrogen
    h2->h1Id = h1->id;
    h2->h2Id = h2->id;
    h2->oId = thisThread->id;

    //signal the other threads 
    pthread_cond_signal(h1->condition);
    pthread_cond_signal(h2->condition);
  }

  //this must be unlocked first
  pthread_mutex_unlock(g->lock);
  char *result = Bond(thisThread->h1Id, thisThread->h2Id, thisThread->oId);
  free(thisThread->condition);
  free(thisThread);
  
  return result; //return this because it will print this out to stdout at some point I think
}
