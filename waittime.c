#define DEBUG 1
/************************************************************************/
/**

   Program:    waittime
   \file       waittime.c
   
   \version    V1.0
   \date       18.11.16   
   \brief      Calculates total wallclock running time and wait time of a 
               set of jobs
   
   \copyright  (c) UCL / Dr. Andrew C. R. Martin 2016
   \author     Dr. Andrew C. R. Martin
   \par
               Institute of Structural & Molecular Biology,
               University College,
               Gower Street,
               London.
               WC1E 6BT.
   \par
               andrew@bioinf.org.uk
               andrew.martin@ucl.ac.uk
               
**************************************************************************

   This program is not in the public domain, but it may be copied
   according to the conditions laid out in the accompanying file
   COPYING.DOC

   The code may be modified as required, but any modifications must be
   documented so that the person responsible can be identified.

   The code may not be sold commercially or included as part of a 
   commercial product except as described in the file COPYING.DOC.

**************************************************************************

   Description:
   ============

   Calculates total wallclock running time and wait time of a set of jobs.

**************************************************************************

   Usage:
   ======

   waittime datafile

   Datafile contains rows of three fields:

   submitTime  startTime   stopTime


**************************************************************************

   Revision History:
   =================

   V1.0    18.11.16   Original   By: ACRM

*************************************************************************/
/* Includes
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "bioplib/macros.h"
#include "bioplib/SysDefs.h"


/************************************************************************/
/* Defines and macros
*/
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


/************************************************************************/
/* Globals
*/

/************************************************************************/
/* Prototypes
*/
JOBS *ReadTimes(char *fileName);
BUSY *FindBusyTimes(JOBS *jobs);
time_t CalculateWaitTime(JOBS *jobs, BUSY *busy);
time_t CalculateRunTime(JOBS *jobs);
time_t CalculateBusyTime(BUSY *busy);
int main(int argc, char **argv);
void PrintBusy(BUSY *busy);
void UsageDie(void);


/************************************************************************/
/*>int main(int argc, char **argv)
   -------------------------------
*//**
   Main program

-  18.11.16 Original   By: ACRM
*/
int main(int argc, char **argv)
{
   JOBS   *jobs    = NULL;
   BUSY   *busy    = NULL;
   time_t waitTime = (time_t)0L,
          busyTime = (time_t)0L,
          runTime  = (time_t)0L;
   
   if((argc != 2) && !strncmp(argv[1], "-h", 2))
      UsageDie();
   
   if((jobs = ReadTimes(argv[1]))==NULL)
      return(1);
   
   if((busy = FindBusyTimes(jobs))==NULL)
      return(1);
      

#ifdef DEBUG
   PrintBusy(busy);
#endif

   busyTime = CalculateBusyTime(busy);
   runTime  = CalculateRunTime(jobs);

   /* Note that this machine modifies the busy linked list              */
   waitTime = CalculateWaitTime(jobs, busy);
   
   printf("Total wait time: %lu seconds\n", (ULONG)waitTime);
   printf("Total run time:  %lu seconds\n", (ULONG)runTime);
   printf("Total busy time: %lu seconds\n", (ULONG)busyTime);
   
   return(0);
}


/************************************************************************/
/*>void UsageDie(void)
   ----------------
*//**
   Prints a usage message and exits

-  18.11.16 Original   By: ACRM
*/
void UsageDie(void)
{
   printf("\nwaittime V1.0 (c) UCL, 2016\n");
   printf("\nUsage: waittime times.dat\n");
   printf("\nCalculates the perceived wait time for a set of jobs where \
the time is\n");
   printf("taken only as the time between submitting a job and having \
something\n");
   printf("running on the machine.\n");

   exit(0);
}


/************************************************************************/
/*>time_t CalculateRunTime(JOBS *jobs)
   -----------------------------------
*//**
   \param[in]  jobs   Linked list of jobs
   \return            The total run time of the jobs

   Calculates the total run time of all jobs

-  18.11.16 Original   By: ACRM
*/
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


/************************************************************************/
/*>time_t CalculateBusyTime(BUSY *busy)
   ------------------------------------
*//**
   \param[in]   busy   The linked list of busy times
   \return             The total busy time

   Calculates the total busy time

-  18.11.16 Original   By: ACRM
*/
time_t CalculateBusyTime(BUSY *busy)
{
   BUSY *b;
   time_t busyTime = (time_t)0L;
   
   for(b=busy; b!=NULL; NEXT(b))
   {
      busyTime += (b->stopTime - b->startTime);
   }
      
   return(busyTime);
}


/************************************************************************/
/*>BUSY *FindBusyTimes(JOBS *jobs)
   -------------------------------
*//**
   \param[in]   jobs   Linked list of jobs
   \return             Linked list of busy periods from the jobs

   Copies the start and end times from the jobs and merges overlapping
   job times so we just have a linked list of busy periods

-  18.11.16 Original   By: ACRM
*/
BUSY *FindBusyTimes(JOBS *jobs)
{
   JOBS *j;
   BUSY *busy = NULL,
      *b, *b1, *b2;
   BOOL changed = FALSE;
   

   /* Copy the start/stop times to the busy linked list                 */
   for(j=jobs; j!=NULL; NEXT(j))
   {
      if(busy==NULL)
      {
         INIT(busy, BUSY);
         b =busy;
      }
      else
      {
         ALLOCNEXT(b, BUSY);
      }
      if(b==NULL)
      {
         FREELIST(busy, BUSY);
         fprintf(stderr,"Error: no memory for busy times linked list\n");
         return(NULL);
      }
      b->startTime = j->startTime;
      b->stopTime  = j->stopTime;
   }

   /* Now iteratively delete overlapping time spans                     */
   do 
   {
      changed = FALSE;

      for(b1=busy; b1!=NULL; NEXT(b1))
      {
         for(b2=b1->next; b2!=NULL; NEXT(b2))
         {
            if((b2->startTime >= b1->startTime) &&   /* 11111111111     */
               (b2->startTime <= b1->stopTime)  &&   /*     22222222222 */
               (b2->stopTime  >= b1->stopTime))
            {
               b1->stopTime = b2->stopTime;
               DELETE(busy, b2, BUSY);
               changed = TRUE;
               goto nextloop;
            }
            else if((b2->startTime <= b1->startTime) &&  /*    11111111 */
                    (b2->stopTime  >= b1->startTime) &&  /* 222222222   */
                    (b2->stopTime  <= b1->stopTime))
            {
               b1->startTime = b2->startTime;
               DELETE(busy, b2, BUSY);
               changed = TRUE;
               goto nextloop;
            }
            else if((b2->startTime >= b1->startTime) &&  /* 11111111111 */
                    (b2->stopTime  <= b1->stopTime))     /*   2222222   */
            {
               DELETE(busy, b2, BUSY);
               changed = TRUE;
               goto nextloop;
            }
            else if((b1->startTime >= b2->startTime) &&  /*   1111111   */
                    (b1->stopTime  <= b2->stopTime))     /* 22222222222 */
            {
               DELETE(busy, b1, BUSY);
               changed = TRUE;
               goto nextloop;
            }
         }
nextloop: 
         continue;
      }
   }  while(changed);
   
   return(busy);
}


/************************************************************************/
/*>void PrintBusy(BUSY *busy)
   --------------------------
*//**
   \param[in]   busy   Linked list of busy periods

   Simply prints a list of busy periods for debugging

-  18.11.16 Original   By: ACRM
*/
void PrintBusy(BUSY *busy)
{
   BUSY *b;
   
   printf("Busy periods:\n");

   for(b=busy; b!=NULL; NEXT(b))
   {
      printf("  %lu - %lu\n", (ULONG)b->startTime, (ULONG)b->stopTime);
   }
}


/************************************************************************/
/*>JOBS *ReadTimes(char *fileName)
   -------------------------------
*//**
   \param[in]   filename  The data file name
   \return                Linked list of job information

-  18.11.16 Original   By: ACRM
*/
JOBS *ReadTimes(char *fileName)
{
   FILE *in = NULL;
   JOBS *jobs = NULL,
      *j;
   
   
   if((in = fopen(fileName, "r"))!=NULL)
   {
      ULONG submit,
            start,
            stop;
      char  buffer[MAXBUFF];
      
      while(fgets(buffer, MAXBUFF, in))
      {
         sscanf(buffer, "%lu %lu %lu", &submit, &start, &stop);
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


/************************************************************************/
/*>time_t CalculateWaitTime(JOBS *jobs, BUSY *busy)
   ------------------------------------------------
*//**
   \param[in]     jobs  Linked list of jobs
   \param[in,out] busy  Linked list of busy periods
   \return              The total wait time

   Calculates the total wait time when the machine is not busy

   Note that this machine modifies the busy linked list and the first
   position in the linked list may no longer be valid. 

   DO NOT USE the busy linked list after calling this routine!

-  18.11.16 Original   By: ACRM
*/
time_t CalculateWaitTime(JOBS *jobs, BUSY *busy)
{
   BUSY *b, *best;
   JOBS *j;
   time_t minDiff, totalWait = 0L;
   
   
   for(j=jobs; j!=NULL; NEXT(j))
   {
      time_t submitTime = j->submitTime;

      /* If the submitTime falls within a busy time, update it to the 
         end of the busy period 
      */
      for(b=busy; b!=NULL; NEXT(b))
      {
         if((submitTime >= b->startTime) &&
            (submitTime <= b->stopTime))
         {
            submitTime = b->stopTime;
            break;
         }
      }

      /* If the startTime is before the submitTime (because we updated 
         it - i.e. the job was submitted and started within a busy time)
         simply skip on to the next job
      */
      if((b!=NULL) && (b->startTime <= submitTime))
         continue;
      
      /* Find the time between the submitTime and the next greatest busy
         time 
      */
      minDiff = 100000000;     /* 1157 days!                            */
      
      for(b=busy; b!=NULL; NEXT(b))
      {
         if(b->startTime > submitTime)
         {
            time_t diff = (b->startTime - submitTime);
            if(diff < minDiff)
            {
               minDiff = diff;
               best    = b;
            }
         }
      }

      /* Update the startTime of this job to the submitTime, so we don't
         count multiple waits for lots of jobs submitted at the same time
      */
      best->startTime = submitTime;
      
      totalWait += minDiff;
   }
   
   return(totalWait);
}
