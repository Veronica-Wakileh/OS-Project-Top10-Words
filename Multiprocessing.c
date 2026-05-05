#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define MAX_SIZE 1705210
#define UNIQUE_SIZE 255000
#define NUM_PROCESSES 2 // Number of child processes

struct array {
    char word[100];
    int frequency;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct array* create_shared_memory(size_t size) {
    struct array* shared_mem = mmap(NULL, size, PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem == MAP_FAILED) {
        perror("Shared memory allocation failed");
        exit(1);
    }
    return shared_mem;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int find_index(struct array* arr, int size, char word[]) {
    for (int i = 0; i < size; i++) {
        if (strcmp(arr[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Process a chunk of the file and calculate word frequencies
void process_chunk(struct array* shared_arr, struct array* local_arr, int start, int end, char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    // Move to the start of the chunk
    fseek(file, start, SEEK_SET);

    char word[100];
    int local_size = 0;

    // Process words within the assigned chunk
    while (ftell(file) < end && fscanf(file, "%s", word) != EOF) {
        int index = find_index(local_arr, local_size, word);
        if (index != -1) {
            local_arr[index].frequency++;
        } else {
            strcpy(local_arr[local_size].word, word);
            local_arr[local_size].frequency = 1;
            local_size++;
        }
    }
    fclose(file);

    // Merge local array into shared memory
    for (int i = 0; i < local_size; i++) {
        int index = find_index(shared_arr, UNIQUE_SIZE, local_arr[i].word);
        if (index != -1) {
            shared_arr[index].frequency += local_arr[i].frequency;
        } else {
            // Find the first empty slot in shared memory
            for (int j = 0; j < UNIQUE_SIZE; j++) {
                if (shared_arr[j].frequency == 0) {
                    strcpy(shared_arr[j].word, local_arr[i].word);
                    shared_arr[j].frequency = local_arr[i].frequency;
                    break;
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Display the top 10 most frequent words
void display_top_ten(struct array* shared_arr) {
    printf("Top 10 Words:\n");
    for (int i = 0; i < 10; i++) {
        int max_freq = 0;
        int max_index = 0;

        for (int j = 0; j < UNIQUE_SIZE; j++) {
            if (shared_arr[j].frequency > max_freq) {
                max_freq = shared_arr[j].frequency;
                max_index = j;
            }
        }

        if (max_freq > 0) {
            printf("%s %d\n", shared_arr[max_index].word, max_freq);
            shared_arr[max_index].frequency = 0; // Mark as processed
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function
int main() {

      struct timespec start, end;

      clock_gettime(CLOCK_MONOTONIC, &start);

      // Shared memory for storing results
      struct array* shared_arr = create_shared_memory(UNIQUE_SIZE * sizeof(struct array));
      memset(shared_arr, 0, UNIQUE_SIZE * sizeof(struct array));

      // File path and size calculation
      char* file_path = "test.txt";
      FILE* file = fopen(file_path, "r");
      if (file == NULL) {
          perror("Error opening file");
          exit(1);
      }
      fseek(file, 0, SEEK_END);
      int file_size = ftell(file);
      fclose(file);

      // Divide file into chunks
      int chunk_size = file_size / NUM_PROCESSES;
      pid_t pids[NUM_PROCESSES];

      // Create child processes
      for (int i = 0; i < NUM_PROCESSES; i++) {
          pids[i] = fork();
          if (pids[i] == 0) {
              // Child process
              struct array* local_arr = malloc(UNIQUE_SIZE * sizeof(struct array));
              memset(local_arr, 0, UNIQUE_SIZE * sizeof(struct array));

              int start = i * chunk_size;
              int end = (i == NUM_PROCESSES - 1) ? file_size : (i + 1) * chunk_size;
              process_chunk(shared_arr, local_arr, start, end, file_path);

              free(local_arr);
              exit(0);
          }
      }

      // Wait for all child processes to finish
      for (int i = 0; i < NUM_PROCESSES; i++) {
          wait(NULL);
      }


      display_top_ten(shared_arr);

      munmap(shared_arr, UNIQUE_SIZE * sizeof(struct array));


      clock_gettime(CLOCK_MONOTONIC, &end);
      double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
      printf("The total time including child processes is: %f minutes\n", elapsed/60);


    return 0;
}
