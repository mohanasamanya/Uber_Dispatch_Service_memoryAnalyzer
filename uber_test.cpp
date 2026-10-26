#include "uber.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <unistd.h>


#include <cstdio>
#include <unistd.h>

static void runProcessRideWithInput(int rideId)
{
    FILE *input = tmpfile();
    ASSERT_NE(input, nullptr);

    fprintf(input, "%d\n", rideId);
    rewind(input);

    int savedStdin = dup(STDIN_FILENO);
    ASSERT_NE(savedStdin, -1);

    // Clear buffered input before redirecting
    clearerr(stdin);

    ASSERT_NE(dup2(fileno(input), STDIN_FILENO), -1);

    processRide();

    // Restore original input
    dup2(savedStdin, STDIN_FILENO);
    close(savedStdin);
    fclose(input);
    clearerr(stdin);
}


class UberTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Each test begins with clean global state.
        ASSERT_EQ(memoryHead, nullptr);
        currentMemory = 0;
        peakMemory = 0;
        activeAllocations = 0;
        driverCount = 0;
        rideHead = nullptr;
        totalRequests = 0;
        completedRides = 0;
        std::memset(drivers, 0, sizeof(drivers));
    }
    void TearDown() override {
        // Free all tracked allocations, including deliberately leaked test blocks.
        while (memoryHead != nullptr) {
            void *ptr = memoryHead->ptr;
            tracked_free(ptr);
        }
        rideHead = nullptr;
        EXPECT_EQ(currentMemory, 0u);
        EXPECT_EQ(activeAllocations, 0);
    }
};

TEST_F(UberTest, MemoryAllocation) {
    void *ptr = tracked_malloc(100);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(currentMemory, 100u);
    EXPECT_EQ(activeAllocations, 1);
    tracked_free(ptr);
    EXPECT_EQ(currentMemory, 0u);
    EXPECT_EQ(activeAllocations, 0);
}
TEST_F(UberTest, PeakMemoryUsage) {
    void *p1 = tracked_malloc(100);
    void *p2 = tracked_malloc(200);
    ASSERT_NE(p1, nullptr);
    ASSERT_NE(p2, nullptr);
    EXPECT_EQ(currentMemory, 300u);
    EXPECT_EQ(peakMemory, 300u);
    tracked_free(p1);
    tracked_free(p2);
}
TEST_F(UberTest, AssignAvailableDriver) {
    drivers[0].driverId = 101; drivers[0].available = 1;
    drivers[1].driverId = 102; drivers[1].available = 1;
    driverCount = 2;
    EXPECT_EQ(assignAvailableDriver(), 101);
    EXPECT_EQ(drivers[0].available, 0);
}
TEST_F(UberTest, NoDriverAvailable) {
    drivers[0].driverId = 101; drivers[0].available = 0;
    driverCount = 1;
    EXPECT_EQ(assignAvailableDriver(), -1);
}
TEST_F(UberTest, ReleaseDriver) {
    drivers[0].driverId = 101; drivers[0].available = 0;
    driverCount = 1;
    releaseDriver(101);
    EXPECT_EQ(drivers[0].available, 1);
}
TEST_F(UberTest, RideAllocation) {
    Ride *ride = static_cast<Ride *>(tracked_malloc(sizeof(Ride)));
    ASSERT_NE(ride, nullptr);
    EXPECT_EQ(activeAllocations, 1);
    tracked_free(ride);
    EXPECT_EQ(activeAllocations, 0);
}
TEST_F(UberTest, RideLinkedListInsertion) {
    Ride *ride = static_cast<Ride *>(tracked_malloc(sizeof(Ride)));
    ASSERT_NE(ride, nullptr);
    ride->rideId = 1;
    ride->next = nullptr;
    rideHead = ride;
    EXPECT_EQ(rideHead->rideId, 1);
    tracked_free(ride);
    rideHead = nullptr;
}
TEST_F(UberTest, CompletedRideCounter) {
    completedRides = 0;
    completedRides++;
    EXPECT_EQ(completedRides, 1);
}
TEST_F(UberTest, TotalRequestCounter) {
    totalRequests = 0;
    totalRequests++;
    totalRequests++;
    EXPECT_EQ(totalRequests, 2);
}
TEST_F(UberTest, MultipleDriverAssignments) {
    driverCount = 3;
    for (int i = 0; i < 3; ++i) {
        drivers[i].driverId = 101 + i;
        drivers[i].available = 1;
    }
    EXPECT_EQ(assignAvailableDriver(), 101);
    EXPECT_EQ(assignAvailableDriver(), 102);
    EXPECT_EQ(assignAvailableDriver(), 103);
    EXPECT_EQ(assignAvailableDriver(), -1);
}
TEST_F(UberTest, LeakDetection) {
    void *ptr = tracked_malloc(100);
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(activeAllocations, 1);
    EXPECT_EQ(currentMemory, 100u);
    EXPECT_NE(memoryHead, nullptr);
    // TearDown frees the block; this tests the tracker, not a Valgrind leak.
}
TEST_F(UberTest, CleanupFreesMemory) {
    Ride *r1 = static_cast<Ride *>(tracked_malloc(sizeof(Ride)));
    Ride *r2 = static_cast<Ride *>(tracked_malloc(sizeof(Ride)));
    ASSERT_NE(r1, nullptr);
    ASSERT_NE(r2, nullptr);
    r1->next = r2;
    r2->next = nullptr;
    rideHead = r1;
    cleanup();
    rideHead = nullptr; // Current project cleanup() does not reset this pointer.
    EXPECT_EQ(activeAllocations, 0);
    EXPECT_EQ(currentMemory, 0u);
}
TEST_F(UberTest, DriverReleaseAfterCompletion) {
    driverCount = 1;
    drivers[0].driverId = 101;
    drivers[0].available = 0;
    releaseDriver(101);
    EXPECT_EQ(drivers[0].available, 1);
}
TEST_F(UberTest, MaxDriverLimit) {
    driverCount = MAX_DRIVERS;
    EXPECT_EQ(driverCount, 50);
}
TEST_F(UberTest, MultipleMemoryOperations) {
    void *a = tracked_malloc(50);
    void *b = tracked_malloc(150);
    ASSERT_NE(a, nullptr);
    ASSERT_NE(b, nullptr);
    EXPECT_EQ(currentMemory, 200u);
    EXPECT_EQ(activeAllocations, 2);
    tracked_free(a);
    EXPECT_EQ(currentMemory, 150u);
    EXPECT_EQ(activeAllocations, 1);
    tracked_free(b);
    EXPECT_EQ(currentMemory, 0u);
    EXPECT_EQ(activeAllocations, 0);
}

/* =====================================
   ADDITIONAL CODE COVERAGE TEST CASES
   ===================================== */

// Test 16: Display drivers
TEST_F(UberTest, DisplayDriversCoverage)
{
    driverCount = 2;

    drivers[0].driverId = 101;
    strcpy(drivers[0].name, "Rahul");
    drivers[0].available = 1;

    drivers[1].driverId = 102;
    strcpy(drivers[1].name, "Amit");
    drivers[1].available = 0;

    testing::internal::CaptureStdout();
    displayDrivers();
    std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Rahul"), std::string::npos);
    EXPECT_NE(output.find("Amit"), std::string::npos);
}

// Test 17: Display active rides
TEST_F(UberTest, DisplayRideRequestsCoverage)
{
    Ride *ride = static_cast<Ride *>(
        tracked_malloc(sizeof(Ride)));

    ASSERT_NE(ride, nullptr);

    ride->rideId = 1;
    strcpy(ride->customer, "Kiran");
    ride->assignedDriver = 101;
    ride->next = nullptr;

    rideHead = ride;

    testing::internal::CaptureStdout();
    displayRideRequests();
    std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("Kiran"), std::string::npos);

    cleanup();
    EXPECT_EQ(rideHead, nullptr);
}

// Test 18: Memory statistics
TEST_F(UberTest, MemoryStatisticsCoverage)
{
    void *ptr = tracked_malloc(100);
    ASSERT_NE(ptr, nullptr);

    testing::internal::CaptureStdout();
    memoryStatistics();
    std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_NE(
        output.find("Current Memory"),
        std::string::npos);

    EXPECT_NE(
        output.find("100"),
        std::string::npos);

    tracked_free(ptr);
}

// Test 19: Leak report with active allocation
TEST_F(UberTest, LeakReportCoverage)
{
    void *ptr = tracked_malloc(100);
    ASSERT_NE(ptr, nullptr);

    testing::internal::CaptureStdout();
    leakReport();
    std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_NE(
        output.find("Memory Leak"),
        std::string::npos);

    tracked_free(ptr);
}

// Test 20: Logging
TEST_F(UberTest, SaveLogCoverage)
{
    saveLog("GCOV Coverage Test");

    FILE *fp = fopen("ride_log.txt", "r");
    ASSERT_NE(fp, nullptr);

    bool found = false;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), fp))
    {
        if (strstr(buffer, "GCOV Coverage Test"))
        {
            found = true;
            break;
        }
    }

    fclose(fp);

    EXPECT_TRUE(found);
}
TEST_F(UberTest, NoMemoryLeaksDetected)
{
    void *ptr = tracked_malloc(100);
    ASSERT_NE(ptr, nullptr);

    tracked_free(ptr);

    testing::internal::CaptureStdout();

    leakReport();

    std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_NE(
        output.find("No Memory Leaks Detected"),
        std::string::npos
    );
}


TEST_F(UberTest, RideNotFoundCoverage)
{
    // Create a ride with ID 101
    Ride *ride = (Ride *)tracked_malloc(sizeof(Ride));
    ASSERT_NE(ride, nullptr);

    ride->rideId = 101;
    ride->next = nullptr;
    rideHead = ride;

    // Simulate keyboard input: 999
    FILE *input = tmpfile();
    ASSERT_NE(input, nullptr);

    fprintf(input, "999\n");
    rewind(input);

    // Save original stdin
    int originalStdin = dup(STDIN_FILENO);
    ASSERT_NE(originalStdin, -1);

    // Redirect stdin to our test input
    ASSERT_NE(dup2(fileno(input), STDIN_FILENO), -1);

    // Capture output
    testing::internal::CaptureStdout();

    processRide();

    std::string output =
        testing::internal::GetCapturedStdout();

    // Restore stdin
    dup2(originalStdin, STDIN_FILENO);
    close(originalStdin);
    fclose(input);

    // Check expected result
    EXPECT_NE(
        output.find("Ride Not Found"),
        std::string::npos
    );

    // The existing ride should remain in the list
    EXPECT_EQ(rideHead, ride);

    cleanup();
}


TEST_F(UberTest, ProcessFirstRideCoverage)
{
    Ride *ride = (Ride *)tracked_malloc(sizeof(Ride));
    ASSERT_NE(ride, nullptr);

    ride->rideId = 101;
    strcpy(ride->customer, "Rahul");
    ride->assignedDriver = 1;
    ride->next = nullptr;

    rideHead = ride;

    int before = completedRides;

    runProcessRideWithInput(101);

    EXPECT_EQ(completedRides, before + 1);
    EXPECT_EQ(rideHead, nullptr);
}


TEST_F(UberTest, ProcessSecondRideCoverage)
{
    Ride *first = (Ride *)tracked_malloc(sizeof(Ride));
    Ride *second = (Ride *)tracked_malloc(sizeof(Ride));

    ASSERT_NE(first, nullptr);
    ASSERT_NE(second, nullptr);

    first->rideId = 101;
    strcpy(first->customer, "Rahul");
    first->assignedDriver = 1;

    second->rideId = 102;
    strcpy(second->customer, "Amit");
    second->assignedDriver = 2;

    first->next = second;
    second->next = nullptr;

    rideHead = first;

    int before = completedRides;

    runProcessRideWithInput(102);

    EXPECT_EQ(completedRides, before + 1);
    EXPECT_EQ(rideHead, first);
    EXPECT_EQ(first->next, nullptr);

    cleanup();
}


TEST_F(UberTest, DriverLimitReachedCoverage)
{
    driverCount = MAX_DRIVERS;

    testing::internal::CaptureStdout();

    addDriver();

    std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_NE(
        output.find("Max Driver Limit Reached"),
        std::string::npos
    );

    EXPECT_EQ(driverCount, MAX_DRIVERS);
}


TEST_F(UberTest, AddDriverSuccessCoverage)
{
    FILE *input = tmpfile();
    ASSERT_NE(input, nullptr);

    fprintf(input, "101\nRahul\n");
    rewind(input);

    int savedStdin = dup(STDIN_FILENO);
    ASSERT_NE(savedStdin, -1);

    clearerr(stdin);
    ASSERT_NE(dup2(fileno(input), STDIN_FILENO), -1);

    addDriver();

    dup2(savedStdin, STDIN_FILENO);
    close(savedStdin);
    fclose(input);
    clearerr(stdin);

    ASSERT_EQ(driverCount, 1);
    EXPECT_EQ(drivers[0].driverId, 101);
    EXPECT_STREQ(drivers[0].name, "Rahul");
    EXPECT_EQ(drivers[0].available, 1);
}


TEST_F(UberTest, AddRideWithoutDriverCoverage)
{
    driverCount = 0;

    FILE *input = tmpfile();
    ASSERT_NE(input, nullptr);

    fprintf(input, "201\nKiran\nHyderabad\nAirport\n");
    rewind(input);

    int savedStdin = dup(STDIN_FILENO);
    ASSERT_NE(savedStdin, -1);

    clearerr(stdin);
    ASSERT_NE(dup2(fileno(input), STDIN_FILENO), -1);

    addRideRequest();

    dup2(savedStdin, STDIN_FILENO);
    close(savedStdin);
    fclose(input);
    clearerr(stdin);

    ASSERT_NE(rideHead, nullptr);

    EXPECT_EQ(rideHead->rideId, 201);
    EXPECT_STREQ(rideHead->customer, "Kiran");
    EXPECT_EQ(rideHead->assignedDriver, -1);
    EXPECT_EQ(totalRequests, 1);

    cleanup();
}


TEST_F(UberTest, AddRideWithDriverCoverage)
{
    driverCount = 1;

    drivers[0].driverId = 101;
    strcpy(drivers[0].name, "Rahul");
    drivers[0].available = 1;

    FILE *input = tmpfile();
    ASSERT_NE(input, nullptr);

    fprintf(input, "202\nAmit\nKukatpally\nHitechCity\n");
    rewind(input);

    int savedStdin = dup(STDIN_FILENO);
    ASSERT_NE(savedStdin, -1);

    clearerr(stdin);
    ASSERT_NE(dup2(fileno(input), STDIN_FILENO), -1);

    addRideRequest();

    dup2(savedStdin, STDIN_FILENO);
    close(savedStdin);
    fclose(input);
    clearerr(stdin);

    ASSERT_NE(rideHead, nullptr);

    EXPECT_EQ(rideHead->rideId, 202);
    EXPECT_EQ(rideHead->assignedDriver, 101);
    EXPECT_EQ(drivers[0].available, 0);
    EXPECT_EQ(totalRequests, 1);

    cleanup();
}

TEST(UberMemoryTest, FreeNullPointer)
{
    tracked_free(NULL);

    SUCCEED();
}

TEST(UberMemoryTest, FreeUntrackedPointer)
{
    int value = 100;

    testing::internal::CaptureStderr();

    tracked_free(&value);

    std::string output =
        testing::internal::GetCapturedStderr();

    EXPECT_NE(output.find("Pointer not found"),
              std::string::npos);
}

