#define _GNU_SOURCE

#include "config_file.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE 256

typedef struct {
    char* path;
    uid_t uid;
    gid_t gid;
    gid_t sup_gid[32]; //32 on Linux < 2.6.3, 2^16 on modern Linux
    size_t sup_gid_count; //pointers dont include size, so add it?
} procContext;

void dumpContext(procContext* con) {
    printf("UID: %d\n", con->uid);
    printf("GID: %d\n", con->gid);
    
    printf("Suplementary GIDs: ");
    for(size_t i = 0; i < con->sup_gid_count; i++) { 
        printf("%d ", con->sup_gid[i]);
    }
    printf("\n");
    if (strlen(con->path) == 0) {
        return;
    }
    printf("Path: %s\n", con->path);
}

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
        //TODO set rights from context

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

void parseConfig(config_file_t* cf, procContext* con) {
    con->sup_gid_count = 0;
    
    for (int i = 0; i < cf->nr_entries; i++) {
        if(strcmp(cf->entries[i].key, "user") == 0) {
            struct passwd* user;
            user = getpwnam(cf->entries[i].value);
            con->uid = user->pw_uid;
            con->gid = user->pw_gid;
        } else if(strcmp(cf->entries[i].key, "groups") == 0) {
            //printf("%s\n", cf->entries[i].value);
            con->sup_gid[con->sup_gid_count] = getgrnam(cf->entries[i].value)->gr_gid;
            con->sup_gid_count++;
        } else if(strcmp(cf->entries[i].key, "path") == 0) {
            con->path = cf->entries[i].value;        
        } else {
            fprintf(stderr, "Wrong key: %s\t\tValue: %s\n", cf->entries[i].key, cf->entries[i].value);            
        }
    }
}

int main() {
    config_file_t *cf = read_config_file("./config.conf"); //TODO make function that loads command specific config
    if (cf == NULL) {
        fprintf(stderr, "Cannot open config file\n");
        return 1;
    }
    if (cf->nr_entries < 0) {
        fprintf(stderr, "The config file could not be parsed\n");
    }
    
     procContext* context;
     context->path = ""; //Initialise and check if empty
     parseConfig(cf, context);
     dumpContext(context);    

    
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
        //printf("filename: %s", basename(parameters[0]));
        //system(x);
        executeProcess(parameters);
    }  
    return 0;
}
