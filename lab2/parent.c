#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>

int main(){
    char *fileName = NULL;
    size_t length;
    getline(&fileName,&length, stdin);
    fileName[strlen(fileName) - 1] = '\0';
    int fd[2];
    int d[2];

    if(pipe(fd) || pipe(d)){
        perror("Pipe error\n");
        exit(1);
    }

    int rfd = fd[0], wfd = fd[1];

    int rd = d[0], wd = d[1];

    pid_t pid = fork();
    if(pid == -1){
        perror("Fork error\n");
        exit(1);
    }
    else if(!pid){
        close(rd);
        close(wfd);
        if(dup2(wd, STDOUT_FILENO) == -1){
            perror("Dup2 error\n");
            exit(1);
        }
        close(wd);
        if(dup2(rfd, STDIN_FILENO) == -1){
            perror("Dup2 error\n");
            exit(1);
        }
        close(rfd);
        execl("child", "child", fileName, NULL);
        perror("execl");
        exit(1);
    }
    
    char c;
    while(scanf("%c",&c) != EOF){
        write(wfd, &c, sizeof(c));
    }
    close(wfd);
    close(rfd);

    close(wd);
    while(read(rd,&c,sizeof(char)) > 0){
        putchar(c);
    }
    close(rd);
    
    int status;
    if(wait(&status) == -1){
        perror("wait");
    }
    if(!WIFEXITED(status) || (WIFEXITED(status) && (WEXITSTATUS(status)) != 0)){
        fprintf(stderr, "Error in child process!\n");
        return 1;
    }
    return 0;
}