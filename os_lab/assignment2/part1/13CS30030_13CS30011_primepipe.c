#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

int AVAILABLE = 30009;
int BUSY = 30010;

#define LIMIT 30000

int *primes,*numbers;
int nprime;
int i,j,k,l,buffer,n,k;

// Binary search
int binary_search(int n,int array[], int start, int end){
    int middle = (start+end)/2;

    if (start>end)
        return 0;

    if (n==array[middle])
        return 1;

    if (n>array[middle])
        return binary_search(n,array,middle+1, end);
    else
        return binary_search(n,array,start,middle-1);
}

int IsPrime(int number)
{
    if(number<2)
        return 0;
    if(number==2)
        return 1;
    if(number%2==0)
        return 0;
    for(int i = 3;i <= sqrt(number); i += 2)
    {
        if(number%i==0)
            return 0;
    }
    return 1;
}

void generator()
{
    primes = malloc(sizeof(int)*LIMIT);
    numbers = malloc(sizeof(int)*LIMIT);

    // Array of natural numbers
    for (i=0;i<LIMIT;i++){
        numbers[i]=i+2;
    }

    for (i=0;i<LIMIT;i++)
    {
        if (numbers[i]!=-1)
        {
            for (j=0;j<LIMIT;j++)
            {
                if(!IsPrime(numbers[j]))
            	numbers[j]=-1;

            }
        }
    }

    // Create the array of all prime numbers
    j = 0;
    for (i=0;i<LIMIT&&j<LIMIT;i++)
        if (numbers[i]!=-1)
        {
            primes[j++] = numbers[i];
        }

    nprime = j-1;
}

int main(int argc,char *argv[])
{
	// Exit if less arguments
	if(argc<3)
	{
		printf("Incomplete arguments\n");
		return 0;
	}

	n = atoi(argv[1]);
	k = atoi(argv[2]);

	int *prime_ar;
	prime_ar = malloc(sizeof(int)*n);
	int prime_no = 0;
	int found;

	generator();
	time_t t;
	srand((unsigned) time(&t));

	// Create the 2*k pipes
	int f[2*k][2];
	for(i = 0 ; i<2*k; i++)
	{
		pipe(f[i]);
	}
	int root = getpid();

	// Create k Child Processes
	for(i=0;i<k;i++)
	{

		// Only the root can fork
		if(getpid() != root){

        }
        else
		{
	        //Child Process
			if(fork()!=0){

            }
            else
			{
				while(1)
				{
					write(f[i+k][1],&AVAILABLE,sizeof(AVAILABLE));
                    j = 0;
                    while(j<k)
                    {
                        read(f[i][0],&buffer,sizeof(int));
                        write(f[i+k][1],&BUSY,sizeof(BUSY));
                        if(binary_search(buffer,primes,0,nprime)==1)
                        {
                            write(f[i+k][1],&buffer,sizeof(buffer));
                        }
                        j++;
                    }
				}
			}
		}

	}

    if(getpid()==root)
    {
        int c = 0;
        printf("Process Index\tIndex\tPrime\n");
        while(1)
        {
            for(i=0;i<k;i++)
            {
                read(f[i+k][0],&buffer,sizeof(buffer));

                // If Available
                if(buffer<=LIMIT)
                {
                    found = 0;

                    while(l < prime_no){

                        if(buffer!=prime_ar[l]){

                        }
                        else
                        {
                            found = 1;
                        }
                        l++;
                    }

                    if(found == 1){

                    }
                    else
                    {
                        prime_ar[prime_no]=buffer;
                        printf("%d\t\t%d\t%d\n",i,prime_no+1,buffer);
                        prime_no++;

                        if(n==prime_no)
                        {
                            kill(0,SIGKILL);
                            return 0;
                        }
                    }

                }

                else if(buffer == 30009)
                {
                    for(j = 0;j<k;j++)
                    {
                        l = (rand() % LIMIT) + 1;
                        write(f[i][1],&l,sizeof(l));
                    }

                }
            }

        }
    }
    return 0;
}
