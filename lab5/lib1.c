#include "lib.h"

int PrimeCount(int A, int B){
    int result = 0;
    for(int i = A;i <= B; ++i){
        if(i == 1)continue;
        int ok = 1;
        for(int j = 2;j < i; ++j){
            if(i % j == 0){
                ok = 0;
                break;
            }
        }
        if(ok)++result;
    }
    return result;
}

int GCF(int A, int B){
    while(A != 0){
        B %= A;
        int temp = A;
        A = B;
        B = temp;
    }
    return B;
}