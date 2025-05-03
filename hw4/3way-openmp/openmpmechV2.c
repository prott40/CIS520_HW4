#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <omp.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

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
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Failed to get file size");
        close(fd);
        return 1;
    }

    size_t file_size = sb.st_size;
    char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to mmap file");
        close(fd);
        return 1;
    }
    //************************DIFFERENT SECTION START************************
    size_t *newline_positions = malloc(max_lines * sizeof(size_t));
    if (!newline_positions) {
        perror("Failed to allocate newline position array");
        munmap(data, file_size);
        close(fd);
        return 1;
    }

    // Step 1: Find newline positions in parallel
    size_t chunk_size = file_size / omp_get_max_threads();
    int total_lines = 0;

    #pragma omp parallel
    {
        int thread_lines = 0;
        size_t thread_start = omp_get_thread_num() * chunk_size;
        size_t thread_end = (omp_get_thread_num() == omp_get_max_threads() - 1) ? file_size : thread_start + chunk_size;

        for (size_t i = thread_start; i < thread_end && thread_lines < max_lines; ++i) {
            if (data[i] == '\n') {
                size_t index;
                #pragma omp atomic capture
                index = total_lines++;
                if (index < max_lines) newline_positions[index] = i;
            }
        }
    }

    if (total_lines == 0) {
        fprintf(stderr, "No lines found\n");
        munmap(data, file_size);
        close(fd);
        free(newline_positions);
        return 1;
    }

    // Step 2: Build line_ptrs and line_lens arrays
    const char **line_ptrs = malloc(total_lines * sizeof(char *));
    size_t *line_lens = malloc(total_lines * sizeof(size_t));
    int *results = malloc(total_lines * sizeof(int));
    if (!line_ptrs || !line_lens || !results) {
        perror("Memory allocation failed");
        munmap(data, file_size);
        close(fd);
        free(newline_positions);
        free(line_ptrs); free(line_lens); free(results);
        return 1;
    }

    #pragma omp parallel for
    for (int i = 0; i < total_lines; i++) {
        size_t start = (i == 0) ? 0 : newline_positions[i - 1] + 1;
        size_t end = newline_positions[i];
        line_ptrs[i] = &data[start];
        line_lens[i] = end - start;
    }

    free(newline_positions);
    printf("Total lines parsed: %d\n", total_lines);
    //************************DIFFERENT SECTION END************************
    // Time and run the parallel computation
    FILE *log = fopen("perf_stat_summary.txt", "a");
    if (!log) {
        perror("Failed to open perf_stat_summary.txt");
        return 1;
    }
    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (int i = 0; i < total_lines; i++) {
        results[i] = find_max_ascii(line_ptrs[i], line_lens[i]);
    }

    double end_time = omp_get_wtime();
    fprintf(log, "Parallel max-char computation time: %f seconds\n", end_time - start_time);
    fclose(log);

    for (int i = 0; i < total_lines; i++) {
        printf("%d: %d\n", i, results[i]);
    }

    munmap(data, file_size);
    close(fd);
    free(line_ptrs);
    free(line_lens);
    free(results);

    return 0;
}
