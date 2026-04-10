#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "monitor.h"

#define STYLIST_DELAY  50000   // 0.05s — fast stylist
#define CUSTOMER_DELAY 500000  // 0.5s  — slow retry when full
#define ARRIVAL_GAP    200000  // 0.2s  — gap between early arrivals

// -------------------------------------------------------
// Stylist thread
// -------------------------------------------------------
void *stylistThread(void *arg) {
    while (1) {
        mon_debugPrint();
        mon_checkCustomer();

        // Cut hair
        usleep(STYLIST_DELAY);

        int total = mon_recordHaircut();
        printf("[STYLIST] Finished cutting hair. Total haircuts: %d\n", total);

        // Exit once all customers served — checked inside monitor
        if (total == CUSTOMERS)
            break;
    }
    return NULL;
}

// -------------------------------------------------------
// Customer thread
// -------------------------------------------------------
void *customerThread(void *arg) {
    int id = *((int *)arg);

    // Three-phase arrivals to demonstrate all scenarios:
    // Phase 1 (1-10):  slow trickle → salon empty, stylist sleeps
    // Phase 2 (11-50): burst        → salon fills up, customers turned away
    // Phase 3 (51-75): slow again   → salon drains, stylist sleeps again
    if (id <= 10) {
        usleep(id * ARRIVAL_GAP);
    } else if (id <= 50) {
        usleep(10 * ARRIVAL_GAP + (id - 10) * 20000);
    } else {
        usleep(10 * ARRIVAL_GAP + 40 * 20000 + (id - 50) * 300000);
    }

    while (1) {
        mon_debugPrint();

        if (mon_checkStylist()) {
            printf("[CUSTOMER %d] Getting haircut!\n", id);
            break;
        }

        // Salon full — go shopping and try again
        printf("[CUSTOMER %d] Going shopping...\n", id);
        usleep(CUSTOMER_DELAY);
    }

    printf("[CUSTOMER %d] Done. Leaving salon.\n", id);
    return NULL;
}

// -------------------------------------------------------
// main
// -------------------------------------------------------
int main(void) {
    pthread_t stylistTid;
    pthread_t customerTids[CUSTOMERS];
    int ids[CUSTOMERS];

    printf("Sleeping Stylist Simulation (Monitor)\n");
    printf("Customers: %d | Chairs: %d\n\n", CUSTOMERS, CHAIRS);

    mon_init();

    pthread_create(&stylistTid, NULL, stylistThread, NULL);

    for (int i = 0; i < CUSTOMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&customerTids[i], NULL, customerThread, &ids[i]);
    }

    for (int i = 0; i < CUSTOMERS; i++) {
        pthread_join(customerTids[i], NULL);
    }

    pthread_join(stylistTid, NULL);

    printf("\nSimulation complete\n");
    return 0;
}