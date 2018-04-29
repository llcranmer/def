#include <iostream>
#include <string>
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
// variables shared by all of the threads 
static int currentNumOfCarsInTunnel; // use a mutex to update this one 
static int maxNumOfCarsAllowedInTunnel;
static int NCarsBearValleyBound;
static int NCarsWhittierBound; 
static int NDelayedCars; 
static string trafficDirection; // which flow of direction is permitted


// one mutex 
static pthread_mutex_t trafficLock;
static pthread_cond_t wakeUp = PTHREAD_COND_INITIALIZER;
static int carCounter;

bool done; 


struct vehicle
{
	string isGoingTo;
	string ID; 
	int arrivalTime;
    int tunnelTime; 
	bool delayed = false; 
};

// does not have access to the cars and their data ..
void *checkTunnelDirection(void *arg){
	
	while(done == 0) { 

		pthread_mutex_lock(&trafficLock);
		trafficDirection = "WB";
		cout << "The tunnel is now open to Whittier bound traffic. \n" << endl;
		pthread_cond_broadcast(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
		sleep(5);
	
		trafficDirection = "N";
		cout << "The tunnel is now closed to ALL traffic.\n" << endl;
		pthread_mutex_unlock(&trafficLock);
		sleep(5);
	
		cout << "The tunnel is now open to Bear Valley bound traffic. \n" << endl;
		trafficDirection ="BB";
		pthread_cond_broadcast(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
		sleep(5);

		trafficDirection = "N";
		cout << "The tunnel is now closed to ALL traffic.\n" << endl;
		pthread_mutex_unlock(&trafficLock);
		sleep(5);
		
	}

	pthread_exit(NULL);
}


// has access to each individual and it's data 
// is the concurrent function 
void *cars(void *arg)
{
	vehicle cars = *((vehicle*) arg);
	// print message : "arrived at tunnel"
	cout << "\nCar # " << cars.ID << " going to "<< cars.isGoingTo << " arrives at the tunnel"<< endl;
    if(trafficDirection == cars.isGoingTo && currentNumOfCarsInTunnel < maxNumOfCarsAllowedInTunnel)
	{	
		// entering the tunnel 
		// request the trafficLock mutex
		pthread_mutex_lock(&trafficLock);
		// Increment the number of cars in the tunnel
		currentNumOfCarsInTunnel = currentNumOfCarsInTunnel + carCounter;
		cout << "current number of cars in the tunnel:" << currentNumOfCarsInTunnel << endl; 
		cout << "Car # " << cars.ID << " going to "<< cars.isGoingTo << " enters the tunnel" << endl;
		// sleep the duration of the crossing time 
		sleep(cars.tunnelTime);
		pthread_cond_signal(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
		
		// exiting the tunnel 
		// Request trafficLock Mutex
		// decrement the number of cars in the tunnel 
		pthread_mutex_lock(&trafficLock);
		cout << "Car # " << cars.ID << " going to "<< cars.isGoingTo << " exits the tunnel" << endl;
		currentNumOfCarsInTunnel = currentNumOfCarsInTunnel - carCounter;
		cout << "current number of cars in the tunnel: " << currentNumOfCarsInTunnel << endl; 
		pthread_cond_signal(&wakeUp);
		pthread_mutex_unlock(&trafficLock);

	} else if(trafficDirection != cars.isGoingTo && currentNumOfCarsInTunnel < maxNumOfCarsAllowedInTunnel) {	
		// if needed then wait for a broadcast from tunnel 
		cout << "\nCars going to " << cars.isGoingTo << " are not allowed entry at this point in time " << endl;
		
		pthread_mutex_lock(&trafficLock);
		while(trafficDirection != cars.isGoingTo)
			pthread_cond_wait(&wakeUp, &trafficLock);
		pthread_mutex_unlock(&trafficLock);

		// Entering the tunnel
		pthread_mutex_lock(&trafficLock);
		currentNumOfCarsInTunnel = currentNumOfCarsInTunnel + carCounter;
		cout << "current number of cars in the tunnel:" << currentNumOfCarsInTunnel << endl; 
		cout << "Car # " << cars.ID << " going to "<< cars.isGoingTo << " enters the tunnel" << endl;
		// sleep the duration of the crossing time 
		sleep(cars.tunnelTime);
		pthread_cond_signal(&wakeUp);
		pthread_mutex_unlock(&trafficLock);

		// exiting the tunnel 
		// Request trafficLock Mutex
		// decrement the number of cars in the tunnel 
		pthread_mutex_lock(&trafficLock);
		cout << "Car # " << cars.ID << " going to "<< cars.isGoingTo << " exits the tunnel" << endl;
		currentNumOfCarsInTunnel = currentNumOfCarsInTunnel - carCounter;
		cout << "current number of cars in the tunnel: " << currentNumOfCarsInTunnel << endl; 
		pthread_cond_signal(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
		
		
		
		cout << "test 1" << endl;
		
	} else {
		// wait for a car leaving
		cout << "The tunnel is full therefore you must wait!" << endl; 
		while((carCounter + currentNumOfCarsInTunnel) > maxNumOfCarsAllowedInTunnel) // and direction as well
			pthread_cond_wait(&wakeUp, &trafficLock);

	}

	pthread_exit(NULL);

	
}


void runProgram()
{
	pthread_mutex_init(&trafficLock, NULL);
	deque<vehicle> listOfCars; 
	string line;

    getline(cin, line);

    stringstream stream(line);
    // read in the first line to get the amount of cars allowed 
	stream >> maxNumOfCarsAllowedInTunnel;
    cout << "\nmaximum number of cars allowed in tunnel: " << maxNumOfCarsAllowedInTunnel << endl; 
  
	while (getline(cin, line))
	{
		stringstream stream(line);
		// Instantiate a new vehicle named car and store corresponding data into it
		vehicle car;
        stream >> car.arrivalTime  >>  car.isGoingTo >> car.tunnelTime;
		carCounter++;
		car.ID = std::to_string(carCounter);
        
		// add cars to dequeue 
		listOfCars.push_back(car);
	}

	deque<pthread_t> listCarsForTunnel; 
	
	// spawning a new thread 
	pthread_t tunnel; 
	pthread_create (&tunnel, NULL, checkTunnelDirection, NULL); 

	for (int i = 0; i < listOfCars.size(); i++)
	{
		sleep(listOfCars.at(i).arrivalTime);
		pthread_t tid;
		pthread_create(&tid, NULL, cars, (void *) &listOfCars.at(i));
		listCarsForTunnel.push_back(tid);
	}

	// waiting for them to finish
	for (int i = 0; i < listCarsForTunnel.size(); i++)
		pthread_join(listCarsForTunnel[i], NULL);

}


 
int main (int argc, char *argv[] )  
{   
	runProgram();
	cout << "Total number of vehicles: " << carCounter << endl;

	return 0;
}