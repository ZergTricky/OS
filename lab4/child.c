#include "stdlib.h"
#include "unistd.h"
#include "stdio.h"
#include "semaphore.h"
#include "sys/wait.h"
#include "sys/mman.h"
#include "string.h"

#include "names.h"

int main(int argc,char * argv[]){
    int shm_error = shm_open(ErrorFile, O_RDWR | O_CREAT, params);

    char * addr_error = mmap(0, MAPPING_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_error, 0);

    sem_t * semerror = sem_open(ErrorSemaphore, O_CREAT, params, 1);

    if(shm_error == -1){
        perror("Error with opening shared file!\n");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm_error,MAPPING_SIZE) == -1){
        perror("Error with ftruncate!\n");
        exit(EXIT_FAILURE);
    }

    char * filename = argv[1];
    FILE * out = fopen(filename, "w");
    if(out == NULL){
        while(1){
            if(sem_wait(semerror) == 0){
                perror("File not opened\n");
                sem_post(semerror);
                exit(EXIT_FAILURE);
            }
            else {    
                perror("Error with sem_wait\n");
                exit(EXIT_FAILURE);
            }
        }
    }
    int shm_input = shm_open(InputFile, O_RDWR | O_CREAT, params);

    if(shm_input == -1 ){
        perror("Error with opening shared file!\n");
        exit(EXIT_FAILURE);
    }

    if(ftruncate(shm_input, MAPPING_SIZE) == -1){
        perror("Error with ftruncate!\n");
        exit(EXIT_FAILURE);
    }

    char * addr_input = mmap(0, MAPPING_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, shm_input, 0);
    
    sem_t * semptr = sem_open(InputSemaphore, O_CREAT, params, 1);
    
    while(1){
        if(sem_wait(semptr) == 0){
            if(addr_input[0] == EOF)break;
            if(addr_input[0] == '\0'){
                sem_post(semptr);
                continue;
            }
            char * parent_input = (char * )malloc(strlen(addr_input) * sizeof(char));
            strcpy(parent_input, addr_input);

            int value = 0;
            double result = 0;
            int flag = 1;

            for(size_t i = 0;parent_input[i] != '\0';++i){
                char c = parent_input[i];
                if(c == ' '){
                    if(flag){
                        result = value;
                        flag = 0;
                    }
                    else{
                        if(value == 0){
                            while(1){
                                if(sem_wait(semerror) == 0){
                                    sprintf(addr_error, "Zero division!\n");
                                    sem_post(semerror);
                                    exit(EXIT_FAILURE);
                                }
                            }
                        }
                        result /= (double)value;
                    }
                    value = 0;
                }
                else if(c == '\n'){
                    if(flag){
                        result = value;
                        flag = 0;
                    }
                    else{
                        if(value == 0){
                            while(1){
                                if(sem_wait(semerror) == 0){
                                    sprintf(addr_error, "Zero division!\n");
                                    sem_post(semerror);
                                    exit(EXIT_FAILURE);
                                }
                            }
                        }
                        result /= (double)value;
                    }
                    value = 0;
                    break;
                }
                else{
                    value *= 10;
                    value += (c - '0');
                }
            }

            fprintf(out, "%f\n", result);

            memset(addr_input, '\0', sizeof(addr_input));
            free(parent_input);
            sem_post(semptr);
        }
        else {
            perror("Error with sem_wait\n");
            exit(EXIT_FAILURE);
        }
    }
    while(1){
        if(sem_wait(semerror) == 0){
            addr_error[0] = EOF;
            sem_post(semerror);
            break;
        }
    }
    sem_close(semptr);
    sem_close(semerror);
    munmap(addr_error, MAPPING_SIZE);
    munmap(addr_input, MAPPING_SIZE);
    fclose(out);
    return 0;
}