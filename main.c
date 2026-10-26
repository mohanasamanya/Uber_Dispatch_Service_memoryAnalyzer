// /* auto-drivr assgn */
 
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <pthread.h>
 
// #define MAX_DRIVERS 50
 
// /* ===========================
//    MEMORY TRACKING
//    =========================== */
 
// typedef struct MemoryRecord
// {
//     void *ptr;
//     size_t size;
//     struct MemoryRecord *next;
// } MemoryRecord;
 
// MemoryRecord *memoryHead = NULL;
 
// size_t currentMemory = 0;
// size_t peakMemory = 0;
// int activeAllocations = 0;
 
// pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;
 
// /* ===========================
//    DRIVER STRUCTURE
//    =========================== */
 
// typedef struct
// {
//     int driverId;
//     char name[50];
//     int available;
// } Driver;
 
// Driver drivers[MAX_DRIVERS];
// int driverCount = 0;
 
// /* ===========================
//    RIDE STRUCTURE
//    =========================== */
 
// typedef struct Ride
// {
//     int rideId;
//     char customer[50];
//     char pickup[50];
//     char destination[50];
 
//     int assignedDriver;
 
//     struct Ride *next;
 
// } Ride;
 
// Ride *rideHead = NULL;
 
// /* ===========================
//    STATISTICS
//    =========================== */
 
// int totalRequests = 0;
// int completedRides = 0;
 
// /* ===========================
//    FUNCTION PROTOTYPES
//    =========================== */
 
// void *tracked_malloc(size_t size);
// void tracked_free(void *ptr);
 
// void addDriver();
// void displayDrivers();
 
// void addRideRequest();
// void displayRideRequests();
 
// void processRide();
 
// void memoryStatistics();
// void leakReport();
 
// void saveLog(const char *msg);
 
// int assignAvailableDriver();
// void releaseDriver(int driverId);
 
// void cleanup();
 
// /* ===========================
//    MEMORY TRACKER
//    =========================== */
 
// void *tracked_malloc(size_t size)
// {
//     void *ptr = malloc(size);
 
//     if(ptr == NULL)
//     {
//         printf("Memory Allocation Failed\n");
//         return NULL;
//     }
 
//     MemoryRecord *record =
//             (MemoryRecord *)malloc(sizeof(MemoryRecord));
 
//     if(record == NULL)
//     {
//         free(ptr);
//         return NULL;
//     }
 
//     record->ptr = ptr;
//     record->size = size;
 
//     pthread_mutex_lock(&memoryMutex);
 
//     record->next = memoryHead;
//     memoryHead = record;
 
//     currentMemory += size;
//     activeAllocations++;
 
//     if(currentMemory > peakMemory)
//     {
//         peakMemory = currentMemory;
//     }
 
//     pthread_mutex_unlock(&memoryMutex);
 
//     return ptr;
// }
 
// void tracked_free(void *ptr)
// {
//     if(ptr == NULL)
//         return;
 
//     pthread_mutex_lock(&memoryMutex);
 
//     MemoryRecord *temp = memoryHead;
//     MemoryRecord *prev = NULL;
 
//     while(temp)
//     {
//         if(temp->ptr == ptr)
//         {
//             currentMemory -= temp->size;
//             activeAllocations--;
 
//             if(prev)
//                 prev->next = temp->next;
//             else
//                 memoryHead = temp->next;
 
//             free(temp);
//             break;
//         }
 
//         prev = temp;
//         temp = temp->next;
//     }
 
//     pthread_mutex_unlock(&memoryMutex);
 
//     free(ptr);
// }
 
// /* ===========================
//    LOGGING
//    =========================== */
 
// void saveLog(const char *msg)
// {
//     FILE *fp = fopen("ride_log.txt", "a");
 
//     if(fp != NULL)
//     {
//         fprintf(fp, "%s\n", msg);
//         fclose(fp);
//     }
// }
 
// /* ===========================
//    DRIVER FUNCTIONS
//    =========================== */
 
// void addDriver()
// {
//     if(driverCount >= MAX_DRIVERS)
//     {
//         printf("Max Driver Limit Reached\n");
//         return;
//     }
 
//     printf("Enter Driver ID : ");
//     scanf("%d", &drivers[driverCount].driverId);
 
//     printf("Enter Driver Name : ");
//     scanf("%49s", drivers[driverCount].name);
 
//     drivers[driverCount].available = 1;
 
//     driverCount++;
 
//     printf("Driver Added Successfully\n");
// }
 
// void displayDrivers()
// {
//     int i;
 
//     printf("\n===== DRIVERS =====\n");
 
//     for(i = 0; i < driverCount; i++)
//     {
//         printf("ID:%d Name:%s Available:%s\n",
//                drivers[i].driverId,
//                drivers[i].name,
//                drivers[i].available ? "YES" : "NO");
//     }
// }
 
// /* ===========================
//    AUTOMATIC DRIVER ASSIGNMENT
//    =========================== */
 
// int assignAvailableDriver()
// {
//     int i;
 
//     for(i = 0; i < driverCount; i++)
//     {
//         if(drivers[i].available)
//         {
//             drivers[i].available = 0;
//             return drivers[i].driverId;
//         }
//     }
 
//     return -1;
// }
 
// void releaseDriver(int driverId)
// {
//     int i;
 
//     for(i = 0; i < driverCount; i++)
//     {
//         if(drivers[i].driverId == driverId)
//         {
//             drivers[i].available = 1;
//             break;
//         }
//     }
// }
 
// /* ===========================
//    RIDE FUNCTIONS
//    =========================== */
 
// void addRideRequest()
// {
//     Ride *ride =
//         (Ride *)tracked_malloc(sizeof(Ride));
 
//     if(ride == NULL)
//         return;
 
//     printf("Enter Ride ID : ");
//     scanf("%d", &ride->rideId);
 
//     printf("Customer Name : ");
//     scanf("%49s", ride->customer);
 
//     printf("Pickup Location : ");
//     scanf("%49s", ride->pickup);
 
//     printf("Destination : ");
//     scanf("%49s", ride->destination);
 
//     ride->assignedDriver =
//         assignAvailableDriver();
 
//     if(ride->assignedDriver == -1)
//     {
//         printf("No Drivers Available\n");
//     }
//     else
//     {
//         printf("Driver %d Assigned Automatically\n",
//                ride->assignedDriver);
//     }
 
//     ride->next = rideHead;
//     rideHead = ride;
 
//     totalRequests++;
 
//     saveLog("Ride Request Created");
// }
 
// void displayRideRequests()
// {
//     Ride *temp = rideHead;
 
//     printf("\n===== ACTIVE RIDES =====\n");
 
//     while(temp)
//     {
//         printf("Ride:%d Customer:%s Driver:%d\n",
//                temp->rideId,
//                temp->customer,
//                temp->assignedDriver);
 
//         temp = temp->next;
//     }
// }
 
// /* ===========================
//    THREAD
//    =========================== */
 
// void *rideProcessor(void *arg)
// {
//     Ride *ride = (Ride *)arg;
 
//     printf("\nProcessing Ride %d...\n",
//            ride->rideId);
 
//     printf("Customer : %s\n",
//            ride->customer);
 
//     printf("Trip Completed Successfully!\n");
 
//     releaseDriver(ride->assignedDriver);
 
//     completedRides++;
 
//     saveLog("Ride Completed");
 
//     return NULL;
// }
 
// /* ===========================
//    PROCESS RIDE
//    =========================== */
 
// void processRide()
// {
//     int rideId;
 
//     printf("Enter Ride ID : ");
//     scanf("%d", &rideId);
 
//     Ride *temp = rideHead;
//     Ride *prev = NULL;
 
//     while(temp)
//     {
//         if(temp->rideId == rideId)
//         {
//             pthread_t tid;
 
//             pthread_create(&tid,
//                            NULL,
//                            rideProcessor,
//                            temp);
 
//             pthread_join(tid, NULL);
 
//             if(prev)
//                 prev->next = temp->next;
//             else
//                 rideHead = temp->next;
 
//             tracked_free(temp);
 
//             printf("Ride Removed From Queue\n");
 
//             return;
//         }
 
//         prev = temp;
//         temp = temp->next;
//     }
 
//     printf("Ride Not Found\n");
// }
 
// /* ===========================
//    MEMORY STATS
//    =========================== */
 
// void memoryStatistics()
// {
//     printf("\n===== MEMORY STATISTICS =====\n");
 
//     printf("Current Memory     : %zu Bytes\n",
//            currentMemory);
 
//     printf("Peak Memory Usage  : %zu Bytes\n",
//            peakMemory);
 
//     printf("Active Allocations : %d\n",
//            activeAllocations);
 
//     printf("Total Requests     : %d\n",
//            totalRequests);
 
//     printf("Completed Rides    : %d\n",
//            completedRides);
// }
 
// /* ===========================
//    LEAK REPORT
//    =========================== */
 
// void leakReport()
// {
//     MemoryRecord *temp = memoryHead;
 
//     printf("\n===== LEAK REPORT =====\n");
 
//     if(temp == NULL)
//     {
//         printf("No Memory Leaks Detected\n");
//         return;
//     }
 
//     while(temp)
//     {
//         printf("Memory Leak -> Address:%p Size:%zu\n",
//                temp->ptr,
//                temp->size);
 
//         temp = temp->next;
//     }
// }
 
// /* ===========================
//    CLEANUP
//    =========================== */
 
// void cleanup()
// {
//     Ride *temp = rideHead;
 
//     while(temp)
//     {
//         Ride *next = temp->next;
 
//         tracked_free(temp);
 
//         temp = next;
//     }
// }
 
// /* ===========================
//    MAIN
//    =========================== */
 
// int main()
// {
//     int choice;
 
//     while(1)
//     {
//         printf("\n");
//         printf("====================================\n");
//         printf(" UBER DISPATCH MEMORY ANALYZER\n");
//         printf("====================================\n");
 
//         printf("1. Add Driver\n");
//         printf("2. Create Ride Request\n");
//         printf("3. Display Drivers\n");
//         printf("4. Display Active Rides\n");
//         printf("5. Process Ride\n");
//         printf("6. Memory Statistics\n");
//         printf("7. Leak Report\n");
//         printf("8. Exit\n");
 
//         printf("Enter Choice : ");
//         scanf("%d", &choice);
 
//         switch(choice)
//         {
//             case 1:
//                 addDriver();
//                 break;
 
//             case 2:
//                 addRideRequest();
//                 break;
 
//             case 3:
//                 displayDrivers();
//                 break;
 
//             case 4:
//                 displayRideRequests();
//                 break;
 
//             case 5:
//                 processRide();
//                 break;
 
//             case 6:
//                 memoryStatistics();
//                 break;
 
//             case 7:
//                 leakReport();
//                 break;
 
//             case 8:
//                 cleanup();
 
//                 printf("\nFinal Leak Check\n");
//                 leakReport();
 
//                 printf("\nExiting...\n");
//                 return 0;
 
//             default:
//                 printf("Invalid Choice\n");
//         }
//     }
 
//     return 0;
// }
 

/* UBER DISPATCH MEMORY ANALYZER */
/* Automatic Driver Assignment */
/* Memory Tracking + Google Test Support */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include<time.h>

#include "uber.h"

/* ===========================
   GLOBAL MEMORY VARIABLES
   =========================== */

MemoryRecord *memoryHead = NULL;

size_t currentMemory = 0;
size_t peakMemory = 0;
int activeAllocations = 0;

pthread_mutex_t memoryMutex = PTHREAD_MUTEX_INITIALIZER;

/* ===========================
   GLOBAL DRIVER VARIABLES
   =========================== */

Driver drivers[MAX_DRIVERS];
int driverCount = 0;

/* ===========================
   GLOBAL RIDE VARIABLES
   =========================== */

Ride *rideHead = NULL;

/* ===========================
   GLOBAL STATISTICS
   =========================== */

int totalRequests = 0;
int completedRides = 0;

/* ===========================
   MEMORY TRACKER
   =========================== */

void *tracked_malloc(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
    {
        printf("Memory Allocation Failed\n");
        return NULL;
    }

    MemoryRecord *record =
        (MemoryRecord *)malloc(sizeof(MemoryRecord));

    if (record == NULL)
    {
        free(ptr);
        return NULL;
    }

    record->ptr = ptr;
    record->size = size;

    pthread_mutex_lock(&memoryMutex);

    record->next = memoryHead;
    memoryHead = record;

    currentMemory += size;
    activeAllocations++;

    if (currentMemory > peakMemory)
    {
        peakMemory = currentMemory;
    }

    pthread_mutex_unlock(&memoryMutex);

    return ptr;
}

/* ===========================
   TRACKED FREE
   =========================== */

void tracked_free(void *ptr)
{
    if (ptr == NULL)
        return;

    pthread_mutex_lock(&memoryMutex);

    MemoryRecord *temp = memoryHead;
    MemoryRecord *prev = NULL;
    int found = 0;

    while (temp != NULL)
    {
        if (temp->ptr == ptr)
        {
            currentMemory -= temp->size;
            activeAllocations--;

            if (prev != NULL)
                prev->next = temp->next;
            else
                memoryHead = temp->next;

            free(temp);
            found = 1;
            break;
        }

        prev = temp;
        temp = temp->next;
    }

    pthread_mutex_unlock(&memoryMutex);

    if (found)
    {
        free(ptr);
    }
    else
    {
        fprintf(stderr,
                "Warning: Pointer not found in memory tracker\n");
    }
}

/* ===========================
   LOGGING
   =========================== */


void saveLog(const char *msg)
{
    FILE *fp = fopen("ride_log.txt", "a");

    if (fp != NULL)
    {
        time_t now = time(NULL);
        struct tm *local = localtime(&now);

        char timestamp[32];

        if (local != NULL)
        {
            strftime(timestamp, sizeof(timestamp),
                     "%Y-%m-%d %H:%M:%S", local);
        }
        else
        {
            snprintf(timestamp, sizeof(timestamp),
                     "Time unavailable");
        }

        fprintf(fp, "[%s] %s\n", timestamp, msg);

        fclose(fp);
    }
}


/* ===========================
   ADD DRIVER
   =========================== */

void addDriver()
{
    if (driverCount >= MAX_DRIVERS)
    {
        printf("Max Driver Limit Reached\n");
        return;
    }

    printf("Enter Driver ID : ");
    scanf("%d", &drivers[driverCount].driverId);

    printf("Enter Driver Name : ");
    scanf("%49s", drivers[driverCount].name);

    drivers[driverCount].available = 1;

    driverCount++;

    printf("Driver Added Successfully\n");
}

/* ===========================
   DISPLAY DRIVERS
   =========================== */

void displayDrivers()
{
    int i;

    printf("\n===== DRIVERS =====\n");

    for (i = 0; i < driverCount; i++)
    {
        printf("ID:%d Name:%s Available:%s\n",
               drivers[i].driverId,
               drivers[i].name,
               drivers[i].available ? "YES" : "NO");
    }
}

/* ===========================
   AUTOMATIC DRIVER ASSIGNMENT
   =========================== */

int assignAvailableDriver()
{
    int i;

    for (i = 0; i < driverCount; i++)
    {
        if (drivers[i].available)
        {
            drivers[i].available = 0;
            return drivers[i].driverId;
        }
    }

    return -1;
}

/* ===========================
   RELEASE DRIVER
   =========================== */

void releaseDriver(int driverId)
{
    int i;

    for (i = 0; i < driverCount; i++)
    {
        if (drivers[i].driverId == driverId)
        {
            drivers[i].available = 1;
            break;
        }
    }
}

/* ===========================
   ADD RIDE REQUEST
   =========================== */

void addRideRequest()
{
    Ride *ride =
        (Ride *)tracked_malloc(sizeof(Ride));

    if (ride == NULL)
        return;

    printf("Enter Ride ID : ");
    scanf("%d", &ride->rideId);

    printf("Customer Name : ");
    scanf("%49s", ride->customer);

    printf("Pickup Location : ");
    scanf("%49s", ride->pickup);

    printf("Destination : ");
    scanf("%49s", ride->destination);

    ride->assignedDriver =
        assignAvailableDriver();

    if (ride->assignedDriver == -1)
    {
        printf("No Drivers Available\n");
    }
    else
    {
        printf("Driver %d Assigned Automatically\n",
               ride->assignedDriver);
    }

    ride->next = rideHead;
    rideHead = ride;

    totalRequests++;

    saveLog("Ride Request Created");
}

/* ===========================
   DISPLAY RIDE REQUESTS
   =========================== */

void displayRideRequests()
{
    Ride *temp = rideHead;

    printf("\n===== ACTIVE RIDES =====\n");

    while (temp != NULL)
    {
        printf("Ride:%d Customer:%s Driver:%d\n",
               temp->rideId,
               temp->customer,
               temp->assignedDriver);

        temp = temp->next;
    }
}

/* ===========================
   RIDE PROCESSOR THREAD
   =========================== */

void *rideProcessor(void *arg)
{
    Ride *ride = (Ride *)arg;

    printf("\nProcessing Ride %d...\n",
           ride->rideId);

    printf("Customer : %s\n",
           ride->customer);

    printf("Trip Completed Successfully!\n");

    releaseDriver(ride->assignedDriver);

    completedRides++;

    saveLog("Ride Completed");

    return NULL;
}

/* ===========================
   PROCESS RIDE
   =========================== */

void processRide()
{
    int rideId;

    printf("Enter Ride ID : ");
    scanf("%d", &rideId);

    Ride *temp = rideHead;
    Ride *prev = NULL;

    while (temp != NULL)
    {
        if (temp->rideId == rideId)
        {
            pthread_t tid;

            int result = pthread_create(
                &tid,
                NULL,
                rideProcessor,
                temp
            );

            if (result != 0)
            {
                printf("Thread Creation Failed\n");
                return;
            }

            pthread_join(tid, NULL);

            if (prev != NULL)
                prev->next = temp->next;
            else
                rideHead = temp->next;

            tracked_free(temp);

            printf("Ride Removed From Queue\n");

            return;
        }

        prev = temp;
        temp = temp->next;
    }

    printf("Ride Not Found\n");
}

/* ===========================
   MEMORY STATISTICS
   =========================== */

void memoryStatistics()
{
    printf("\n===== MEMORY STATISTICS =====\n");

    printf("Current Memory     : %zu Bytes\n",
           currentMemory);

    printf("Peak Memory Usage  : %zu Bytes\n",
           peakMemory);

    printf("Active Allocations : %d\n",
           activeAllocations);

    printf("Total Requests     : %d\n",
           totalRequests);

    printf("Completed Rides    : %d\n",
           completedRides);
}

/* ===========================
   LEAK REPORT
   =========================== */

void leakReport()
{
    MemoryRecord *temp = memoryHead;

    printf("\n===== LEAK REPORT =====\n");

    if (temp == NULL)
    {
        printf("No Memory Leaks Detected\n");
        return;
    }

    while (temp != NULL)
    {
        printf("Memory Leak -> Address:%p Size:%zu\n",
               temp->ptr,
               temp->size);

        temp = temp->next;
    }
}

/* ===========================
   CLEANUP
   =========================== */

void cleanup()
{
    Ride *temp = rideHead;

    while (temp != NULL)
    {
        Ride *next = temp->next;

        tracked_free(temp);

        temp = next;
    }

    rideHead = NULL;
}

/* ===========================
   MAIN FUNCTION
   Excluded during Google Test
   =========================== */

#ifndef UNIT_TEST

int main()
{
    int choice;

    while (1)
    {
        printf("\n");
        printf("====================================\n");
        printf(" UBER DISPATCH MEMORY ANALYZER\n");
        printf("====================================\n");

        printf("1. Add Driver\n");
        printf("2. Create Ride Request\n");
        printf("3. Display Drivers\n");
        printf("4. Display Active Rides\n");
        printf("5. Process Ride\n");
        printf("6. Memory Statistics\n");
        printf("7. Leak Report\n");
        printf("8. Exit\n");

        printf("Enter Choice : ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Invalid Input\n");
            break;
        }

        switch (choice)
        {
            case 1:
                addDriver();
                break;

            case 2:
                addRideRequest();
                break;

            case 3:
                displayDrivers();
                break;

            case 4:
                displayRideRequests();
                break;

            case 5:
                processRide();
                break;

            case 6:
                memoryStatistics();
                break;

            case 7:
                leakReport();
                break;

            case 8:
                cleanup();

                printf("\nFinal Leak Check\n");
                leakReport();

                printf("\nExiting...\n");

                return 0;

            default:
                printf("Invalid Choice\n");
        }
    }

    cleanup();

    return 0;
}

#endif
