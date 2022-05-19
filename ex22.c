#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <time.h> 
#include <sys/stat.h>
#include <sys/times.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#define M 8192

int printStr (const char *s){

    return write (STDOUT_FILENO, s, strlen(s));
}

char getNextChar(int fd, char* bf){

    if (read(fd,bf,1) == 0){
        *bf = EOF;
    }
    return *bf;
}

void readLine(int fd, char* line){

    char ch;
    char c;

    getNextChar(fd, &ch);
    int bCounter = 0;
    while (1)
    {
        if (ch == '\n' || ch == EOF){
            line[bCounter] = '\0';
            break;
        }
        line[bCounter] = ch;
        getNextChar(fd, &ch);
        bCounter++;
    }
}
int isDirectory(const char *path) {
   struct stat statbuf;
   if (stat(path, &statbuf) != 0)
       return 0;
   return S_ISDIR(statbuf.st_mode);
}

getRealPath(char *symlinkpath){
        char actualpath [PATH_MAX];
        char *ptr;
        ptr = realpath(symlinkpath, actualpath);
        strcpy(symlinkpath, ptr);
}
int dirIsValid(char* folder){

    int r = 1; //dir is valid.
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if(ent->d_name[0] == '.'){
                continue;
            }
            char* path[PATH_MAX];
            strcpy(path, folder);
            strcat(path, "/");
            strcat(path, ent->d_name);
            if (!(isDirectory(path))){
                //dir contains a file, so dir is not valid.
                r = 0;
                closedir (dir);
                break;

            }
        }

    closedir(dir);
    } 
    else {
        r = 0; //dir is not valid.
        return r; 
    }
    return r;
}

int openConfile(char* conFile, char* folder, char* inputFile, char* expOutPutFile){

    int fd = open(conFile, O_CLOEXEC);
    if (fd == -1){
        return -1;
    }
    readLine(fd, folder);
    readLine(fd,inputFile);
    readLine(fd,expOutPutFile);
    close(fd);

    //check if the files exists.

    int r = 0;

    if (dirIsValid(folder)) {
        getRealPath(folder);
    } 
    else {
        printStr("Not a valid directory\n");
        r = -1;
    }

 
    if (access(inputFile, F_OK) != -1) {
        getRealPath(inputFile);
    }
    else{
        printStr("Input file not exist\n");
        r = -1;
    }
    if (access(expOutPutFile, F_OK) != -1) {
        getRealPath(expOutPutFile);
    }
    else{
        printStr("Output file not exist\n");
        r = -1;
    }

    return r;
}

int isThereACfile(const char *path){
    
    char ending[] = ".c";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path)) != NULL) {
        FILE *fpt;
            while ((ent = readdir(dir)) != NULL) {
                char* t = &(ent->d_name[strlen(ent->d_name) - 2]);
                if((!(strcmp(ending, t))) && (!(isDirectory(ent->d_name)))){
                    closedir (dir);
                    return 1;
                }     
            }

    closedir (dir);
    return 0;
    } 
}

int timer(char** command_arr){

    pid_t pid = getpid();
    int status;

    time_t begin, end;
    time(&begin);

    if ((pid = fork()) == 0) {
        
        if(execvp(command_arr[0], command_arr) == -1){
            printStr("Error in: execvp\n");
            exit(0);
        }
    }
    else {

        pid = wait(&status);
        
    } 

    time(&end);
    time_t elapsed = end - begin;

    return elapsed;

}
int isThereAout(const char* path){
    char aout[] = "a.out";
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(path)) != NULL) {
        FILE *fpt;
            while ((ent = readdir(dir)) != NULL) {
                char t[MAX_INPUT];
                strcpy(t,ent->d_name);
                if(!(strcmp(aout, t))){
                    closedir (dir);
                    return 1;
                }     
    }
    closedir (dir);
    return 0;
    } 
}

int check(const char* path, const char* input, const char* output, char* result, char* bwd){

    
    chdir(path);
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);

    
    //check if there is  c file.
    if(!(isThereACfile(cwd))){
        strcat(result, "0,NO_C_FILE");
        chdir(bwd);
        return 0;

    }
    //prepering the compilation command.
    char errorFile[PATH_MAX];
    strcpy(errorFile, bwd);
    strcat(errorFile, "/errors.txt");
    char* co[M];
    strcpy(co, "gcc *.c -o a.out 2>>");
    strcat(co, errorFile);
    
    //check for compliation errors.
    int flag = system(co);
    if (flag == -1) {
        printStr("Error in: system\n");
        chdir(bwd);
        return -1;
    }
    if(!(isThereAout(cwd))){
        strcat(result, "10,COMPILATION_ERROR");
        chdir(bwd);
        return 0;
    }

    //prepering the command.
    strcpy(co, "./a.out <");
    strcat(co, input);
    strcat(co, " > output.txt");

    //with time check
    // char* command_arr[] = {"bash", "-c", co, NULL};
    // if(timer(command_arr) > 5){
    //     strcat(result, "20,TIMEOUT");
    //     chdir(bwd);
    //     return 0;
    // }

    //without time check 

    system(co);



    //preparing the comand, that compering the outputs.
    char* pout[PATH_MAX]; //absulut path to output.txt
    strcpy(pout, cwd);
    strcat(pout, "/output.txt");

    strcpy(co, "./comp.out");
    strcat(co, " ");
    strcat(co, pout);
    strcat(co, " ");
    strcat(co, output);

    //go back to the dir comp.out is at (same as as this program).
    chdir(bwd);

    //use comp.out to compare output.txt with expected output.
    int r = system(co);
    //system returns the return value of compare multiple by 256 . 
    r = r / 256;

    //put the result in result str.
    switch (r)
    {
    case 1:
        strcat(result, "100,EXCELLENT");
        break;
    case 2:
        strcat(result, "50,WRONG");
        break;
    case 3:
        strcat(result, "75,SIMILAR");
        break; 
    case -1:
        printStr("Error in: system\n");
        return -1; 
    }
    
    chdir(cwd);

    //delete the program output file - pout, and return.
    strcpy(co, "rm ");
    strcat(co, pout);
    if (system(co) == -1) {
        printStr("Error in: system\n");
        chdir(bwd);
        return -1; 
    }
    strcpy(co, "rm ");
    strcat(co, "a.out");
    if (system(co) == -1) {
        printStr("Error in: system\n");
        chdir(bwd);
        return -1; 
    }
    chdir(bwd);
    return 0;
}

int checkAll(char* folder, char* inputFile, char* expOutPutFile, char* cwd){

    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(folder)) != NULL) {
 

        int fd = open("results.csv", O_CREAT);
        close(fd);
        system("chmod ugo+rwx results.csv");
        fd = open("results.csv", O_RDWR);

        while ((ent = readdir(dir)) != NULL) {
            if(ent->d_name[0] == '.'){
                continue;
            }
            char* path[PATH_MAX];
            strcpy(path, folder);
            strcat(path, "/");
            strcat(path, ent->d_name);
            char* result[M];
            strcpy(result, ent->d_name);
            strcat(result, ",");
             if (check(path, inputFile, expOutPutFile, result, cwd) == -1) {
                return -1;
            }
            strcat(result, "\n");
            write(fd,result, strlen(result));  
        }
    close(fd);
    closedir(dir);
    } 
    return 0;
}


int main(int argc, char* argv[]){

    char folder[PATH_MAX];
    char inputFile[PATH_MAX];
    char expOutPutFile[PATH_MAX];

    if(argc < 1){
        return -1;
    }

    if (openConfile(argv[1], folder, inputFile, expOutPutFile) == -1){
        return -1;
    }
    char cwd[PATH_MAX];
    getcwd(cwd, PATH_MAX);
    if (checkAll(folder, inputFile, expOutPutFile,cwd) == -1){
        return -1;
    }
        
    return 0;
}