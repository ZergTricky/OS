#include "lib1.c"
#include "stdio.h"

void menu(){
    printf("Enter a command: 1 - PrimeCount\n2 - GCF\n");
}

int main(){
    
    int c;

    menu();

    while(scanf("%d",&c) != EOF){
        if(c == 1){
            int A, B;
            printf("Enter values for PrimeCount: ");
            scanf("%d %d", &A, &B);
            printf("%d\n", PrimeCount(A, B));
        }
        else if(c == 2){
            int A, B;
            printf("Enter values for GCF: ");
            scanf("%d %d", &A, &B);
            printf("%d\n", GCF(A, B));
        }
        menu();
    }

    return 0;
}