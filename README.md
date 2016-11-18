# waittime

An experimental piece of code for calculating a wait time in job scheduling

```
   The wait time is calculated as the time after a job submission when
   nothing else is running - i.e. the amount of time that the user
   perceives that they are not getting any time on the machine. Thus
   submitting a batch of jobs will only be counted as a single wait
   time until the machine starts processing (at least) one of those jobs.

   Suppose we have a set of jobs like this (S=submit, X=start, Y=stop)

            1         2         3         4         5         6         7
   1234567890123456789012345678901234567890123456789012345678901234567890
      S     X---------Y
               S   X-----------Y
                         S   X-----Y
                           S              X-----Y
                                      S     X------Y
                                                      S    X-----Y

    We can represent the 'busy' times as:

            1         2         3         4         5         6         7
   1234567890123456789012345678901234567890123456789012345678901234567890
            BBBBBBBBBBBBBBBBBBBBBBBY      BBBBBBBBBY       BBBBBBY

    Which means the only signficant wait times are between Submits and
    busy times:

            1         2         3         4         5         6         7
   1234567890123456789012345678901234567890123456789012345678901234567890
      S        s         s s          S               S
            BBBBBBBBBBBBBBBBBBBBBBBY      BBBBBBBBBY       BBBBBBY
      ^^^^^^                          ^^^^            ^^^^^

   i.e. total run  (X--) time is 47
        total busy (B)   time is 38
        total wait (^)   time is 15
```