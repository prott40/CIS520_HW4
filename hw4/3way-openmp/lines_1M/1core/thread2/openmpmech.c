#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

#define NUM_THREADS 16
#define BUFFER_LEN 4096
#define CHUNK_SIZE 10000  // Number of lines to process per chunk

int find_max_ascii(const char *line) {
    int max_value = 0;
    for (int i = 0; line[i] != '\0'; i++) {
        if ((int)line[i] > max_value) {
            max_value = (int)line[i];
        }
    }
    return max_value;
}

int main() {
    const char *filename = "/homes/dan/625/wiki_dump.txt";
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        return 1;
    }

    char **lines = malloc(CHUNK_SIZE * sizeof(char *));
    if (!lines) {
        perror("Failed to allocate line buffer");
        fclose(file);
        return 1;
    }

    char buffer[BUFFER_LEN];
    size_t total_lines = 0;
    size_t chunk_index = 0;

    while (fgets(buffer, sizeof(buffer), file)) {
        size_t line_len = strnlen(buffer, BUFFER_LEN);
        char *full_line = malloc(BUFFER_LEN * 2);
        if (!full_line) {
            perror("Line allocation failed");
            break;
        }

        snprintf(full_line, BUFFER_LEN * 2, "%s", buffer);

        while (line_len > 0 && full_line[line_len - 1] != '\n' && !feof(file)) {
            if (fgets(buffer, sizeof(buffer), file)) {
                size_t buffer_len = strnlen(buffer, BUFFER_LEN);
                size_t new_size = line_len + buffer_len + 1;
                char *temp = realloc(full_line, new_size);
                if (!temp) {
                    perror("Realloc failed");
                    free(full_line);
                    break;
                }
                full_line = temp;
                snprintf(full_line + line_len, buffer_len + 1, "%s", buffer);
                line_len = strnlen(full_line, new_size);
            } else {
                break;
            }
        }

        if (line_len > 0 && full_line[line_len - 1] == '\n') {
            full_line[line_len - 1] = '\0';
        }

        lines[chunk_index++] = full_line;

        if (chunk_index == CHUNK_SIZE) {
            // Process chunk in parallel
            #pragma omp parallel for num_threads(NUM_THREADS)
            for (int i = 0; i < CHUNK_SIZE; i++) {
                int max_ascii = find_max_ascii(lines[i]);
                // For demonstration:
                // printf("%lu: %d\n", total_lines + i, max_ascii);
                (void)max_ascii; // Prevent unused warning
            }

            // Free processed lines
            for (int i = 0; i < CHUNK_SIZE; i++) {
                free(lines[i]);
            }

            total_lines += CHUNK_SIZE;
            chunk_index = 0;
        }
    }

    // Handle any leftover lines
    if (chunk_index > 0) {
        #pragma omp parallel for num_threads(NUM_THREADS)
        for (int i = 0; i < (int)chunk_index; i++) {
            int max_ascii = find_max_ascii(lines[i]);
            // printf("%lu: %d\n", total_lines + i, max_ascii);
            (void)max_ascii;
        }

        for (int i = 0; i < (int)chunk_index; i++) {
            free(lines[i]);
        }

        total_lines += chunk_index;
    }

    free(lines);
    fclose(file);
    printf("Processed %lu lines.\n", total_lines);
    return 0;
}
