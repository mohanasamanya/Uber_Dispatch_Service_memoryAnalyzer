#ifndef UBER_H
#define UBER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DRIVERS 50

typedef struct MemoryRecord {
    void *ptr;
    size_t size;
    struct MemoryRecord *next;
} MemoryRecord;

typedef struct {
    int driverId;
    char name[50];
    int available;
} Driver;

typedef struct Ride {
    int rideId;
    char customer[50];
    char pickup[50];
    char destination[50];
    int assignedDriver;
    struct Ride *next;
} Ride;

extern MemoryRecord *memoryHead;
extern size_t currentMemory;
extern size_t peakMemory;
extern int activeAllocations;
extern Driver drivers[MAX_DRIVERS];
extern int driverCount;
extern Ride *rideHead;
extern int totalRequests;
extern int completedRides;

void *tracked_malloc(size_t size);
void tracked_free(void *ptr);
int assignAvailableDriver(void);
void releaseDriver(int driverId);
void cleanup(void);
void displayDrivers(void);
void displayRideRequests(void);
void memoryStatistics(void);
void leakReport(void);
void saveLog(const char *msg);
void processRide(void);
void addDriver(void);
void addRideRequest(void);

#ifdef __cplusplus
}
#endif
#endif
