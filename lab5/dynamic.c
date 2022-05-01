#include "dlfcn.h"
#include "stdio.h"
#include "stdlib.h"

int current_lib = 0;

const char* lib1 = "./lib1.so";
const char* lib2 = "./lib2.so";

void *handle = NULL;

int (*GCF)(int, int);
int (*PrimeCount)(int, int);

void load(){
    if(current_lib == 0){
        handle = dlopen(lib1, RTLD_LAZY);
    }
    else {
        handle = dlopen(lib2, RTLD_LAZY);
    }

    if(handle == 0){
        printf("Error with loading library!\n");
        exit(EXIT_FAILURE);
    }

    PrimeCount = dlsym(handle, "PrimeCount");
    GCF = dlsym(handle, "GCF");
}

void unload(){
    dlclose(handle);
}

void swap_lib(){
    unload();
    
    if(current_lib == 0){
        current_lib = 1;
    }
    else current_lib = 0;
    
    load();
}


void menu(){
    printf("Enter a command: 0 - swap lib\n1 - PrimeCount\n2 - GCF\n");
}

int main(){
    load();

    int c;

    menu();

    while(scanf("%d", &c) != EOF){
        if(c == 0){
            swap_lib();
            const char * name = current_lib ? lib2 : lib1;
            printf("Swapped! Current lib is: %s\n", name);
        }
        else if(c == 1){
            int A, B;

            printf("Enter values for PrimeCount: ");
            scanf("%d %d", &A, &B);

            printf("%d\n", PrimeCount(A, B));
        }
        else{
            int A, B;

            printf("Enter values for GCF: ");
            scanf("%d %d", &A, &B);

            printf("%d\n", GCF(A, B));
        }
        menu();
    }
}