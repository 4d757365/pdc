#include <iostream>
#include <stdlib.h>
#include <thread>
#include <vector>
#include "../hpc_helpers.hpp"

using namespace std;

// size of elements
int DATA_SIZE;

// size of thread
int THREAD_SIZE;

// vector to hold elements
vector<int> data;

// final sum of all elements
int sum;


// function to fill vector with random numbers from (0-99)
void populate_data() {
	srand(time(NULL));
	// resize vector to elements size
	data.resize(DATA_SIZE);	
	for(int i=0; i < DATA_SIZE; i++)
		data[i] = rand() % 100;
}

// function to print out elements in vector
void print_data(int start, int end) {
	for(int i=start; i < end; i++)
		cout << data[i] << ", ";
	cout << endl;
}

// function to compute summation with given range (start and end)
void summation(int start, int end) {
	for(int i=start; i < end; i++) 
		sum += data[i];
}


int main() {
	string option;
	cout << "ASSIGNMENT 2: SUMMATION" << endl;
	cout << "ENTER THE NUMBER OF ELEMENTS: ";
	cin >> DATA_SIZE;
	cout << "OPTIONS: \n\t1.NON-THREADED\n\t2.MULTITHREADED" << endl;
	cout << "ENTER OPTION: ";
	cin >> option;
	
	// if option is non-threaded
	if(option == "1") {	
		cout << "\nCOMPUTING " << DATA_SIZE << " ELEMENTS (NON-THREADED SUMMATION)" << endl;	
		
		// fill vector and print the elements
		populate_data();
		print_data(0, DATA_SIZE);
		cout << endl;
		
		// start timer for non-threaded summation
		TIMERSTART(nonthreadedsummation);
		
		// call summation function
		summation(0, DATA_SIZE);
		
		// stop timer for non-threaded summation
		TIMERSTOP(nonthreadedsummation);
		
		// print sum of non-threaded summation
		cout << "\nSUM: " << sum << endl;
	
	// if option is multithreaded	
	} else if(option == "2") {
		cout << "ENTER THE NUMBER OF THREADS: ";
		// set threaded size 
		cin >> THREAD_SIZE;
		cout << "\nCOMPUTING " << DATA_SIZE << " ELEMENTS (USING "<< THREAD_SIZE << " MULTI-THREADED SUMMATION)" << endl;	
		
		// fill vector and print the elemnts
		populate_data();
		print_data(0, DATA_SIZE);
		cout << endl;
		
		// setup a vector to hold the threads
		vector<thread> threads;
		
		// calculate portion of elements per thread
		int portion = DATA_SIZE / THREAD_SIZE;
		
		// start timer for multithreaded summation
		TIMERSTART(multithreadedsummation);

        // loop and assign the range of elements using size of the thread
		for(int i=0; i < THREAD_SIZE; i++) {
			// every portion-th is the start
			int start = i * portion;
			int end;
			
			// if it is about to have more elements than threads left assign the rest
			// of elements to the last threads
			if(i == THREAD_SIZE - 1) {
				end = DATA_SIZE;
			} else {
			    // else assign i-th + 1 of portion
				end = (i + 1) *  portion;
			}	
				
			// print portion for the thread
			//cout << "PORTION: ";
			//print_data(start, end);
			//cout << endl;
			
			// insert the thread with it's start and end range
			threads.emplace_back(summation, start, end);
		}
		//cout << endl;
		
		// join the threads
		for(auto& thread: threads) {
			thread.join();
		}
        
        // stop timer for multithreaded summation
		TIMERSTOP(multithreadedsummation);
        
        // print sum of the multithreaded summation
		cout << "\nSUM: " << sum << endl;
		
	// invalid option breaks the program
	} else {
		cout << "INVALID OPTION. PLEASE TRY AGAIN." << endl;
	}
		
	return 0;
}
