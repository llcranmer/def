#include <iostream>
#include <string>
#include <vector> 
#include <stdlib.h>
#include <pthread.h>
#include <queue>
#include <sstream>
#include <iomanip>
#include <unistd.h>
#include <utility>
#include <unistd.h>

using namespace std;

//Global Variables
static int currentNumOfCarsInTunnel;
static int maxNumOfCarsAllowedInTunnel;
// this is my mutex
static pthread_mutex_t trafficLock;

// the initializer...
static pthread_cond_t wb_can = PTHREAD_COND_INITIALIZER;
static pthread_cond_t bb_can = PTHREAD_COND_INITIALIZER;
static int carCounter;
static char trafficLightSignal; 
bool done; 


struct vehicle
{
	string isGoingTo;
	string ID; 
	int arrivalTime;
    int tunnelTime; 
	int exitTime;
	bool delayed = false; 
};
