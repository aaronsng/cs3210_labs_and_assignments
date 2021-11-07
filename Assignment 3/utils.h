/***************************************************** 
 * Note: this file is provided for the convenience of
 *       students. Students can choose to edit, delete
 *       or even leave the file as-is. This file will
 *       NOT be replaced during grading.
 ****************************************************/
#ifndef UTILS_H_

#define UTILS_H

// General debug parameter_
#define DEBUG 0
#define CHAR_STREAM_LEN 12
#define KEY_LEN 8

#include "tasks.h"

typedef struct _Files {
    char * file;
    int len;
} Files;
typedef struct _Partitions {
    int len;                // Total number of partitions
    MapTaskOutput ** arr;          // Array of MapTaskOutput aka partitions
} Partitions;
typedef struct _IntCollection {
    int len;
    int val;
    struct _IntCollection * next;
} IntCollection;
typedef struct _CharCollection {
    char key[KEY_LEN];
    IntCollection * val;
    struct _CharCollection * next;
} CharCollection;

int partition(char *key, int num_partitions);
void output_map_task_output(MapTaskOutput * output, int rank);
Partitions * partition_map_task_output(MapTaskOutput * output, int num_partitions);
void keyvalue_to_char_stream(KeyValue * kvs, unsigned char * output);
int sum_char_array(char * char_arr, int len); 
int byte_array_to_int(char * bytes);
#endif 
