#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <omp.h>

#define NUM_THREADS 16
#define BUFFER_LEN 4096

// Function to find the maximum ASCII value in a given line
int find_max_ascii(const char *line)
{
    int max_value = 0;
    // Loop over the line before the end of string char
    for (int i = 0; line[i] != '\0'; i++)
    {
        unsigned char c = (unsigned char)line[i];
        if (c > max_value)
        {
            max_value = c;
        }
    }
    return max_value;
}

int main()
{

    const char *filename = "/homes/dan/625/wiki_dump.txt";
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Failed to open file");
        return 1;
    }

    // Read all lines into memory
    char **lines = NULL;
    size_t lines_capacity = 1024;
    size_t total_lines = 0;
    char buffer[BUFFER_LEN];

    lines = malloc(lines_capacity * sizeof(char *));
    if (!lines)
    {
        perror("Initial memory allocation failed");
        fclose(file);
        return 1;
    }

    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        // Handle lines longer than buffer
        size_t line_len = strnlen(buffer, BUFFER_LEN);
        char *full_line = malloc(BUFFER_LEN * 2);
        if (!full_line)
        {
            perror("Memory allocation failed");
            fclose(file);
            return 1;
        }

        snprintf(full_line, BUFFER_LEN * 2, "%s", buffer);

        while (line_len > 0 && full_line[line_len - 1] != '\n' && !feof(file))
        {
            if (fgets(buffer, sizeof(buffer), file) != NULL)
            {
                size_t buffer_len = strnlen(buffer, BUFFER_LEN);
                size_t new_size = line_len + buffer_len + 1;
                char *temp = realloc(full_line, new_size);
                if (!temp)
                {
                    perror("Memory reallocation failed");
                    free(full_line);
                    fclose(file);
                    return 1;
                }
                full_line = temp;
                snprintf(full_line + line_len, buffer_len + 1, "%s", buffer);
                line_len = strnlen(full_line, new_size);
            }
            else
            {
                break;
            }
        }

        if (line_len > 0 && full_line[line_len - 1] == '\n')
        {
            full_line[line_len - 1] = '\0';
        }

        if (total_lines >= lines_capacity)
        {
            lines_capacity *= 2;
            char **temp_lines = realloc(lines, lines_capacity * sizeof(char *));
            if (!temp_lines)
            {
                perror("Reallocating lines buffer failed");
                fclose(file);
                return 1;
            }
            lines = temp_lines;
        }

        lines[total_lines++] = full_line;
    }

    fclose(file);
    printf("Total lines in file: %ld\n", total_lines);

    // Prepare results array
    int *results = malloc(total_lines * sizeof(int));
    if (!results)
    {
        perror("Memory allocation failed for results");
        return 1;
    }

    // Parallel processing with OpenMP
    #pragma omp parallel for num_threads(NUM_THREADS)
    for (long i = 0; i < total_lines; i++)
    {
        results[i] = find_max_ascii(lines[i]);
    }

    // Print results
    for (long i = 0; i < total_lines; i++)
    {
        printf("%ld: %d\n", i, results[i]);
        free(lines[i]);  // Clean up each line
    }

    free(lines);
    free(results);

    return 0;
}