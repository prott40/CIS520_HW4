#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define NUM_THREADS 4
#define BUFFER_LEN 2001

typedef struct {
    char *filename; // may need to be file descriptor
    long start_line;// where begins
    long end_line;// where it ends
    int *results; //
} Thread;

// Function to find the maximum ASCII value in a given line
int find_max_ascii(const char *line) {
    int max_value = 0;
    // hold max value while iterating over the string 
    for (int i = 0; line[i] != '\0'; i++) {
        unsigned char c = (unsigned char)line[i];
        if (c > max_value) {
            max_value = c;
        }
    }
    // return the max
    return max_value;
}

// Thread function to process lines
void *process_lines(void *arg) {
    //
    Thread *current = (Thread *)arg;
    FILE *file = fopen(current->filename, "r");
    if (file == NULL) {
        perror("Failed to open file in thread");
        pthread_exit(NULL);
    }
    
    // Skip to start_line
    char buffer[BUFFER_LEN]; // length that simon read for
    long line_count = 0;
    while (line_count < current->start_line && fgets(buffer, sizeof(buffer), file) != NULL) {
        line_count++;
    }
    
    // Process assigned lines
    long current_line = current->start_line;
    while (current_line < current->end_line && fgets(buffer, sizeof(buffer), file) != NULL) {
        // Remove newline if present
        size_t len = strnlen(buffer,BUFFER_LEN);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
        }
        
        current->results[current_line - current->start_line] = find_max_ascii(buffer);
        current_line++;
    }
    
    fclose(file);
    pthread_exit(NULL);
}

int main() {
    // save file name
    const char *filename = "/homes/dan/625/wiki_dump.txt";
    
    // Count total lines in file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
    
    long total_lines = 0;
    char buffer[BUFFER_LEN]; // Large enough for any line
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        total_lines++;
    }
    
    fclose(file);
    
    printf("Total lines in file: %ld\n", total_lines);
    
    // Allocate memory for all results
    int *all_results = (int *)malloc(total_lines * sizeof(int));
    if (all_results == NULL) {
        perror("Memory allocation failed");
        return 1;
    }
    
    // Create and start threads
    pthread_t threads[NUM_THREADS];
    Thread thread_args[NUM_THREADS];
    // calulate threads per line
    long lines_per_thread = total_lines / NUM_THREADS;
    long remainder = total_lines % NUM_THREADS;
    
    long start_line = 0;
    // set arguments for each thread
    for (int i = 0; i < NUM_THREADS; i++) {
        // adjust for remaineder
        long lines_for_this_thread = lines_per_thread + (i < remainder ? 1 : 0);
        // set arguments
        thread_args[i].filename = (char *)filename;
        thread_args[i].start_line = start_line;
        thread_args[i].end_line = start_line + lines_for_this_thread;
        thread_args[i].results = &all_results[start_line];
        //crete thread with all argumetns
        int rc = pthread_create(&threads[i], NULL, process_lines, (void *)&thread_args[i]);
        if (rc) {
            printf("ERROR: Return code from pthread_create() is %d\n", rc);
            free(all_results);
            return 1;
        }
        // increase start by lines for the thread
        start_line += lines_for_this_thread;
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    
    // Print results
    for (long i = 0; i < total_lines; i++) {
        printf("%ld: %d\n", i, all_results[i]);
    }
    
    free(all_results);
    return 0;
}