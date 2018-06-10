#include "config_file.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE 256

void dumpStr(char* str) {
     for(size_t i = 0; str[i] != 0; i++) {
         printf("%x ", str[i]);
    }
    printf("\n");
}

char* readInput(char* buffer) {    
    //TODO Error on buffer overflow
    //getline?   
    return fgets(buffer, INPUT_BUFFER_SIZE, stdin);
    
}

void executeProcess(char* cmd) {
    int pid = fork();
	if(pid == 0) {
        //TODO strncpm with key
		//set rights
        
   
        char* paras[50];
        int index = 0;
        
        char* ptr = strtok(cmd," ");
        while(ptr != NULL) {
            //printf("parameters: %s\n", ptr);
            
            int lastChar = strlen(ptr)-1;
            //printf("last: %x\n", ptr[lastChar]);
            if(ptr[lastChar] == '\n') {
                ptr[lastChar] = '\0';
            }
            //dumpStr(ptr);
            paras[index] = ptr;
            
            index++;
            ptr = strtok(NULL," ");
        }

        // execvp searches in PATH if 1. arg contains no slash
        if (execvp(paras[0], paras) != NULL) {
            printf("exec %s\n", strerror(errno));
            _exit(1);            
        }
	} else {
        //printf("PID: %d", pid);
        wait(&pid);
    }
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
    
    
    char x[INPUT_BUFFER_SIZE];
    
    while(1) {
        printf("CMD: ");
        char* result = readInput(x);
        if(result == NULL || x == NULL) {
            printf("Exiting...\n");
            return 0; //Exit with Ctrl-D
        }
        if(x[0] == '\n') {
            //printf("No input\n");
            continue;
        }
        executeProcess(x);
    }    
    return 0;
}
