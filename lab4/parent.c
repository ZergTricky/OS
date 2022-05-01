#include "stdlib.h"
#include "unistd.h"
#include "stdio.h"
#include "semaphore.h"
#include "sys/wait.h"
#include "sys/mman.h"
#include "string.h"

#include "names.h"

int main(){
    char * filename = NULL;
    size_t length;

    if(getline(&filename,&length,stdin) == -1){
        perror("Error with getline!\n");
        exit(EXIT_FAILURE);
    }
    filename[strlen(filename) - 1] = '\0';
    int shm_input = shm_open(InputFile, O_RDWR | O_CREAT, params);
    int shm_error = shm_open(ErrorFile, O_RDWR | O_CREAT, params);

    if(shm_input == -1 || shm_error == -1){
        perror("Error with opening shared file!\n");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm_input, MAPPING_SIZE) == -1){
        perror("Error with ftruncate!\n");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm_error,MAPPING_SIZE) == -1){
        perror("Error with ftruncate!\n");
        exit(EXIT_FAILURE);
    }

    char * addr_input = mmap(0, MAPPING_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_input, 0);
    char * addr_error = mmap(0, MAPPING_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_error, 0);

    if(addr_input == (char *)-1 || addr_error == (char *)-1){
        perror("Error with mmap!\n");
        exit(EXIT_FAILURE);
    }

    sem_t * sem_input = sem_open(InputSemaphore, O_CREAT, params, 1);
    sem_t * sem_error = sem_open(ErrorSemaphore, O_CREAT, params, 1);

    int input_value, error_value;

    if(sem_getvalue(sem_input, &input_value) != 0){
        perror("Error with sem_getvalue!\n");
        exit(EXIT_FAILURE);
    }
    if(sem_getvalue(sem_error, &error_value) != 0){
        perror("Error with sem_getvalue!\n");
        exit(EXIT_FAILURE);
    }

    while(input_value < 1){
        sem_post(sem_input);
    }

    while(error_value < 1){
        sem_post(sem_error);
    }
    
    int pid = fork();

    if(!pid){
        sem_close(sem_input);
        sem_close(sem_error);
        munmap(addr_input, MAPPING_SIZE);
        munmap(addr_error, MAPPING_SIZE);
        execl("child", "child", filename, NULL);
        perror("execl\n");
        exit(EXIT_FAILURE);
        exit(0);
    }
    memset(addr_input, '\0', sizeof(addr_input));

    fflush(STDIN_FILENO);
    
    char c;
    char buf[512];
    memset(buf, '\0', sizeof(buf));
    int ptr = 0;
    while((c = getchar()) != EOF){
        buf[ptr++] = c;
        if(c == '\n'){
            while(1){
                if(sem_wait(sem_input) == 0){
                    sprintf(addr_input, "%s", buf);
                    sem_post(sem_input);
                    memset(buf,'\0', sizeof(buf));
                    ptr = 0;
                    break;
                }
                else{
                    exit(EXIT_FAILURE);
                }
            }
        }
    }
    while(1){
            if(sem_wait(sem_input) == 0){
                addr_input[0] = EOF;
                sem_post(sem_input);
                break;
            }
            else{
                exit(EXIT_FAILURE);
            }
    }

    while(1){
        if(sem_wait(sem_error) == 0) {
            if(addr_error[0] == EOF)break;
            if(addr_error[0] == '\0'){
                sem_post(sem_error);
                continue;
            }
            char * str = (char *)(malloc(sizeof(char) * strlen(addr_error)));
            strcpy(str, addr_error);
            printf("%s", str);
            sem_post(sem_error);
            memset(addr_error, '\0', sizeof(addr_error));
            free(str);
            break;
        }
        else{
            printf("Error sem_wait!\n");
            exit(EXIT_FAILURE);
        }
    }
    
    int status;
    if(wait(&status) == -1){
        perror("wait");
    }
    if(!WIFEXITED(status) || (WIFEXITED(status) && (WEXITSTATUS(status)) != 0)){
        fprintf(stderr, "Error in child process!\n");
        return 1;
    }
    munmap(addr_error, MAPPING_SIZE);
    munmap(addr_input, MAPPING_SIZE);
    sem_close(sem_input);
    sem_close(sem_error);
    return 0;
}