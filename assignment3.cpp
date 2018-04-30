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

//Global Variables that variables shared by all of the threads 
static int currentNumOfCarsInTunnel; 
static int maxNumOfCarsAllowedInTunnel;
static int NCarsBearValleyBound;
static int NCarsWhittierBound; 
static int NDelayedCars; 
static string trafficDirection; 


static pthread_mutex_t trafficLock;
// Condition to wait for until the thread is awake
static pthread_cond_t wakeUp = PTHREAD_COND_INITIALIZER;
static pthread_cond_t specialWakeUp= PTHREAD_COND_INITIALIZER;
// For giving car's their ID's.
static int carCounter;
bool done; 

struct vehicle {
	string isGoingTo;
	string ID; 
	int arrivalTime;
    int tunnelTime; 
    bool isDelayed;
};

// To change the flow of direction until the end of the program. 
void *checkTunnelDirection(void *arg) {
	
	while(done == 0) { 
		
		pthread_mutex_lock(&trafficLock);
		trafficDirection = "WB";
		cout << "\nThe tunnel is now open to Whittier-bound traffic. \n" << endl;
		pthread_cond_broadcast(&specialWakeUp);
		pthread_cond_broadcast(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
		sleep(5);
	
		pthread_mutex_lock(&trafficLock);
		trafficDirection = "N";
		cout << "\nThe tunnel is now closed to ALL traffic.\n" << endl;
		pthread_mutex_unlock(&trafficLock);
		sleep(5);
	
		pthread_mutex_lock(&trafficLock);
		cout << "\nThe tunnel is now open to Bear Valley-bound traffic. \n" << endl;
		trafficDirection ="BB";
		pthread_cond_broadcast(&specialWakeUp);
		pthread_cond_broadcast(&wakeUp);
	
		pthread_mutex_unlock(&trafficLock);
		sleep(5);

		pthread_mutex_lock(&trafficLock);
		trafficDirection = "N";
		cout << "\nThe tunnel is now closed to ALL traffic.\n" << endl;
		pthread_mutex_unlock(&trafficLock);
		sleep(5);
	}
	pthread_exit(NULL);
}

string printDest(string a) {
	if(a == "WB"){
		return "Whittier";
	}else if(a == "BB"){
		return "Bear Valley";
	}else{
		return " ";
	}
}

// To keep account for the number of cars arriving to each respective destination. 
void destBoundCount(struct vehicle cars){
	if(cars.isGoingTo == "WB"){
		pthread_mutex_lock(&trafficLock);
		NCarsWhittierBound = NCarsWhittierBound + 1;
		pthread_cond_signal(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
	}
	if (cars.isGoingTo == "BB"){
		pthread_mutex_lock(&trafficLock);
		NCarsBearValleyBound = NCarsBearValleyBound + 1;
		pthread_cond_signal(&wakeUp);
		pthread_mutex_unlock(&trafficLock);
	} 
}

// To account for a car entering the tunnel.
void enterTunnel(struct vehicle cars){
	pthread_mutex_lock(&trafficLock);
	currentNumOfCarsInTunnel = currentNumOfCarsInTunnel + 1; 
	cout << "Car # " << cars.ID << " going to "<< printDest(cars.isGoingTo) << " enters the tunnel" << endl;
	pthread_cond_signal(&wakeUp);
	pthread_mutex_unlock(&trafficLock);
	sleep(cars.tunnelTime); 
}

// To account for car exiting the tunnel. 
void exitTunnel(struct vehicle cars) {
	
	pthread_mutex_lock(&trafficLock);
	cout << "Car # " << cars.ID << " going to "<< printDest(cars.isGoingTo) << " exits the tunnel" << endl; 
	currentNumOfCarsInTunnel = currentNumOfCarsInTunnel - 1; 
	pthread_cond_signal(&wakeUp);
	pthread_mutex_unlock(&trafficLock);
}

// Concurrent function for car threads.
void *cars(void *arg) {
	
	vehicle cars = *((vehicle*) arg);
	cout << "Car # " << cars.ID << " going to "<< printDest(cars.isGoingTo) << " arrives at the tunnel"<< endl;
	// Case 1: The car is going the same direction as traffic and the tunnel has room so car enters. 
    if(trafficDirection == cars.isGoingTo && currentNumOfCarsInTunnel <= maxNumOfCarsAllowedInTunnel){	
		// car enters the tunnel 
		enterTunnel(cars);
		// sending the car out of the tunnel and signaling..
		exitTunnel(cars);
		// Counting how many of each car goes to WB and BB. 
		destBoundCount(cars);


	} // Case 2: The car is not going the same direction as traffic, car must wait but is not delayed.
	else if(trafficDirection != cars.isGoingTo && currentNumOfCarsInTunnel <= maxNumOfCarsAllowedInTunnel) {	

		pthread_mutex_lock(&trafficLock);
		while(trafficDirection != cars.isGoingTo)
			pthread_cond_wait(&wakeUp, &trafficLock);
		pthread_mutex_unlock(&trafficLock);
		
		enterTunnel(cars);
		exitTunnel(cars);
		destBoundCount(cars);
		
	}//Case 3: The traffic flow is the same yet the car must wait to enter, resulting in a delay.
	else if(trafficDirection == cars.isGoingTo && currentNumOfCarsInTunnel > maxNumOfCarsAllowedInTunnel) {
		
		pthread_mutex_lock(&trafficLock);
		// Wait until the number of cars in the tunnel is less than the maximum amount allowed. 
		while((currentNumOfCarsInTunnel) > maxNumOfCarsAllowedInTunnel) 
			pthread_cond_wait(&wakeUp, &trafficLock);
		NDelayedCars = NDelayedCars + 1; 
		pthread_mutex_unlock(&trafficLock);
		
		enterTunnel(cars);
		exitTunnel(cars);
		destBoundCount(cars);
		
	}// Case 4: The traffic flow is not the same and the tunnel is full therefore the car is delayed. 
	else if(trafficDirection != cars.isGoingTo && currentNumOfCarsInTunnel > maxNumOfCarsAllowedInTunnel) {
	
		pthread_mutex_lock(&trafficLock);
		// Wait until the number of cars in the tunnel is less than the maximum amount allowed
		// and traffic flows in the same direction (specialWakeUp). 
		while((currentNumOfCarsInTunnel) > maxNumOfCarsAllowedInTunnel) // and direction as well
			pthread_cond_wait(&specialWakeUp, &trafficLock);
		NDelayedCars = NDelayedCars + 1; 
		pthread_mutex_unlock(&trafficLock);
		
		enterTunnel(cars);
		exitTunnel(cars);
		destBoundCount(cars);
	
	}// Case 5: Undefined behavior. Ideally, the car should not end up here.EVER.
	else {
		
		pthread_mutex_lock(&trafficLock);
		while((currentNumOfCarsInTunnel) > maxNumOfCarsAllowedInTunnel) // and direction as well
			pthread_cond_wait(&wakeUp, &trafficLock);
		NDelayedCars = NDelayedCars + 1;
		pthread_mutex_unlock(&trafficLock);

		// Entering the tunnel
		enterTunnel(cars);
		exitTunnel(cars);
		// To account for the number of cars for each destination. 
		destBoundCount(cars);
	}
	// Close the thread. 
	pthread_exit(NULL);
}


// Driver that starts the program.
void runProgram()
{
	pthread_mutex_init(&trafficLock, NULL);
	deque<vehicle> listOfCars; 
	string line;
    getline(cin, line);
    stringstream stream(line);
	stream >> maxNumOfCarsAllowedInTunnel;

	// Instantiate a new vehicle named car and store corresponding data into it
	while (getline(cin, line)){
		stringstream stream(line);
		vehicle car;string arrtime,tuntime;
        stream >> arrtime  >>  car.isGoingTo >> tuntime;
		carCounter++;
		car.arrivalTime = stoi(arrtime); // converting using stoi to be safe
		car.tunnelTime = stoi(tuntime);
		car.ID = std::to_string(carCounter);
		listOfCars.push_back(car);
	}

	deque<pthread_t> listCarsForTunnel; 
	// Spawn a thread for the tunnel.
	pthread_t tunnel; 
	pthread_create (&tunnel, NULL, checkTunnelDirection, NULL); 
	
	// Spawn threads for the cars. 
	for (int i = 0; i < listOfCars.size(); i++){
		sleep(listOfCars.at(i).arrivalTime);
		pthread_t tid;
		pthread_create(&tid, NULL, cars, (void *) &listOfCars.at(i));
		listCarsForTunnel.push_back(tid);
	}

	// Wait for the threads to finish. 
	for (int i = 0; i < listCarsForTunnel.size(); i++)
		pthread_join(listCarsForTunnel[i], NULL);
}

int main (int argc, char *argv[] ){   
	runProgram();
	cout << "\n" <<NCarsBearValleyBound << " car(s) going to Bear Valley arrived at the tunnel " << endl; 
	cout << NCarsWhittierBound << " car(s) going to Whittier arrived at the tunnel " << endl; 
	cout << NDelayedCars << " car(s) were delayed " << endl; 
	return 0;
}