#ifndef MONITOR_H
#define MONITOR_H

#include <semaphore.h>

#define CHAIRS    6
#define CUSTOMERS 75

typedef struct cond {
    int   count;
    sem_t semaphore;
} CV;

// CV operations
int  cv_count(CV *cv);
void cv_wait(CV *cv, sem_t *entryQueue);
void cv_signal(CV *cv);

// Monitor functions
void mon_init(void);
void mon_checkCustomer(void);
int  mon_checkStylist(void);
int  mon_recordHaircut(void);
void mon_debugPrint(void);

#endif
