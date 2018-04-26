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
static pthread_mutex_t trafficLock;
static pthread_cond_t wb_can = PTHREAD_COND_INITIALIZER;
static pthread_cond_t bb_can = PTHREAD_COND_INITIALIZER;
static int carCounter;
static char traffic; 
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


bool enterTunnel(int carCounter)
{
	cout << "entering tunnel portion of program" << endl;
	// while(done == 0) { 
	// 	pthread_mutex_lock(&trafficLock);
	// 	traffic = 'W';
	// 	cout << "The tunnel is now open to Whittier bound traffic. \n" << endl;
	// 	pthread_cond_broadcast(&wb_can);
	// 	pthread_mutex_unlock(&trafficLock);
	// 	sleep(5);
	// 	traffic = 'N';
	// 	cout << "The tunnel is now closed to ALL traffic.\n" << endl;
	// 	pthread_mutex_unlock(&trafficLock);
	// 	sleep(5);
	// 	cout << "The tunnel is now open to Bear Valley bound traffic. \n" << endl;
	// 	pthread_cond_broadcast(&wb_can);
	// 	pthread_mutex_unlock(&trafficLock);
	// 	sleep(5);
		
	// }

	// will not make it to this code ...
	if (carCounter > maxNumOfCarsAllowedInTunnel)
		return false;


	pthread_mutex_lock(&trafficLock);
	while((carCounter + currentNumOfCarsInTunnel) > maxNumOfCarsAllowedInTunnel)
		pthread_cond_wait(&wb_can, &trafficLock);

	currentNumOfCarsInTunnel +=  carCounter;
	pthread_cond_signal(&wb_can);
	pthread_mutex_unlock(&trafficLock);

	return true;
}

void exitTunnel(int carCounter)
{
	pthread_mutex_lock(&trafficLock);
	currentNumOfCarsInTunnel -= carCounter;
	pthread_cond_signal(&wb_can);
	pthread_mutex_unlock(&trafficLock);
}

void *cars(void *arg)
{
	vehicle cars = *((vehicle*) arg);

	cout << "\nCar # " << cars.ID << " going to "<< cars.isGoingTo << " arrives at the tunnel"<< endl;

	if (enterTunnel(carCounter) == false)
	{
		cout << cars.isGoingTo << " " << " exceeds number of cars allowed in the tunnel " << endl;
    }
	else
	{
		cout << "Car # " << cars.ID << " going to "<< cars.isGoingTo << " is going through the tunnel" << endl;
		sleep(cars.exitTime);
	    // exitTunnel(carCounter);
	    cout << "Car # " << cars.ID << " going to "<< cars.isGoingTo << " is leaving the tunnel" << endl;
	 
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

	for (int i = 0; i < listOfCars.size(); i++)
	{
		sleep(listOfCars.at(i).arrivalTime);
		pthread_t tid;
		pthread_create(&tid, NULL, cars, (void *) &listOfCars.at(i));
		listCarsForTunnel.push_back(tid);
	}
	for (int i = 0; i < listCarsForTunnel.size(); i++)
		pthread_join(listCarsForTunnel[i], NULL);
}

int main (int argc, char *argv[] )   //char *argv[])
{   

	
	// maxNumOfCarsAllowedInTunnel = atoi(argv[1]);
	// cout << "Max bridge load: " << maxNumOfCarsAllowedInTunnel << " tons" << endl;
	runProgram();


	cout << "Total number of vehicles: " << carCounter << endl;

   
	return 0;
}
