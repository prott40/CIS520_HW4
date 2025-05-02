/*
 * mpi.c
 * 
 * Reads a file and returns the highest associated ASCII value found in each line
 *
 * Ailthon Cerna 4/14/2025
 */

#define _POSIX_C_SOURCE 200809L
#define MAX_LINES 1000000
#define BUFFER_LEN 4096

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"
#include <errno.h>
#include <time.h>

/* Function Prototypes */
int find_max(char* line, int count);
void process_lines(int start, int stop, int rank);

int main(int argc, char *argv[])
{
	/* Clock Times for Process Calculations */
	
	int rc, numTasks, rank;
	int root_rank = 0;
	
	/* Init MPI */
	rc = MPI_Init(&argc,&argv);
    if (rc != MPI_SUCCESS) 
	{
      printf ("Error starting MPI program. Terminating.\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      return 0;
    }
    
    rc = MPI_Comm_size(MPI_COMM_WORLD,&numTasks);
	if (rc != MPI_SUCCESS) 
	{
      printf ("Error Getting Number of Process. Terminating.\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      return 0;
    }
	
    rc = MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	if (rc != MPI_SUCCESS) 
	{
      printf ("Error Getting Process Rank. Terminating.\n");
      MPI_Abort(MPI_COMM_WORLD, rc);
      return 0;
    }
	
	//For N Processes
	/* Initialize Memory */
	int* displacements = malloc(sizeof(int)*numTasks); //Four Slots for Four Displacement Indicies
	memset(displacements, 0, numTasks*sizeof(int));
	for(int i = 0; i < numTasks; i++)
	{
		displacements[i] = i*MAX_LINES/numTasks; 	
		//Each location at i contains the number of assumed lines read by each process
	}
	if(displacements == NULL)
	{
		printf("Memory allocation for array 'displacements' was unsuccessful");
		return 0;
	}
	
	int* counts = malloc(sizeof(int)*numTasks); //Number of Receives from Gather from each Process
	memset(counts, 0, numTasks*sizeof(int));
	for(int i = 0; i < numTasks; i++)
	{
		counts[i] = MAX_LINES/numTasks; //Each process (i) will send MAX_LINES/numTasks to root 
	}
	if(counts == NULL)
	{
		printf("Memory allocation for array 'counts' was unsuccessful");
		return 0;
	}
	
	/* Read the file into the the list of entries */
	printf("Rank: %i\n", rank);
	
	
	//Open file from static location
	FILE *fp = fopen( "/homes/dan/625/wiki_dump.txt", "r" );
	//FILE *fp = fopen( "/homes/acerna/CIS520/hw4/3way-mpi/test.txt", "r" );
	int ernsv = errno;
	if(fp == NULL || errno != 0)
	{
	  printf("Error during fopen: %i", ernsv);
	}
	char buffer[BUFFER_LEN];
	int line_count = 0;
	int startLoc = rank * (MAX_LINES  / numTasks);
	int endLoc = startLoc + (MAX_LINES  / numTasks);
	
	int* rankResults = malloc(sizeof(int) * (endLoc - startLoc)); //allocate memory for an int for every line in process
	memset(rankResults, 0, (endLoc - startLoc) * sizeof(int));
	if(rankResults == NULL)
	{
		printf("Memory allocation for array 'Rank Results' was unsuccessful");
		return 0;
	}
	
	// Skip to start_line for this process
	while (line_count < startLoc && fgets(buffer, sizeof(buffer), fp) != NULL) 
	{
		// For lines that exceed buffer size, read until end of line
		while (strnlen(buffer, BUFFER_LEN) > 0 && 
			   buffer[strnlen(buffer, BUFFER_LEN) - 1] != '\n' && 
			   !feof(fp)) 
		{
			if (fgets(buffer, sizeof(buffer), fp) == NULL) break;
		}
		line_count++;
	}
	
	// Reset counter for processing assigned lines
	line_count = 0;
	
	// Process our assigned lines
	while (line_count < (endLoc - startLoc) && fgets(buffer, sizeof(buffer), fp) != NULL) 
	{
		// Allocate space for the full line with extra room
		char *full_line = malloc(BUFFER_LEN * 2);
		if (!full_line) 
		{
			perror("Memory allocation failed");
			fclose(fp);
		}
		
		// Use snprintf to copy buffer to full_line safely
		int copy_result = snprintf(full_line, BUFFER_LEN, "%s", buffer);		
		if (copy_result < 0 || copy_result >= BUFFER_LEN) {
			perror("String copy failed or truncated");
			free(full_line);
			fclose(fp);
			
		}		
		
		size_t line_len = strnlen(full_line, BUFFER_LEN);
		
		// When line didn't end with newline and we're not at EOF, read the rest of the line
		while (line_len > 0 && full_line[line_len - 1] != '\n' && !feof(fp)) 
		{
			
			if (fgets(buffer, sizeof(buffer), fp) != NULL) 
			{
				size_t buffer_len = strnlen(buffer, BUFFER_LEN);
				
				// Reallocate memory to accommodate more text
				size_t new_size = line_len + buffer_len + 1;
				char *temp = realloc(full_line, new_size);
				
				if (!temp) {
					perror("Memory reallocation failed");
					free(full_line);
					fclose(fp);
				}
				
				full_line = temp;
				
				// Safely append buffer to full_line
				int append_result = snprintf(full_line + line_len, buffer_len + 1, "%s", buffer);
				if (append_result < 0 || append_result >= (int)(buffer_len + 1)) 
				{
					perror("String append failed or truncated");
					free(full_line);
					fclose(fp);
				}
				
				line_len = strnlen(full_line, new_size);
			} 
			else 
			{
				break;
			}
		}
		
		// Remove newline if present
		if (line_len > 0 && full_line[line_len - 1] == '\n') {
			full_line[line_len - 1] = '\0';
		}
		
		// Find the maximum ASCII value for this line
		//printf("%s\n", full_line); //Debugging
		rankResults[line_count] = find_max(full_line, line_len);
		free(full_line);
		line_count++;
		
		
	}
	//Do the MPI Message Passing from Current Rank to Root Rank, tag 2 for line count, 1 for parse results
	//MPI_Send(&line_count, 1, MPI_LONG, 0, 2, MPI_COMM_WORLD);
	//MPI_Send(&rankResults, line_count, MPI_INT, 0, 1, MPI_COMM_WORLD);
	if(rank != 0)
	{
		MPI_Gatherv(rankResults, counts[rank], MPI_INT, NULL, NULL, NULL, MPI_INT, root_rank, MPI_COMM_WORLD);
	}
	printf("File Parse on Rank %d Finished.\n", rank);
	
	//MPI_Barrier(MPI_COMM_WORLD); //Sync Process Ends
	fclose(fp);
	
    //not needed tend = clock(); //end execution time
    
	if ( rank == 0 ) 
	{
		int* fileResults = malloc(sizeof(int) * MAX_LINES); //allocate memory for an int for every line
		memset(fileResults, 0, MAX_LINES * sizeof(int));
		if(fileResults == NULL)
		{
			printf("Memory allocation for array 'FileResults' was unsuccessful");
			return 0;
		}	
		//print rank 0 results
		MPI_Gatherv(rankResults, counts[rank], MPI_INT, fileResults, counts, displacements, MPI_INT, root_rank, MPI_COMM_WORLD);
		
		for(int i = 0; i < numTasks; i++) //Receive results from each rank/process
		{
			
			printf("Total Cluster Size = %d Current Rank = %d Results from Rank %d\n", numTasks, rank, i);
			//MPI_Recv(&line_count, 1, MPI_INT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE); //Get the correct line count from 
			//MPI_Recv(&fileResults, line_count, MPI_INT, i, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			for (int j = 0; j < counts[rank]; j++) //For the assigned number of lines per process
			{ 
				int loc = i*counts[i]+j;
				printf("%d: %d\n", loc, fileResults[loc]); //print da results fo each
			}	
		}
		
		free(fileResults);
	
    }
    	
	//MPI_Barrier(MPI_COMM_WORLD); Idk why this throws an error
	
	free(rankResults);
	free(displacements);
	free(counts);
	
	printf("Memory Freed on process %d. Exiting.\n", rank);

    //exit program and cleanup
	//MPI_Barrier(MPI_COMM_WORLD);
	
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

