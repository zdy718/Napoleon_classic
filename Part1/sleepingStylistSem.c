#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>

#define CHAIRS         6
#define CUSTOMERS      75

#define STYLIST_DELAY  50000    // 0.05s — stylist is very fast
#define CUSTOMER_DELAY 500000   // 0.5s  — customers retry slowly when full

sem_t mutex, stylist, customers;
int waiting    = 0;
int done       = 0;
int salonFull  = 0;
int salonEmpty = 0;

void *stylistFunction(void *arg) {
    while (1) {
        // Blocks here when no customers — this is the stylist sleeping
        sem_wait(&customers);

        sem_wait(&mutex);
        waiting--;
        sem_post(&stylist);
        sem_post(&mutex);

        // Cut hair (fast)
        usleep(STYLIST_DELAY);

        sem_wait(&mutex);
        done++;
        printf("[STYLIST] Haircut done! Total: %d | Waiting: %d\n", done, waiting);

        if (waiting == 0 && done < CUSTOMERS) {
            salonEmpty++;
            printf("[STYLIST] Salon empty — stylist going to sleep. (Sleep count: %d)\n", salonEmpty);
        }

        if (done == CUSTOMERS) {
            sem_post(&mutex);
            pthread_exit(NULL);
        }
        sem_post(&mutex);
    }
    return NULL;
}

void *customerFunction(void *arg) {
    int id = *((int *)arg);

    // Three phases of arrivals:
    // Customers 1-10:  arrive slowly one by one (empty salon phase)
    // Customers 11-50: arrive in a burst (full salon phase)
    // Customers 51-75: arrive slowly again (draining phase)
    if (id <= 10) {
        usleep(id * 200000);          // 0.2s apart — slow trickle
    } else if (id <= 50) {
        usleep(200000 * 10 + (id - 10) * 20000);  // burst: 0.02s apart
    } else {
        usleep(200000 * 10 + 40 * 20000 + (id - 50) * 300000); // slow again
    }

    while (1) {
        sem_wait(&mutex);

        if (waiting < CHAIRS) {
            waiting++;
            printf("[CUSTOMER %d] Entered salon. Chairs occupied: %d/%d\n", id, waiting, CHAIRS);
            sem_post(&customers);
            sem_post(&mutex);

            sem_wait(&stylist);
            printf("[CUSTOMER %d] Getting haircut!\n", id);
            break;
        } else {
            salonFull++;
            printf("[CUSTOMER %d] Salon FULL (%d/%d). Going shopping. (Full count: %d)\n",
                   id, waiting, CHAIRS, salonFull);
            sem_post(&mutex);
            usleep(CUSTOMER_DELAY);
        }
    }
    return NULL;
}

int main(void) {
    pthread_t stylistTid;
    pthread_t customerTids[CUSTOMERS];
    int ids[CUSTOMERS];

    srand(time(NULL));

    sem_init(&mutex,     0, 1);
    sem_init(&stylist,   0, 0);
    sem_init(&customers, 0, 0);

    printf("Sleeping Stylist Simulation (Semaphores)\n");
    printf("Customers: %d | Chairs: %d\n\n", CUSTOMERS, CHAIRS);

    pthread_create(&stylistTid, NULL, stylistFunction, NULL);

    for (int i = 0; i < CUSTOMERS; i++) {
        ids[i] = i + 1;
        pthread_create(&customerTids[i], NULL, customerFunction, &ids[i]);
    }

    for (int i = 0; i < CUSTOMERS; i++) {
        pthread_join(customerTids[i], NULL);
    }
    pthread_join(stylistTid, NULL);

    printf("\nSimulation Complete\n");
    printf("Total haircuts given : %d\n", done);
    printf("Salon full events    : %d\n", salonFull);
    printf("Stylist slept        : %d times\n", salonEmpty);

    sem_destroy(&mutex);
    sem_destroy(&stylist);
    sem_destroy(&customers);

    return 0;
}