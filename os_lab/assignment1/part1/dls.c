// Implementation of Distributed Linear Search (DLS)
// using the concepts of Linux System calls. In DLS, the array is split up into
// subarrays and searching the sub-arrays in delegated to other processes (hence
// distributed).
/*
Name: Shubham Agrawal, Divyansh Gupta
Roll: 13CS30030, 13CS30011
*/
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// Note: returning -1 from exit() does not work because WEXITSTATUS()
// reads the least 8 bits of the status returned by waitpid().
// Hence 0 is used to signal that the process did not find the value.
// Consequently, the values are stored from A[1] onwards.

#define MAX_SIZE 1000000

int main(int argc, char* argv[]) {
    if(argc < 3) {
        printf("Error: Insufficient arguments");
        return 0;
    }

    int mainpid = getpid();

    FILE* fp = fopen(argv[1], "r");

    int A[MAX_SIZE];

    int i;
    for(i = 1; fscanf(fp, "%d", &A[i]) != EOF; i++);

    fclose(fp);

    int left = 1, right = i - 1, mid;

    int k = atoi(argv[2]), retval = 0;


    while(1) {
        if(right - left < 5) { // base case -> linear search
            for(i = left; i <= right; i++) {
                if(A[i] == k) {
                    if(getpid() != mainpid) { // not the primary process
                        exit(i); // return index via exit code
                    }
                    else { // primary process
                        retval = i;
                        break;
                    }
                }

            }

            if(getpid() != mainpid) {
                exit(0); //value not found
            }
            else {
                break;
            }
        }
        else { // spawn child processes
            int cid1, cid2, status1, status2;
            mid = (left + right) / 2;

            if((cid1 = fork()) == 0) { // child process 1
                right = mid;
            }
            else if((cid2 = fork()) == 0) { // child process 2
                left = mid + 1;
            }
            else { // parent process
                waitpid(cid1, &status1, 0);
                int left_result = WEXITSTATUS(status1);
                waitpid(cid2, &status2, 0);
                int right_result = WEXITSTATUS(status2);

                if(left_result != 0) {
                    retval = left_result;
                }
                else if(right_result != 0) {
                    retval = right_result;
                }
                else {
                    retval = 0;
                }

                if(getpid() != mainpid) {
                    exit(retval);
                }
                break; // parent process shouldnt loop
            }
        }
    }

    if(retval != 0)
        printf("Value %d found at position %d in file \"%s\"\n", k, retval - 1, argv[1]);
    else
        printf("Value %d not found in file \"%s\" \n", k, argv[1]);

    return 0;
}
