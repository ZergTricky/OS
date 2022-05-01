#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

int main(int argc, char* argv[]) {
    char * fileName = argv[1];
    double res = 0.0;
    int x;
    int flag = 1;
    char c;
    FILE * out = fopen(fileName, "w");
    if(out == NULL){
        perror("File not opened\n");
        exit(1);
    }
    while(scanf("%d%c", &x, &c) != EOF) {
        if(flag){
            res = x;
            flag = 0;
        }
        else {
            if(x == 0){
                printf("Zero division!\n");
                exit(1);
            }
            res /= (double)x;
        }
        if(c == '\n') {
            fprintf(out,"%f\n", res);
            res = 0;
            flag = 1;
            continue;
        }
    }
    write(STDOUT_FILENO, "Child finished successfully!\n",sizeof("Child finished successfully!\n"));
    fclose(out);
    return 0;
}