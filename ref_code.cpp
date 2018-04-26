//Jorge Garcia-Lizarraga
//Assignment 2
//Operating Systems

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
static int current_bridge_load;
static int max_bridge_load;
static pthread_mutex_t lockMechanism;
static pthread_cond_t ok = PTHREAD_COND_INITIALIZER;
static int carCounter;

struct car
{
	string licensePlate;
	int wieght;
	int timeToGetOn;
	int timeToGetOff;
};

bool enterBridge(int wieght)
{
	if (wieght > max_bridge_load)
		return false;

	pthread_mutex_lock(&lockMechanism);
	while((wieght + current_bridge_load) > max_bridge_load)
		pthread_cond_wait(&ok, &lockMechanism);

	current_bridge_load = current_bridge_load + wieght;
	pthread_cond_signal(&ok);
	pthread_mutex_unlock(&lockMechanism);

	return true;
}

void leaveBridge(int wieght)
{
	pthread_mutex_lock(&lockMechanism);
	current_bridge_load = current_bridge_load - wieght;
	pthread_cond_signal(&ok);
	pthread_mutex_unlock(&lockMechanism);
}

void *cars(void *arg)
{
	car cars = *((car*) arg);

	cout << cars.licensePlate << " has arrived to the bridge. Current Bridge Load: " << current_bridge_load << endl;

	if (enterBridge(cars.wieght) == false)
	{
		cout << cars.licensePlate << " " << " exceeds bridge weight." << endl;
	}

	else
	{
		cout << cars.licensePlate << " is crossing the bridge" << endl;
	    cout << "Current bridge load: " << current_bridge_load << endl;
		sleep(cars.timeToGetOff);
	    leaveBridge(cars.wieght);
	    cout << cars.licensePlate << " is leaving the bridge" << endl;
	    cout << "Current bridge load: " << current_bridge_load << endl;
	}
	
	pthread_exit(NULL);
}


void runProgram()
{
	pthread_mutex_init(&lockMechanism, NULL);

	deque<car> listOfCars;

	string line;
	while (getline(cin, line))
	{
		stringstream stream(line);

		car vehicle;
		stream >> vehicle.licensePlate >> vehicle.timeToGetOn >> vehicle.wieght >> vehicle.timeToGetOff;
		listOfCars.push_back(vehicle);
		carCounter++;
	}

	deque<pthread_t> listOfthings;

	for (int i = 0; i < listOfCars.size(); i++)
	{
		sleep(listOfCars.at(i).timeToGetOn);
		pthread_t tid;
		pthread_create(&tid, NULL, cars, (void *) &listOfCars.at(i));
		listOfthings.push_back(tid);
	}
	for (int i = 0; i < listOfthings.size(); i++)
		pthread_join(listOfthings[i], NULL);
}

int main (int argc, char *argv[])
{
	if (argc < 2) 
	{
		cout << "Not enough parameters" << endl;

		return 0;
	}

	max_bridge_load = atoi(argv[1]);
	cout << "Max bridge load: " << max_bridge_load << " tons" << endl;
	runProgram();

	cout << "Total number of vehicles: " << carCounter;

	return 0;
}
