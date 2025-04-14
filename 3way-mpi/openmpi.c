/*
 * mpimech.c
 * 
 * Reads a file and returns the highest associated ASCII value found in each line
 *
 * Ailthon Cerna 4/14/2025
 */

#include <stdio.h>
#include "mpi.h"
#include <string.h>

#define BUFFER_LEN 4096
#define START 1
#define END 2
#define LINES 3


// Function to find the maximum ASCII value in a given line
int find_max_ascii(const char *line) {
    int max_value = 0;
    // Loop over the line before the end of string char
    for (int i = 0; line[i] != '\0'; i++) {
        // Convert from string to char
        unsigned char c = (unsigned char)line[i];
        // Check for new max
        if (c > max_value) {
            max_value = c;
        }
    }
    // Return val
    return max_value;
}

void process_lines(long start_line, long end_line)
{
	
	const char *filename = "3way-mpi/test.txt" //"/homes/dan/625/wiki_dump.txt"; 
	FILE *file = fopen(current->filename, "r");
	// Check for failed open
	if (file == NULL)
	{
    perror("Failed to open file");
    pthread_exit(NULL);
	}
  
    long line_count = 0;
    char buffer[BUFFER_LEN];
	
	// Skip to start_line for this rank
    while (line_count < start_line && fgets(buffer, sizeof(buffer), file) != NULL) 
	{
        // For lines that exceed buffer size, read until end of line
        while (strnlen(buffer, BUFFER_LEN) > 0 && 
               buffer[strnlen(buffer, BUFFER_LEN) - 1] != '\n' && 
               !feof(file)) {
            if (fgets(buffer, sizeof(buffer), file) == NULL) break;
        }
        line_count++;
    }
	
	// Reset counter for processing assigned lines
    line_count = 0;
	
	while (line_count < (current->end_line - start_line) && 
           fgets(buffer, sizeof(buffer), file) != NULL) 
	{
		
	}
	
}

int main(int argc, char *argv[])
{
	const char *filename = "3way-mpi/test.txt" //"/homes/dan/625/wiki_dump.txt"; 
	FILE *file = fopen(filename, "r");
	// Check for failed open
	if (file == NULL)
	{
		perror("Failed to open file");
	}
  
    long total_lines = 0;
    char buffer[BUFFER_LEN];
    
    // Count number of lines, handling long lines properly
    while (fgets(buffer, sizeof(buffer), file) != NULL) 
	{
        // For lines that exceed buffer size, read until end of line
        while (strnlen(buffer, BUFFER_LEN) > 0 && 
               buffer[strnlen(buffer, BUFFER_LEN) - 1] != '\n' && 
               !feof(file)) 
		{
            if (fgets(buffer, sizeof(buffer), file) == NULL) break;
        }
        total_lines++;
    }
    
    // Close file 
    fclose(file);
    // Tell how long
    printf("Total lines in file: %ld\n", total_lines);
	
	// Allocate memory for all results
    int *all_results = (int *)malloc(total_lines * sizeof(int));
    // Check for failure
    if (all_results == NULL) {
        perror("Memory allocation failed");
        return 1;
    }
	
	memset(all_results, 0, total_lines * sizeof(int));
  
	/* Start up MPI */
	MPI_Status status;
	int num, rank, size, tag, next, from;
	long start_line = 0;
	long end_line = 0;
	long lines_per_process = total_lines / size; //Partition work amongst all cores
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
 
	/* Arbitrarily choose 201 to be our tag.  Calculate the */
	/* rank of the next process in the ring.  Use the modulus */
	/* operator so that the last process "wraps around" to rank */
	/* zero. */

	/* Begin Parallel Sequencing */
	//Take Line read-in and send to process i 
	
	if(rank == 0) 
	{
		for(i = 1; i < size; i++) //For # of processes, 
		{
			start_line = end_line;	//Start at last process end
			end_line = end_line + lines_per_process;	//Denote this process end location
			MPI_Send(&start_line, 1, MPI_LONG, i, START, MPI_COMM_WORLD);	//Send Start Location to Rank i
			MPI_Send(&end_line, 1, MPI_LONG, i, END, MPI_COMM_WORLD);  		//Send End Location to Rank i
			//MPI_Send(&lines_per_process, 1, MPI_LONG, i, LINES, MPI_COMM_WORLD); 	//Send lines per process to Rank i
		}
		
	}
	else
	{
		//Do worker processes
		MPI_Recv(&start_line, 1, MPI_LONG, rank, START, MPI_COMM_WORLD);
		MPI_Recv(&end_line, 1, MPI_LONG, rank, END, MPI_COMM_WORLD);
	}
	//Hold here for process sync
	MPI_Barrier(MPI_COMM_WORLD);
	
  
	if(rank == 0) //Print Results only on Master Process
	{
		for (long i = 0; i < total_lines; i++) 
		{
			printf("%ld: %d\n", i, all_results[i]);
		}
    }
    // Free all memory
    free(all_results);

	/* Quit */

	MPI_Finalize();
	return 0;
}
