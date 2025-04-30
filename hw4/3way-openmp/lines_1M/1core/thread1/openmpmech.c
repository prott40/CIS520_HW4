#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <omp.h>
#include <stdint.h>

#define NUM_THREADS 16
#define BUFFER_LEN 4096
#define MAX_CHUNKS 100
#define LINES_PER_CHUNK 10000  // 10000 lines per chunk

void cleanup(char **lines, size_t total_lines, int *results) {
    if (lines) {
        for (size_t i = 0; i < total_lines; i++) {
            free(lines[i]);
        }
        free(lines);
    }
    free(results);
}

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
    if (file == NULL) {
        perror("Failed to open file");
        return 1;
    }

    char buffer[BUFFER_LEN];
    size_t chunk_counter = 0;

    while (!feof(file) && chunk_counter < MAX_CHUNKS) {
        size_t total_lines = 0;

        char **lines = malloc(LINES_PER_CHUNK * sizeof(char *));
        if (!lines) {
            perror("Memory allocation for lines failed");
            fclose(file);
            return 1;
        }

        int *results = NULL;

        // Read up to LINES_PER_CHUNK lines
        while (fgets(buffer, sizeof(buffer), file) != NULL && total_lines < LINES_PER_CHUNK) {
            size_t line_len = strnlen(buffer, BUFFER_LEN);
            char *full_line = malloc(BUFFER_LEN * 2);
            if (!full_line) {
                perror("Memory allocation for line failed");
                fclose(file);
                cleanup(lines, total_lines, results);
                return 1;
            }

            snprintf(full_line, BUFFER_LEN * 2, "%s", buffer);

            while (line_len > 0 && full_line[line_len - 1] != '\n' && !feof(file)) {
                if (fgets(buffer, sizeof(buffer), file) != NULL) {
                    size_t buffer_len = strnlen(buffer, BUFFER_LEN);
                    size_t new_size = line_len + buffer_len + 1;
                    char *temp = realloc(full_line, new_size);
                    if (!temp) {
                        perror("Reallocating line buffer failed");
                        free(full_line);
                        fclose(file);
                        cleanup(lines, total_lines, results);
                        return 1;
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

            lines[total_lines++] = full_line;
        }

        if (total_lines == 0) {
            // No more lines to process
            free(lines);
            break;
        }

        printf("Processing chunk %zu with %zu lines\n", chunk_counter + 1, total_lines);

        // Allocate results
        results = malloc(total_lines * sizeof(int));
        if (!results) {
            perror("Memory allocation for results failed");
            fclose(file);
            cleanup(lines, total_lines, NULL);
            return 1;
        }

        // Process lines in parallel
        #pragma omp parallel for num_threads(NUM_THREADS)
        for (long i = 0; i < total_lines; i++) {
            results[i] = find_max_ascii(lines[i]);
        }

        // Clean up memory for this chunk
        cleanup(lines, total_lines, results);

        chunk_counter++;
    }

    fclose(file);
    return 0;
}
