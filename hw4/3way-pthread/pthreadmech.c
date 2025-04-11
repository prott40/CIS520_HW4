#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_THREADS 4  // Number of threads to use for parallelism
#define LINES_PER_BATCH 1000  // Number of lines to read per batch
#define STRING_SIZE 16  // Maximum length of each line (assuming no line exceeds this length)
#define MAX_FILE_PATH "/~dan/625/wiki_dump.txt"  // Path to the input file (adjust this path)

pthread_mutex_t mutex;  // Mutex for synchronization when printing results

// Global array to store the maximum ASCII values for each line processed by threads
int max_ascii_values[NUM_THREADS][LINES_PER_BATCH];  // Each thread has its own batch of results

// Function to find the maximum ASCII value in a given line
int find_max_ascii(char *line) {
    int max_value = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        int ascii_value = (int) line[i];
        if (ascii_value > max_value) {
            max_value = ascii_value;
        }
    }
    return max_value;
}

// Function to process a batch of lines from the file (Each thread processes its batch independently)
void *process_batch(void *arg) {
    FILE *file = (FILE *)arg;
    char line[STRING_SIZE];  // Buffer for reading each line
    int thread_id = *((int *)arg);  // Thread ID is passed as an argument
    int start_line = thread_id * LINES_PER_BATCH;  // Start line for this thread

    fseek(file, start_line * STRING_SIZE, SEEK_SET);  // Move file pointer to the correct position

    for (int i = 0; i < LINES_PER_BATCH; i++) {
        if (fgets(line, STRING_SIZE, file) == NULL) {
            break;  // End of file reached
        }
        
        // Find the maximum ASCII value for the line and store it in the thread's result array
        max_ascii_values[thread_id][i] = find_max_ascii(line);
    }

    pthread_exit(NULL);
}

// Function to print the results of each line
void print_results(int total_lines_processed) {
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < LINES_PER_BATCH; j++) {
            if (i * LINES_PER_BATCH + j >= total_lines_processed) {
                break;
            }
            printf("%d: %d\n", i * LINES_PER_BATCH + j, max_ascii_values[i][j]);
        }
    }
}

// Main function to read the file and spawn threads to process batches
int main() {
    FILE *file = fopen(MAX_FILE_PATH, "r");
    if (file == NULL) {
        perror("Failed to open the file");
        exit(1);
    }

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];
    int total_lines = 0;
    int rc;

    // Initialize the mutex
    pthread_mutex_init(&mutex, NULL);

    // Read the file in batches and process concurrently
    while (!feof(file)) {
        for (int i = 0; i < NUM_THREADS; i++) {
            // Each thread processes a specific batch of lines
            thread_ids[i] = i;
            rc = pthread_create(&threads[i], NULL, process_batch, (void *)&thread_ids[i]);
            if (rc) {
                printf("ERROR: Return code from pthread_create() is %d\n", rc);
                exit(1);
            }
            total_lines += LINES_PER_BATCH;
        }

        // Wait for threads to finish processing the current batch
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        // Print the results for the current batch
        print_results(total_lines);
    }

    // Destroy the mutex after use
    pthread_mutex_destroy(&mutex);

    fclose(file);  // Close the file

    printf("Main: Program completed. Exiting.\n");
    return 0;
}
