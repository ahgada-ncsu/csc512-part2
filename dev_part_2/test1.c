#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    char * str1[1000];
    FILE * fp = fopen("file.txt", "r");  // Note: Fixed FILE instead of File
    char c;
    int len = 0;
    while(1) {
        c = getc(fp);
        if(c == EOF) break;
        str1[len++] = c;
        if(len >= 1000) break;
    }
    printf("%s\n", str1);
}

// void fun(int a){
//     printf("%d\n",a);
// }

// int main(){
//     void (*fun_ptr)(int) = &fun;
//     (*fun_ptr)(10);

//     int d = 0;
//     for(int i=0;i<3;i++) {
//         d+=1;
//     }

//     int k = 10;
//     int l = 100;

//     if(k>9){
//         if(l<200){ 
//             k-=1;
//         }
//     }else if(k==10) {
//         k+=1;
//     }else{
//         k+=2;
//     }

//     return d;
// }