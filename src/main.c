#include "config_file.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE 256

void readInput(char* buffer) {
    
    //TODO Error on buffer overflow
    char* ret = fgets(buffer, INPUT_BUFFER_SIZE, stdin);
    if(ret == NULL) {
        fprintf(stderr, "Cannot read input\n");
    }
//    printf("Input: %s\n", x);
//    printf("Return: %s\n", ret);
//    printf("end: %d\n", x[INPUT_BUFFER_SIZE-1]);
    
}

void executeProcess(char* cmd) {
    int pid = fork();
	if(pid == 0) {
        //TODO strncpm with key
		//set rights
        
        // TODO split command parameters
		//execve(cmd, cmd, (char*) NULL); //TODO error handling
        //cmd = "/bin/ls";
        
        char* paras[50];
        int index = 0;
        
        char* ptr = strtok(cmd," ");
        while(ptr != NULL) {
            printf("parameters: %s\n", ptr);
            paras[index] = ptr;
            
            ptr = strtok(NULL," ");
        }
        
        char* test = "/bin/ls ";
        printf("len %d\n", paras[0][8]);

        unsigned int ret = execl(test, "ls", "-1", (char *)0);
        printf("exec %s", strerror(errno));
	} else {
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
    readInput(x);
    executeProcess(x);

    
    return 0;
}
