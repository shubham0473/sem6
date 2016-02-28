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
		cout << "Enter number of iterations:\n";
		cin >> itr;
		cout << "Enter priority:";
		cin >> priority;
		cout << "Enter sleep probability\n";
		cin >> sleepProb;
		cout << "Enter sleep time\n";
		cin >> sleepTime;



		int child = fork();
		if(child == 0) {
			printf("child started\n");
			char* args[11];
			args[0] = strdup("xterm");
			args[1] = strdup("-hold");
			args[2] = strdup("-title");
			asprintf(&args[3], "P%d:,itr:%d,prio:%d,sprob:%lf,stime:%d", i, itr, priority, sleepProb, sleepTime);
			args[4] = strdup("-e");
			asprintf(&args[5], "%s/./process", getcwd(NULL, 0));
			asprintf(&args[6], "%d", itr);
			asprintf(&args[7], "%d", priority);
			asprintf(&args[8], "%lf", sleepProb);
			asprintf(&args[9], "%d", sleepTime);
			asprintf(&args[10], "%d", schedID);
			//printf("%s, %s, %s, %s, %s, %s, %s, %s, %s, %s, %s\n", args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9], args[10]);
			execl("/usr/bin/xterm", args[0], args[1], args[2], args[3], args[4],
				args[5], args[6], args[7], args[8], args[9], args[10], NULL);
			// printf("sfs\n");
		}
		sleep(t);
	}

	for(int i = 0; i < N; i++) wait();

	return 0;
}
