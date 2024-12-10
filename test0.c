#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void func(int pp, FILE *fp);
int main();

int a = 0;

void func(int pp, FILE *fp){
    char str1[1000];
    char c;
    a=0;
    int len = a;
    while(1) {
        c = getc(fp);
        if(c == EOF) break;
        str1[len] = c;
        len=len + 1;
        if(len >= 1000) break;
    }
    str1[len] = '\0';  // Add null terminator
    printf("%s\n", str1);  // Removed the * operator
}

int main() {
    FILE * ppp = fopen("file.txt", "r");
    if (ppp == NULL) {  // Add error checking for file open
        printf("Error opening file\n");
        return 1;
    }
    func(5, ppp);
    fclose(ppp);  // Don't forget to close the file
    return 0;
}