#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_SIZE 1705210
#define UNIQUE_SIZE 255000
#define NUM_THREADS 2

struct array {
    char word[100];
    int frequency;
};

pthread_mutex_t mutex;
struct array *globalUniqueArray;
int globalUniqueCount = 0;

struct thread_data {
    FILE *file;
    long start;
    long end;
};

struct array *makeArray(int size) {
    struct array *arr = (struct array *)malloc(size * sizeof(struct array));
    if (arr == NULL) {
        printf("Out of memory!\n");
        exit(1);
    }
    return arr;
}

int find_Index(struct array *arr, int size, char word[]) {
    for (int i = 0; i < size; i++) {
        if (strcmp(arr[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}

void *processChunk(void *arg) {
    struct thread_data *data = (struct thread_data *)arg;
    FILE *file = data->file;
    long start = data->start;
    long end = data->end;

    struct array *localArray = makeArray(UNIQUE_SIZE);
    int localCount = 0;

    fseek(file, start, SEEK_SET);

    // Align to the next word boundary if not at the start of the file
    if (start != 0) {
        char c;
        while ((c = fgetc(file)) != EOF && c != ' ' && ftell(file) <= end);
    }

    char word[100];
    while (ftell(file) < end && fscanf(file, "%s", word) == 1) {
        int index = find_Index(localArray, localCount, word);
        if (index != -1) {
            localArray[index].frequency++;
        } else if (localCount < UNIQUE_SIZE) {
            strcpy(localArray[localCount].word, word);
            localArray[localCount].frequency = 1;
            localCount++;
        }
    }

    pthread_mutex_lock(&mutex);
    for (int i = 0; i < localCount; i++) {
        int index = find_Index(globalUniqueArray, globalUniqueCount, localArray[i].word);
        if (index != -1) {
            globalUniqueArray[index].frequency += localArray[i].frequency;
        } else if (globalUniqueCount < UNIQUE_SIZE) {
            strcpy(globalUniqueArray[globalUniqueCount].word, localArray[i].word);
            globalUniqueArray[globalUniqueCount].frequency = localArray[i].frequency;
            globalUniqueCount++;
        }
    }
    pthread_mutex_unlock(&mutex);

    free(localArray);
    fclose(file);
    free(data);

    return NULL;
}

void TopTenMostFrequentWords(struct array *arr, int size) {
    printf("Top 10 Words:\n");
    for (int j = 0; j < 10; j++) {
        int max = 0, max_index = -1;
        for (int i = 0; i < size; i++) {
            if (arr[i].frequency > max) {
                max = arr[i].frequency;
                max_index = i;
            }
        }

        if (max_index != -1) {
            printf("%s %d\n", arr[max_index].word, arr[max_index].frequency);
            arr[max_index].frequency = 0;
        }
    }
}

int main() {

    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    globalUniqueArray = makeArray(UNIQUE_SIZE);
    pthread_mutex_init(&mutex, NULL);

    FILE *file = fopen("test.txt", "r");
    if (file == NULL) {
        printf("Error: Could not open file\n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    long chunkSize = fileSize / NUM_THREADS;
    rewind(file);

    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        struct thread_data *data = (struct thread_data *)malloc(sizeof(struct thread_data));
        data->file = fopen("test.txt", "r");
        data->start = i * chunkSize;
        data->end = (i == NUM_THREADS - 1) ? fileSize : (i + 1) * chunkSize;

        if (pthread_create(&threads[i], NULL, processChunk, (void *)data) != 0) {
            printf("Error: Could not create thread %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    TopTenMostFrequentWords(globalUniqueArray, globalUniqueCount);

    free(globalUniqueArray);
    pthread_mutex_destroy(&mutex);

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("The total time including child processes is: %f minutes\n", elapsed/60);

    return 0;
}
