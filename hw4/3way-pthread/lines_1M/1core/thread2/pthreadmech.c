#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define NUM_THREADS 2
#define BUFFER_LEN 4096  // More reasonable buffer size
#define MAX_LINE_READ 100000 
typedef struct {
    char *filename;
    long start_line;
    long end_line;
    int *results;
} Thread;

// Function to find the maximum ASCII value in a given line
int find_max_ascii(const char *line) {
    int max_value = 0;
    // Loop over the line before the end of string char
    for (int i = 0; line[i] != '\0'; i++) {
        // Check for new max
        if ((int)line[i] > max_value) {
            max_value = (int)line[i];
        }
    }
    // Return val
    return max_value;
}

// Thread function to process lines
void *process_lines(void *arg) {
    // Convert to the threads struct
    Thread *current = (Thread *)arg;
    // Open file from thread
    FILE *file = fopen(current->filename, "r");
    // Check for failed open
    if (file == NULL) {
        perror("Failed to open file in thread");
        pthread_exit(NULL);
    }
    
    char buffer[BUFFER_LEN];
    long line_count = 0;
    
    // Skip to start_line for this thread
    while (line_count < current->start_line && fgets(buffer, sizeof(buffer), file) != NULL) {
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
    
    // Process our assigned lines
    while (line_count < (current->end_line - current->start_line) && 
           fgets(buffer, sizeof(buffer), file) != NULL) {
        
        // Allocate space for the full line with extra room
        char *full_line = malloc(BUFFER_LEN * 2);
        if (!full_line) {
            perror("Memory allocation failed");
            fclose(file);
            pthread_exit(NULL);
        }
        
        // Use snprintf to copy buffer to full_line safely
        int copy_result = snprintf(full_line, BUFFER_LEN, "%s", buffer);
        if (copy_result < 0 || copy_result >= BUFFER_LEN) {
            perror("String copy failed or truncated");
            free(full_line);
            fclose(file);
            pthread_exit(NULL);
        }
        
        size_t line_len = strnlen(full_line, BUFFER_LEN);
        
        // When line didn't end with newline and we're not at EOF, read the rest of the line
        while (line_len > 0 && 
               full_line[line_len - 1] != '\n' && 
               !feof(file)) {
            
            if (fgets(buffer, sizeof(buffer), file) != NULL) {
                size_t buffer_len = strnlen(buffer, BUFFER_LEN);
                
                // Reallocate memory to accommodate more text
                size_t new_size = line_len + buffer_len + 1;
                char *temp = realloc(full_line, new_size);
                
                if (!temp) {
                    perror("Memory reallocation failed");
                    free(full_line);
                    fclose(file);
                    pthread_exit(NULL);
                }
                
                full_line = temp;
                
                // Safely append buffer to full_line
                int append_result = snprintf(full_line + line_len, buffer_len + 1, "%s", buffer);
                if (append_result < 0 || append_result >= (int)(buffer_len + 1)) {
                    perror("String append failed or truncated");
                    free(full_line);
                    fclose(file);
                    pthread_exit(NULL);
                }
                
                line_len = strnlen(full_line, new_size);
            } else {
                break;
            }
        }
        
        // Remove newline if present
        if (line_len > 0 && full_line[line_len - 1] == '\n') {
            full_line[line_len - 1] = '\0';
        }
        
        // Find the maximum ASCII value for this line
        current->results[line_count] = find_max_ascii(full_line);
        free(full_line);
        line_count++;
    }
    
    // Close the file and exit
    fclose(file);
    pthread_exit(NULL);
}

int main() {
    // Where the text file is
    const char *filename = "/homes/dan/625/wiki_dump.txt";
    /*
    // Open the file
    FILE *file = fopen(filename, "r");
    // Check for failure
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }
    */
    long total_lines = MAX_LINE_READ;
    char buffer[BUFFER_LEN];
    /*
    // Count number of lines, handling long lines properly
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        // For lines that exceed buffer size, read until end of line
        while (strnlen(buffer, BUFFER_LEN) > 0 && 
               buffer[strnlen(buffer, BUFFER_LEN) - 1] != '\n' && 
               !feof(file)) {
            if (fgets(buffer, sizeof(buffer), file) == NULL) break;
        }
        total_lines++;
    }
    
    // Close file 
    fclose(file);
    */
    // Tell how long
    //printf("Total lines in file: %ld\n", total_lines);
    
    // Allocate memory for all results
    int *all_results = (int *)malloc(total_lines * sizeof(int));
    // Check for failure
    if (all_results == NULL) {
        perror("Memory allocation failed");
        return 1;
    }
    
    // Initialize all values to avoid garbage values
    memset(all_results, 0, total_lines * sizeof(int));
    
    // Create and start threads
    pthread_t threads[NUM_THREADS];
    Thread thread_args[NUM_THREADS];
    long lines_per_thread = total_lines / NUM_THREADS;
    long remainder = total_lines % NUM_THREADS;
    
    long start_line = 0;
    
    // Create the threads and initialize the values
    for (int i = 0; i < NUM_THREADS; i++) {
        long lines_for_this_thread = lines_per_thread + (i < remainder ? 1 : 0);
        thread_args[i].filename = (char *)filename;
        thread_args[i].start_line = start_line;
        thread_args[i].end_line = start_line + lines_for_this_thread;
        thread_args[i].results = &all_results[start_line];
        
        // Create the thread and give it the function to process
        int rc = pthread_create(&threads[i], NULL, process_lines, (void *)&thread_args[i]);
        // Check for failure
        if (rc) {
            printf("ERROR: Return code from pthread_create() is %d\n", rc);
            free(all_results);
            return 1;
        }
        
        // Increment the start for each function
        start_line += lines_for_this_thread;
    }
    
    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    /*
    // Print all results
    for (long i = 0; i < total_lines; i++) {
        printf("%ld: %d\n", i, all_results[i]);
    }
    */
    // Free all memory
    free(all_results);
    return 0;
}