#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "monitor.h"

// -------------------------------------------------------
// Monitor state
// -------------------------------------------------------
static sem_t entryQueue;
static CV    stylistAvailable;
static CV    customerAvailable;

static int customerCount   = 0;  // customers currently in waiting chairs
static int stylistReady    = 0;  // 1 = stylist available, 0 = busy/sleeping
static int haircuts        = 0;  // total completed haircuts
static int salonFullCount  = 0;  // times salon was found full
static int salonEmptyCount = 0;  // times stylist went to sleep

static int chairs[CHAIRS];       // chairs[i] = 1 occupied, 0 empty

// -------------------------------------------------------
// mon_init — call once from main() before threads start
// -------------------------------------------------------
void mon_init(void) {
    sem_init(&entryQueue, 0, 1);

    stylistAvailable.count = 0;
    sem_init(&stylistAvailable.semaphore, 0, 0);

    customerAvailable.count = 0;
    sem_init(&customerAvailable.semaphore, 0, 0);

    for (int i = 0; i < CHAIRS; i++)
        chairs[i] = 0;
}

// -------------------------------------------------------
// CV operations
// -------------------------------------------------------
int cv_count(CV *cv) {
    return cv->count;
}

// Release monitor lock, suspend, re-acquire on wake-up (signal-and-continue)
void cv_wait(CV *cv, sem_t *entry) {
    cv->count++;
    sem_post(entry);           // release monitor so others can enter
    sem_wait(&cv->semaphore);  // suspend self

    // Re-acquire monitor lock — this is signal-and-continue:
    // the woken thread must compete at entry queue, not jump back in directly
    sem_wait(entry);
    cv->count--;
    printf("[CV] Thread resumed after cv_wait — re-entered monitor (signal-and-continue)\n");
}

// Wake one blocked thread. Signaler keeps running inside monitor.
void cv_signal(CV *cv) {
    if (cv->count > 0) {
        sem_post(&cv->semaphore);
    }
}

// -------------------------------------------------------
// mon_checkCustomer — called by stylist
// Signals ready, sleeps if no customers, then takes one
// -------------------------------------------------------
void mon_checkCustomer(void) {
    sem_wait(&entryQueue);  // enter monitor

    stylistReady = 1;
    cv_signal(&stylistAvailable);  // wake any waiting customer

    if (customerCount == 0) {      // no customers — go to sleep
        salonEmptyCount++;
        printf("[STYLIST] Salon empty. Stylist going to sleep. (Empty count: %d)\n",
               salonEmptyCount);
        cv_wait(&customerAvailable, &entryQueue);
    }

    // Take one customer from chairs
    customerCount--;
    chairs[customerCount] = 0;  // free the last occupied chair slot

    sem_post(&entryQueue);  // exit monitor
}

// -------------------------------------------------------
// mon_checkStylist — called by customer
// Sits in chair if available, waits for stylist if busy
// Returns 1 if got haircut appointment, 0 if salon full
// -------------------------------------------------------
int mon_checkStylist(void) {
    sem_wait(&entryQueue);  // enter monitor

    int status = 0;

    if (customerCount < CHAIRS) {
        // Sit in a chair
        chairs[customerCount] = 1;
        customerCount++;

        cv_signal(&customerAvailable);  // wake stylist if sleeping

        if (stylistReady == 0) {        // stylist busy — wait
            printf("[CUSTOMER] Stylist not ready. Waiting on stylistAvailable CV.\n");
            cv_wait(&stylistAvailable, &entryQueue);
        }

        stylistReady = 0;  // take the stylist
        status = 1;
        printf("[CUSTOMER] Got stylist. Proceeding to chair.\n");
    } else {
        salonFullCount++;
        printf("[CUSTOMER] Salon FULL (%d/%d chairs). Will try again. (Full count: %d)\n",
               customerCount, CHAIRS, salonFullCount);
    }

    sem_post(&entryQueue);  // exit monitor
    return status;
}

// -------------------------------------------------------
// mon_recordHaircut — called by stylist after cutting hair
// Increments haircut counter inside the monitor
// -------------------------------------------------------
int mon_recordHaircut(void) {
    sem_wait(&entryQueue);
    haircuts++;
    int total = haircuts;
    sem_post(&entryQueue);
    return total;
}

// -------------------------------------------------------
// mon_debugPrint — prints monitor state (inside monitor)
// -------------------------------------------------------
void mon_debugPrint(void) {
    sem_wait(&entryQueue);  // enter monitor

    printf("|");
    for (int i = 0; i < CHAIRS; i++) {
        printf(" %d |", chairs[i]);
    }
    printf(" => %d\n", customerCount);
    printf("Given haircuts = %d\n", haircuts);
    printf("Salon full = %d times\n", salonFullCount);
    printf("Salon empty = %d times\n", salonEmptyCount);
    printf("\n");

    sem_post(&entryQueue);  // exit monitor
}
