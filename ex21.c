#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char getNextChar(int fd, char* bf){

    if(read(fd,bf,1) == 0){
        *bf = EOF;
    }
    return *bf;
}



int compareFile(int fd1, int fd2)
{
 
    int flag = 1;

    char c1;
    char c2;

    char* bf1 = &c1;
    char* bf2 = &c2;

    
    char ch1 = getNextChar(fd1, bf1);
    char ch2 = getNextChar(fd2, bf2);


    do{

        if(ch1 == ch2){
            ch1 = getNextChar(fd1, bf1);
            ch2 = getNextChar(fd2, bf2);
            continue;
        }
        else if ((isalpha(ch1)) && (isalpha(ch2)))
        {
           if (abs(ch1 - ch2) == 32){
                flag = 3;
                ch1 = getNextChar(fd1, bf1);
                ch2 = getNextChar(fd2, bf2);
                continue;
            }
            else {
                flag = 2;
                break;
            }
        }
        else if((ch1 == ' ')||(ch1 == '\n')){
            flag = 3;
            ch1 = getNextChar(fd1, bf1);
            continue;

        }
        else if((ch2 == ' ')||(ch2 == '\n')){
            flag = 3;
            ch2 = getNextChar(fd2, bf2);
            continue;
        }
        else {
            flag = 2;
            break;
        }

    } while (ch1 != EOF || ch2 != EOF);

    return flag;
}

int main(int argc, char* argv[]){

    if(argc < 2){

       return -1;
    }


    int diff;
    int fd1 = open(argv[1], O_CLOEXEC);
    int fd2 = open(argv[2], O_CLOEXEC);

    if((fd1 == -1) || (fd2 == -1)){
        return -1;
    }

    diff = compareFile(fd1, fd2);

    close(fd1);
    close(fd2);


    return diff;
}