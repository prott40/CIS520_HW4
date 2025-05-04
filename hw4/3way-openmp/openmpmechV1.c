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
    //create a file descriptor for the text file
    const char *filename = "/homes/dan/625/wiki_dump.txt";
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open file");
        return 1;
    }

    //get size of the file
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("Failed to get file size");
        close(fd);
        return 1;
    }

    // memory map the file using previous size
    // can I just memory map the max lines?
    size_t file_size = sb.st_size;
    char *data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("Failed to mmap file");
        close(fd);
        return 1;
    }


    //allocate the array which hold the ptrs to each line
    const char **line_ptrs = malloc(max_lines * sizeof(char *));
    //allocate the array which stores the length of each line
    size_t *line_lens = malloc(max_lines * sizeof(size_t));
    //allocate the array that holds the greatest char in each line
    int *results = malloc(max_lines * sizeof(int));
    if (!line_ptrs || !line_lens || !results) {
        perror("Memory allocation failed");
        munmap(data, file_size);
        close(fd);
        free(line_ptrs);
        free(line_lens);
        free(results);
        return 1;
    }

    //Go through the file and make a pointer to each line along with its line length
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
    FILE *log = fopen("custom_log.txt", "w");
    if (!log) {
        perror("Failed to open custom_log.txt for appending");
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

    for (size_t i = 0; i < line_count; i++) {
        printf("%zu: %d\n", i, results[i]);
    }

    munmap(data, file_size);
    close(fd);
    free(line_ptrs);
    free(line_lens);
    free(results);

    return 0;
}
