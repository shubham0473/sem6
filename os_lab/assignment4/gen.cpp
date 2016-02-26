/****************
* gen.c
* Generates N processes at t interval
*
* Usage:
* $ ./gen <schedID>
*
*****************/

#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

using namespace std;

int main(int argc, char* argv[]) {
	int N, t, schedID;
	cout << "Enter number of processes (N):";
	cin >> N;
	cout << "Enter time interval (t):";
	cin >> t;
	//cout << "Enter pid of scheduler:";
	//cin >> schedID;
	schedID = atoi(argv[1]);

	for(int i = 0; i < N; i++) {
		int itr, priority, sleepTime;
		double sleepProb;

		cout << "Process " << i << endl;
		cout << "Enter number of iterations:";
		cin >> itr;
		cout << "Enter priority:";
		cin >> priority;
		cout << "Enter sleep probability";
		cin >> sleepProb;
		cout << "Enter sleep time";
		cin >> sleepTime;

		int child = fork();
		if(child == 0) {
			char* args[8];
			args[0] = strdup("xterm");
			args[1] = strdup("-e");
			args[2] = strdup("./process");
			sprintf(args[3], "%d", itr);
			sprintf(args[4], "%d", priority);
			sprintf(args[5], "%lf", sleepProb);
			sprintf(args[6], "%d", sleepTime);
			sprintf(args[7], "%d", schedID);
			execv("xterm", args);
		}
		sleep(t);
	}

	for(int i = 0; i < N; i++) wait();

	return 0;
}
