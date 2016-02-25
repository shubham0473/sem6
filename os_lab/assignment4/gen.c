#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

using namespace std;

int main() {
	int N, t;
	cout << "Enter number of processes (N):";
	cin >> N;
	cout << "Enter time interval (t):";
	cin >> t;

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
			char* args[7];
			args[0] = "xterm";
			args[1] = "-e";
			args[2] = "process";
			sprintf(args[3], "%d", itr);
			sprintf(args[4], "%d", priority);
			sprintf(args[5], "%lf", sleepProb);
			sprintf(args[6], "%d", sleepTime);
			execv("xterm", args);
		}

		sleep(t);
	}

	return 0;
}
