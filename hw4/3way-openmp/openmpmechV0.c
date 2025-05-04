#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <omp.h>

int find_max_ascii(const char *line, size_t len) {
    int max_value = 0;
    for (size_t i = 0; i < len; i++) {
        if ((int)line[i] > max_value) {
            max_value = (int)line[i];
        }
    }
    return max_value;
}

int main(int argc, char *argv[]) {
    size_t max_lines = 1000000; // default value

    if (argc >= 2) {
        max_lines = strtoull(argv[1], NULL, 10);
    }

    const char *filename = "/homes/dan/625/wiki_dump.txt";
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return 1;
    }

    // Allocate memory for line pointers and lengths
    char **line_ptrs = malloc(max_lines * sizeof(char *));
    size_t *line_lens = malloc(max_lines * sizeof(size_t));
    int *results = malloc(max_lines * sizeof(int));

    if (!line_ptrs || !line_lens || !results) {
        perror("Memory allocation failed");
        fclose(fp);
        free(line_ptrs);
        free(line_lens);
        free(results);
        return 1;
    }

    // Read up to max_lines from the file
    size_t line_count = 0;
    size_t bufsize = 4096;
    char *buffer = malloc(bufsize);

    while (line_count < max_lines && getline(&buffer, &bufsize, fp) != -1) {
        size_t len = strlen(buffer);

        // Strip newline if present
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        // Copy the line into a separate buffer
        line_ptrs[line_count] = strdup(buffer);
        line_lens[line_count] = len;
        line_count++;
    }

    free(buffer);
    fclose(fp);

    if (line_count == 0) {
        fprintf(stderr, "No lines read\n");
        free(line_ptrs);
        free(line_lens);
        free(results);
        return 1;
    }

    printf("Total lines read: %zu\n", line_count);

    // Time and run the parallel computation
    FILE *log = fopen("custom_log.txt", "w");
    if (!log) {
        perror("Failed to open custom_log.txt");
        return 1;
    }
    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < (int)line_count; i++) {
        results[i] = find_max_ascii(line_ptrs[i], line_lens[i]);
    }

    double end_time = omp_get_wtime();
    fprintf(log, "Parallel max-char computation time: %f seconds\n", end_time - start_time);
    fclose(log);

    for (size_t i = 0; i < line_count; i++) {
        printf("%zu: %d\n", i, results[i]);
        free(line_ptrs[i]);
    }

    free(line_ptrs);
    free(line_lens);
    free(results);

    return 0;
}
