/*
 * mpi.c
 * 
 * Reads a file and returns the highest associated ASCII value found in each line
 *
 * Ailthon Cerna 4/14/2025
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include <errno.h>
#include <time.h>


#define MAX_LINES 1000

u_int16_t* results; //Global for Results storage
int NUM_THREADS;

/* Function Prototypes */
int find_max(char* line, int count);
void read_lines(void* id);

int main(int argc, char *argv[])
{
	/* Clock Times for Process Calculations */
	clock_t tstart, tend; 
	double elapsedTime = 0;
	tstart = clock(); 
	
	int rc, numTasks, rank;
	
	/* Init MPI */
	rc = MPI_Init(&argc,&argv);
    if (rc != MPI_SUCCESS) 
	{
      printf ("Error starting MPI program. Terminating.\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      return 0;
    }
    
    MPI_Comm_size(MPI_COMM_WORLD,&numTasks);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	NUM_THREADS = numTasks;
	
	/* Initialize Memory */
	
	results = malloc(sizeof(u_int16_t)*MAX_LINES); //Allocate memory for every line
	if(results == NULL)
	{
		printf("Memory allocation for array 'Results' was unsuccessful");
		return 0;
	}
	
	printf("Cluster Size = %d Current Rank = %d\n", numTasks, rank); //Get Size and Rank from MPI
    fflush(stdout);
	
	/* Read the file into the the list of entries */

	read_lines(&rank);

	
    tend = clock(); //end execution time
    elapsedTime = (double)(tend - tstart) / CLOCKS_PER_SEC; //calculate total execution time
    MPI_Barrier(MPI_COMM_WORLD); //Sync Process Ends
	
	if ( rank == 0 ) 
	{
		for (int j = 0; j < MAX_LINES; j++) //Get ASCII results from every line
		{ 
			printf("%d: %d\n", j, results[j]);
		}		
    }
	MPI_Barrier(MPI_COMM_WORLD); //Sync Print Statements
    printf( "The run on core %d took %lf seconds for %d lines\n",
           rank, elapsedTime, MAX_LINES); //print timing results
    free(results);
    printf("Main completed. Exiting.\n");

    //exit program and cleanup
    MPI_Finalize();
    return 0;
	
}

/*	Finds the Maximum ASCII Value in a string and
 *  returns that value in the form of an integer
 *   
 * <char* line> line to parse
 * <int count> number of characters in line
 *
 * <returns> integer value of highest ASCII character
 */ 
int find_max(char* line, int count) 
{
	//valid int values from 32 to 126 inclusive
	
    int i; 				//index count
    int max = 0; 		//smallest printable ASCII character
    int ernsv = errno; 	//Error Handling Report
    
    if (line == NULL || count <= 0)
    {
        printf("Error during line analysis: %i", ernsv);
        return -1;
    }
    
    for (i = 0; i < count; i++) 
	{ //go through all chars in the line
        int letterVal = (int)line[i]; //find ASCII value
        if (letterVal > max && letterVal < 127) //new Max and within printable ASCII values (excluding space)
		{ 
            max = letterVal;
        }
    }
    return max;
}

/* Read the lines found in the text file specified by the assignment and
 * create an association between maximum allocation of lines per process and process rank and
 * return the resulting max ASCII character value found in each line
 *
 * <void* rank>rank of root process
 */
void read_lines(void *rank) 
{    
    //Initialize Variables
    int nchars = 0;
    int myID = *((int*) rank);
    int startLoc = myID * (MAX_LINES  / NUM_THREADS);
    int endLoc = startLoc + (MAX_LINES  / NUM_THREADS);
    int err = 0;
    
    //Open file from static location
    FILE *fp = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
    //FILE *fp = fopen( "/homes/acerna/CIS520/hw4/3way-mpi/test.txt", "r" );
    int ernsv = errno;
    if(fp == NULL || errno != 0)
	{
      printf("Error during fopen: %i", ernsv);
    }
    
    //
    char *line = (char*) malloc( 4096 ); // Ensure lines are no longer than 2000 characters
    ernsv = errno;
    if (line == NULL || errno != 0)
    {
        printf("Line allocation unsuccessful: %i", ernsv);
    }

    printf("myID = %d Starting Location = %d End Location = %d \n", myID, startLoc, endLoc); //Confirm threads assignments
    
    // Read in the lines from the data file
    for (int i = startLoc; i < endLoc; i++) //go through assigned lines
	{ 
        fseek(fp, i, SEEK_SET);  				//Go to file position assigned to this Core
        err = fscanf( fp, "%[^\n]\n", line); 	//Define a line when we reach newline
        if( err == EOF ) break; 				//Stop if we reach the end of file
        nchars = strlen(line); 					//# of chars in line
        results[i] = find_max(line, nchars); 	//Max ASCII line value to results array
    }

    fclose(fp);
    free(line);
}