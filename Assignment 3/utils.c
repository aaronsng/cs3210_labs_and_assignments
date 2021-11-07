/***************************************************** 
 * Note: this file is provided for the convenience of
 *       students. Students can choose to edit, delete
 *       or even leave the file as-is. This file will
 *       NOT be replaced during grading.
 ****************************************************/
#include "utils.h"

/* Helper func to choose a partition based on `key`,*/
int partition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;

    while (c = *key++) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % num_partitions;
}

/* Helper function to print the key value pairs given a MapTaskOutput*/
void output_map_task_output(MapTaskOutput * output, int rank) {
    int len = output->len;
    KeyValue * arr;
    arr = output->kvs;

    printf("Rank (%d):\n", rank);
    for (int i = 0; i < len; i++) {
        printf("%s: %d\n", arr[i].key, arr[i].val);
    }
}

/* Helper function to partition the key value pairs given a MapTaskOutput*/
Partitions * partition_map_task_output(MapTaskOutput * output, int num_partitions) {
    int len = output->len;
    int size_of_each_partition[num_partitions];
    memset(size_of_each_partition, 0, num_partitions * sizeof(int));

    // Initialise variable to be returned
    Partitions * computed = malloc(sizeof(Partitions)); 
    computed->len = num_partitions;
    computed->arr = malloc(sizeof(MapTaskOutput *) * num_partitions);
    for (int i = 0; i < len; i++) {
        KeyValue kv = output->kvs[i];
        size_of_each_partition[partition(kv.key, num_partitions)] += 1;
    }

    // malloc each partition with the key-value
    for (int i = 0; i < num_partitions; i++) {
        KeyValue * partition_kv = malloc(sizeof(KeyValue) * size_of_each_partition[i]);
        MapTaskOutput * individual_partition = malloc(sizeof(MapTaskOutput));
        individual_partition->kvs = partition_kv;
        individual_partition->len = size_of_each_partition[i];
        computed->arr[i] = individual_partition;
    }
    memset(size_of_each_partition, 0, num_partitions * sizeof(int));

    // Push them to all the partitions
    for (int i = 0; i < len; i++) {
        KeyValue kv = output->kvs[i];
        int partition_id = partition(kv.key, num_partitions);
        computed->arr[partition_id]->kvs[size_of_each_partition[partition_id]] = kv;
        size_of_each_partition[partition_id] += 1;
    }

    return computed;
}

/* Function to convert KeyValue pair to a char array of 12 bytes */
void keyvalue_to_char_stream(KeyValue * kvs, unsigned char * output) {
    strncpy(output, kvs->key, KEY_LEN);
    unsigned long val = kvs->val;
    *(output + KEY_LEN) = (val >> 24) & 0xFF;
    *(output + KEY_LEN + 1) = (val >> 16) & 0xFF;
    *(output + KEY_LEN + 2) = (val >> 8) & 0xFF;
    *(output + KEY_LEN + 3) = val & 0xFF;
}

/* Simple function to find the sum of a char array. Returns an integer */
int sum_char_array(char * char_arr, int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += char_arr[i];
    }
    return sum;
}

/* Convert byte array to a 32-bit integer */
int byte_array_to_int(char * bytes) {
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += (bytes[i] & 0xFF) << ((3 - i) * 8);
    }
    return sum;
}
