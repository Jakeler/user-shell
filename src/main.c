#include "config_file.h"

#include <stdio.h>

#define INPUT_BUFFER_SIZE 3

void readInput() {
    char x[INPUT_BUFFER_SIZE];
    // Error on buffer overflow
    char* ret = fgets(x, INPUT_BUFFER_SIZE, stdin);
    if(ret == NULL) {
        fprintf(stderr, "Cannot read input\n");
    }
//    printf("Input: %s\n", x);
//    printf("Return: %s\n", ret);
//    printf("end: %d\n", x[INPUT_BUFFER_SIZE-1]);
    
}

int main() {
    config_file_t *cf = read_config_file("./config.conf");
    if (cf == NULL) {
        fprintf(stderr, "Cannot open config file\n");
        return 1;
    }
    if (cf->nr_entries < 0) {
        fprintf(stderr, "The config file could not be parsed\n");
    }

    for (int i = 0; i < cf->nr_entries; i++) {
        printf("Key: %s\t\tValue: %s\n", cf->entries[i].key, cf->entries[i].value);
    }
    if(!release_config(cf)) {
        fprintf(stderr, "Error: Could not free config\n");
    }
    
    
    
    readInput();
    readInput();

    
    return 0;
}
