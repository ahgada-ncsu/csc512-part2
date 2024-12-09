#include<stdio.h>
#include <string.h>
#include <stdlib.h>


void fun(int a){
    for(int i = 0; i < a; i++) {
        printf("Hello World\n");
    }
}

int main(){
    char *word = "Hello";
    char *string = (char *)malloc((strlen(word) + 1) * sizeof(char));

    int a;
    scanf("%d",&a);
    void (*fun_ptr)(int) = &fun;
    (*fun_ptr)(a);

    int d = 0;
    for(int i=0;i<3;i++) {
        d+=1;
    }
    return d;
}