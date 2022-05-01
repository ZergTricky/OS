#include "lib.h"

#include "stdlib.h"

int PrimeCount(int A, int B){
    int * sieve = (int *)(calloc(B + 1, sizeof(int)));

    sieve[1] = sieve[0] = 1;
    
    for(int i = 2; i * i <= B; ++i){
        if(sieve[i])continue;
        for(int j = i * i;j <= B; j += i){
            sieve[j] = 1;
        }
    }
    
    int result = 0;
    for(int i = A; i <= B; ++i){
        if(sieve[i] == 0){
            ++result;
        }
    }

    free(sieve);
    
    return result;
}

int GCF(int A, int B){
    if(A > B){
        int temp = A;
        A = B;
        B = temp;
    }
    
    int result = 0;

    for(int i = 1; i <= B;++i){
        if(B % i == 0 && A % i == 0){
            result = i;
        }
    }

    return result;
}