#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include "bonding.h"
#include "dllist.h"

/* This solution compiles and hangs. */
struct global_info{
  pthread_mutex_t *lock;
  Dllist hydrogen;
  Dllist oxygen;
}Global_Info;

struct thread_info{
  int id;
  pthread_cond_t *condition;
  int h1Id;
  int h2Id;
  int oId;
}Thread_Info;

void *initialize_v(char *verbosity)
{
  return NULL;
}

void *hydrogen(void *arg)
{
  return NULL;
}

void *oxygen(void *arg)
{
  return NULL;
}
