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
static int currentNumOfCarsInTunnel;
static int maxNumOfCarsAllowedInTunnel;
static pthread_mutex_t trafficLock;
static pthread_cond_t ok = PTHREAD_COND_INITIALIZER;
static int carCounter;
static char traffic; 
bool done; 


struct car
{
	string goingTo;
	int arrivalTime;
    int tunnelTime; 
	int exitTime;
};


bool enterTunnel(int carCounter)
{

	if (carCounter > maxNumOfCarsAllowedInTunnel)
		return false;


	pthread_mutex_lock(&trafficLock);
	while((carCounter + currentNumOfCarsInTunnel) > maxNumOfCarsAllowedInTunnel)
		pthread_cond_wait(&ok, &trafficLock);

	currentNumOfCarsInTunnel = currentNumOfCarsInTunnel + carCounter;
	pthread_cond_signal(&ok);
	pthread_mutex_unlock(&trafficLock);

	return true;
}

void exitTunnel(int carCounter)
{
	pthread_mutex_lock(&trafficLock);
	currentNumOfCarsInTunnel = currentNumOfCarsInTunnel - carCounter;
	pthread_cond_signal(&ok);
	pthread_mutex_unlock(&trafficLock);
}

void *cars(void *arg)
{
	car cars = *((car*) arg);

	cout << "Car going to "<< cars.goingTo << " arrives at the tunnel"<< endl;

	if (enterTunnel(carCounter) == false)
	{
		cout << cars.goingTo << " " << " exceeds number of cars allowed in the tunnel " << endl;
    }
	else
	{
		cout << cars.goingTo << " is going through the tunnel" << endl;
		sleep(cars.exitTime);
	    exitTunnel(carCounter);
	    cout << cars.goingTo << " is leaving the tunnel" << endl;
	 
	}
	
	pthread_exit(NULL);
}


void runProgram()
{
	pthread_mutex_init(&trafficLock, NULL);

	deque<car> listOfCars;

	string line;

    getline(cin, line);
    stringstream stream(line);
    stream >> maxNumOfCarsAllowedInTunnel;
    cout << "\nmaximum number of cars allowed in tunnel: " << maxNumOfCarsAllowedInTunnel << endl; 
  
	while (getline(cin, line))
	{
		stringstream stream(line);

		car vehicle;
        stream >> vehicle.arrivalTime  >>  vehicle.goingTo >> vehicle.tunnelTime;

        cout << "\nCar arrival time: " << vehicle.arrivalTime  << endl; 
        cout << "Car is going to: " << vehicle.goingTo << endl; 
        vehicle.exitTime = vehicle.arrivalTime + vehicle.tunnelTime; 
        cout << "Car will exit the tunnel at: " << vehicle.exitTime  << endl; 
        cout << " " << endl; 	
        listOfCars.push_back(vehicle);
		carCounter++;
        cout << "the number of cars in total is: " << carCounter << endl;
	}

	deque<pthread_t> listOfthings;

	for (int i = 0; i < listOfCars.size(); i++)
	{
		sleep(listOfCars.at(i).arrivalTime);
		pthread_t tid;
		pthread_create(&tid, NULL, cars, (void *) &listOfCars.at(i));
		listOfthings.push_back(tid);
	}
	for (int i = 0; i < listOfthings.size(); i++)
		pthread_join(listOfthings[i], NULL);
}

int main (int argc, char *argv[] )   //char *argv[])
{   

	
	// maxNumOfCarsAllowedInTunnel = atoi(argv[1]);
	// cout << "Max bridge load: " << maxNumOfCarsAllowedInTunnel << " tons" << endl;
	runProgram();


	cout << "Total number of vehicles: " << carCounter << endl;

   
	return 0;
}
