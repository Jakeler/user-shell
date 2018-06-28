#define _GNU_SOURCE

#include "config_file.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h> 
#include <string.h>
#include <sys/stat.h>
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
    size_t sup_gid_count;
} procContext;


void executeProcess(char** parameters, procContext* con) {
   
    int pid = fork();
	if(pid == 0) {   
        if (con != NULL) {
            errno = 0;
            setresuid(con->uid, con->uid, con->uid);
            perror("> UID set");
            errno = 0;
            setresgid(con->gid, con->gid, con->gid);
            perror("> GID set");
            errno = 0;
            setgroups(con->sup_gid_count, con->sup_gid);
            perror("> Groups set");
        }
        // execvp searches in PATH if 1. arg contains no slash, absolute paths from the config get used directly
        if (execvp(parameters[0], parameters)) {
            printf("exec: %s\n", strerror(errno));           
        }
	} else {
        wait(&pid);
    }
}

int parseConfig(config_file_t* cf, procContext* con) {
    int error_count = 0;
    
    con->sup_gid_count = 0;
    con->path = NULL; // Initialise to make checks possible
    bool got_user = false;
    
    for (int i = 0; i < cf->nr_entries; i++) {
        if(strcmp(cf->entries[i].key, "user") == 0) {
            struct passwd* user = getpwnam(cf->entries[i].value);
            if (user == NULL) {
                error_count++;
                printf("User %s not found\n", cf->entries[i].value);
                //perror("User");
            } else {
                got_user = true;
                con->uid = user->pw_uid;
                con->gid = user->pw_gid;
            }
        } else if(strcmp(cf->entries[i].key, "groups") == 0) {
            struct group* grp = getgrnam(cf->entries[i].value);
            if (grp == NULL) {
                error_count++;
                printf("Group %s not found\n", cf->entries[i].value);
                //perror("Groups");
            } else {
                con->sup_gid[con->sup_gid_count] = grp->gr_gid; //Error handling
                con->sup_gid_count++;
            }
        } else if(strcmp(cf->entries[i].key, "path") == 0) {
            con->path = cf->entries[i].value;
        } else {
            error_count++;
            fprintf(stderr, "Wrong key: %s\t\tValue: %s\n", cf->entries[i].key, cf->entries[i].value);            
        }
    }
    
    if(con->path == NULL || !got_user) {
        printf("> Ingnoring incomplete config! Make sure it contains at least an user and path\n");
        error_count++;
    }
    
    return error_count;
}

void dumpContext(procContext* con) {
    printf("UID: %d\n", con->uid);
    printf("GID: %d\n", con->gid);
    
    printf("Suplementary GIDs: ");
    for(size_t i = 0; i < con->sup_gid_count; i++) { 
        printf("%d ", con->sup_gid[i]);
    }
    printf("\n");
    if (con->path == NULL) {
        printf("No path included\n");
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
    //Cuts of the input if it is more than the buffer size
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


int main() {
    char x[INPUT_BUFFER_SIZE];
   
    printf("\nWelcome to the User Shell!\n");
    
    while(1) {
        printf("CMD: ");
        char* result = readInput(x);
        if(result == NULL || x == NULL) { //Exit with Ctrl-D
            printf("Exiting...\n");
            return 0;
        }
        if(x[0] == '\n') { //just return hit without any input
            continue;
        }
        char* parameters[16];
        processCmd(x, parameters);
        
        char cf_path[256];
        sprintf(cf_path, "/etc/ush/%s.conf", basename(parameters[0]));
        
        struct stat cf_stats;
        int stat_status = stat(cf_path, &cf_stats);
        
        int error = 0;
        
        if (stat_status != 0) {
            printf("> Config file for command %s not found, using standard\n", parameters[0]);
            error++;
        }        
        if ((S_IWOTH & cf_stats.st_mode) != 0) {
            printf("> Ignoring config file, because of set write bit for others\n");
            error++;
        }
        if ((S_IWGRP & cf_stats.st_mode) != 0) {
            printf("> Ignoring config file, because of set write bit for group\n");
            error++;
        }       
        
        
        config_file_t *cf;
        procContext context;
        
        
        if (error == 0) {
            cf = read_config_file(cf_path);
            if (cf == NULL || cf->nr_entries < 0) {
                fprintf(stderr, "> Cannot open config file, using standard user, groups...\n");
                error++;
            }
        }
        if (error == 0) {
            error += parseConfig(cf, &context);
            dumpContext(&context);
            
            if (cf_stats.st_uid != context.uid && cf_stats.st_uid != 0) {
                printf("> Ignoring config file, because of other (non root) owner\n");
                error++;
            }
        }
        
        if (error == 0) {
            parameters[0] = context.path; //Use always path from config
            if (parameters[0] != NULL) {
                executeProcess(parameters, &context);
            }
            
            if(!release_config(cf)) {
                fprintf(stderr, "> Error: Could not free config\n");
            }
        } else { //Execute without changing context if any error exists
            executeProcess(parameters, NULL);
        }
        
    }  
    return 0;
}
