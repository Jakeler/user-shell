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
    //TODO Error on buffer overflow / malloc
    //or switch to getline?   
    return fgets(buffer, INPUT_BUFFER_SIZE, stdin);
    
}

char** processCmd(char* cmd, char** paras) {
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
        paras[index] = (char*)0;
        return paras;
}

void executeProcess(char** parameters) {
    int pid = fork();
	if(pid == 0) {
        //TODO strncpm with key
        //set rights

        // execvp searches in PATH if 1. arg contains no slash
        if (execvp(parameters[0], parameters)) {
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
        if(result == NULL || x == NULL) { //Exit with Ctrl-D
            printf("Exiting...\n");
            return 0;
        }
        if(x[0] == '\n') {
            //printf("No input\n");
            continue;
        }
        char* parameters[16];
        processCmd(x, parameters);
        executeProcess(parameters);
    }    
    return 0;
}
