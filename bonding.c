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

typedef struct thread_info{
  int id;
  pthread_cond_t *condition;
  int h1Id;
  int h2Id;
  int oId;
} Thread_Info;

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

  if(dll_empty(g->hydrogen) == 1 || dll_empty(g->oxygen) == 1){
    dll_append(g->hydrogen, new_jval_v(thisThread));
    pthread_cond_wait(thisThread->condition, g->lock);
  }else{
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

    pthread_cond_signal(h2->condition);
    pthread_cond_signal(o->condition);
  }

  pthread_mutex_unlock(g->lock);
  char *result = Bond(thisThread->h1Id, thisThread->h2Id, thisThread->oId);
  free(thisThread);

  return result;
}

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

  if(dll_empty(g->hydrogen) || dll_next(dll_first(g->hydrogen)) == g->hydrogen){
    dll_append(g->hydrogen, new_jval_v(thisThread));
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

    pthread_cond_signal(h1->condition);
    pthread_cond_signal(h2->condition);
  }

  pthread_mutex_unlock(g->lock);
  char *result = Bond(thisThread->h1Id, thisThread->h2Id, thisThread->oId);
  free(thisThread);
  
  return result; //?? idk do I need to return this who knows??
}
