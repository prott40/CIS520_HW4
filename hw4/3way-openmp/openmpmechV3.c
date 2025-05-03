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

    const char **line_ptrs = malloc(max_lines * sizeof(char *));
    size_t *line_lens = malloc(max_lines * sizeof(size_t));
    int *results = malloc(max_lines * sizeof(int));
    if (!line_ptrs || !line_lens || !results) {
        perror("Memory allocation failed");
        munmap(data, file_size);
        close(fd);
        return 1;
    }

    // Extract lines
    size_t line_start = 0, line_end = 0, line_count = 0;
    while (line_end < file_size && line_count < max_lines) {
        if (data[line_end] == '\n') {
            size_t len = line_end - line_start;
            line_ptrs[line_count] = &data[line_start];
            line_lens[line_count] = len;
            line_count++;
            line_start = line_end + 1;
        }
        line_end++;
    }

    printf("Total lines read: %zu\n", line_count);

    //file to log the loop time
    FILE *log = fopen("perf_stat_summary.txt", "a");  // Open in append mode
    if (!log) {
        perror("Failed to open perf_stat_summary.txt for appending");
        return 1;
    }
    double start_time = omp_get_wtime();

    #pragma omp parallel for
    for (long i = 0; i < (long)line_count; i++) {
        results[i] = find_max_ascii(line_ptrs[i], line_lens[i]);
    }

    double end_time = omp_get_wtime();
    fprintf(log, "Parallel max-char computation time: %f seconds\n", end_time - start_time);
    fclose(log);

    // Parallel output using thread-local buffers
    int num_threads = omp_get_max_threads();
    char **output_buffers = malloc(num_threads * sizeof(char *));
    size_t *buffer_sizes = calloc(num_threads, sizeof(size_t));
    size_t max_line_len = 32; // e.g., "1234567: 255\n"

    for (int t = 0; t < num_threads; t++) {
        output_buffers[t] = malloc(max_line_len * (line_count / num_threads + 1));
    }

    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        char *buf = output_buffers[tid];
        size_t offset = 0;

        #pragma omp for schedule(static)
        for (long i = 0; i < (long)line_count; i++) {
            offset += sprintf(&buf[offset], "%ld: %d\n", i, results[i]);
        }
        buffer_sizes[tid] = offset;
    }

    // Write buffers in order
    FILE *out = fopen("output.txt", "w");
    if (!out) {
        perror("Failed to open output.txt");
        return 1;
    }
    for (int t = 0; t < num_threads; t++) {
        fwrite(output_buffers[t], 1, buffer_sizes[t], out);
        free(output_buffers[t]);
    }
    fclose(out);
    free(output_buffers);
    free(buffer_sizes);

    munmap(data, file_size);
    close(fd);
    free(line_ptrs);
    free(line_lens);
    free(results);

    return 0;
}
