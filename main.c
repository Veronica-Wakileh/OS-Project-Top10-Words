#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define MAX_SIZE 1705210
#define UNIQUE_SIZE 255000

 struct array {
     char word[100];
    int frequency;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct array * makeArray(int size) {
    struct array * arr;
    arr = (struct array *) malloc(size * sizeof(struct array));
    if (arr == NULL) {
        printf("out of memory!\n");
        exit(0);
    }
    return arr;

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int find_Index(struct array * arr, int size, char word[]) {
    for (int i = 0; i < size; i++) {
        if (strcmp(arr[i].word, word) == 0) {
            return i;
        }
    }
    return -1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct array* compareAndCreateArray(struct array * arr) {
    struct array * arr1 = makeArray(UNIQUE_SIZE);
    int k = 0 ;
    for (int i = 0; i < MAX_SIZE ;  i++) {
        if (k < UNIQUE_SIZE) {
        int index = find_Index(arr1,k, arr[i].word);
        if (index != -1) {
            arr1[index].frequency++;
        }
        else {
            strcpy(arr1[k].word, arr[i].word);
            arr1[k].frequency = 1;
            k++;
        }

        }

    }

    return arr1;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct array * insertArray() {
    struct array * arr = makeArray(MAX_SIZE);
    FILE *file  = fopen("test.txt", "r");

    if (file == NULL) {
        printf("Error: Could not reopen file\n");
        free(arr);
        exit(0);
    }

    for(int i = 0; i <MAX_SIZE; i++){
        if (fscanf(file, "%s", arr[i].word) !=1) {
            break;
        }
        arr[i].frequency = 1;
    }
        fclose(file);
        return arr;
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TopTenMostFrequentWords(struct array * arr2) {
    int max, max_index;
    printf("Top 10 Words:\n");
    for (int j = 0; j < 10; j++) {
        max = 0;
        max_index = 0;
        for (int i = 0; i < UNIQUE_SIZE; i++){
            if (arr2[i].frequency > max){
                max = arr2[i].frequency;
                max_index = i;
            }
        }

        printf("%s %d\n", arr2[max_index].word, max);

        arr2[max_index].frequency = 0;
    }
    }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    int main() {
        const clock_t start = clock();
        struct array *arr1 = insertArray();
        struct array *arr2 = compareAndCreateArray(arr1);
        TopTenMostFrequentWords(arr2);
        const clock_t end = clock();
        double cpu_time_used = ((double) (end - start)) ;
        printf("The total time is: %f minutes\n", cpu_time_used / 60);
        free(arr1);
        free(arr2);
        return 0;
    }