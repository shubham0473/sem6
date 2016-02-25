#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

using namespace std;

int main(int argc, char* argv[]) {
	assert(argc > 4);
	int itr = atoi(argv[1]);
	int priority = atoi(argv[2]);
	double sleepProb = atoi(argv[3]);
	int sleepTime = atoi(argv[4]);

	srand(time(NULL));

	for(int i = 0; i < itr; i++) {

	}
}
