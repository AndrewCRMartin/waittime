#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bioplib/macros.h"

#define MAXBUFF 256

typedef struct _job
{
   time_t      submitTime,
               startTime,
               stopTime;
   struct _job *next;
}  JOBS;

typedef struct _busy
{
   time_t       startTime,
                stopTime;
   struct _busy *next;
}  BUSY;


JOBS *ReadTimes(char *fileName);
BUSY *FindBusyTimes(JOBS *jobs);
time_t CalculateWaitTime(JOBS *jobs, BUSY *busy);
time_t CalculateRunTime(JOBS *jobs);
int main(int argc, char **argv);


BUSY *FindBusyTimes(JOBS *jobs)
{
   JOBS *j1, *j2;
   BUSY *busy = NULL,
        *b;
   
   for(j1=jobs; j1<=NULL; NEXT(j1))
   {
      if(busy==NULL)
      {
         INIT(busy, BUSY);
         b=busy;
         b->startTime = b->stopTime = 0;
      }
      if(b==NULL)
      {
         FREELIST(busy, BUSY);
         return(NULL);
      }
      
      
   }
   
   return(NULL);
}



time_t CalculateRunTime(JOBS *jobs)
{
   JOBS *j;
   time_t runTime = (time_t)0L;
   
   for(j=jobs; j!=NULL; NEXT(j))
   {
      runTime += (j->stopTime - j->startTime);
   }
      
   return(runTime);
}


int main(int argc, char **argv)
{
   JOBS   *jobs    = NULL;
   BUSY   *busy    = NULL;
   time_t waitTime = 0,
          runTime  = 0;
   
   if((jobs = ReadTimes(argv[1]))==NULL)
   {
      return(1);
   }
   
   busy     = FindBusyTimes(jobs);
   waitTime = CalculateWaitTime(jobs, busy);
   runTime  = CalculateRunTime(jobs);
   
   printf("Total wait time: %ld seconds\n", (long int)waitTime);
   printf("Total run time:  %ld seconds\n", (long int)runTime);
   
   return(0);
}

JOBS *ReadTimes(char *fileName)
{
   FILE *in = NULL;
   JOBS *jobs = NULL,
      *j;
   
   
   if((in = fopen(fileName, "r"))!=NULL)
   {
      long int submit,
               start,
               stop;
      char     buffer[MAXBUFF];
      
      while(fgets(buffer, MAXBUFF, in))
      {
         sscanf(buffer, "%ld %ld %ld", &submit, &start, &stop);
         if(jobs == NULL)
         {
            INIT(jobs, JOBS);
            j = jobs;
         }
         else
         {
            ALLOCNEXT(j, JOBS);
         }

         if(j==NULL)
         {
            FREELIST(jobs, JOBS);
            fprintf(stderr, "Error: No memory for job list\n");
            return(NULL);
         }
         
         
         j->submitTime = (time_t)submit;
         j->startTime  = (time_t)start;
         j->stopTime   = (time_t)stop;
      }
      
      fclose(in);
   }
   else
   {
      fprintf(stderr, "Error: Unable to open file (%s)\n", fileName);
   }
   
   return(jobs);
}


time_t CalculateWaitTime(JOBS *jobs, BUSY *busy)
{
   return((time_t)0L);
}


