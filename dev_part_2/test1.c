#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
   int id;
   int n;
   scanf("%d, %d", &id, &n);
   int s = 0;
   for (int i = 0; i < n; i++) {
      s += rand();
   }
   printf("id=%d; sum=%d\n", id, n);
   return 0;
}


// void func(int pp, FILE *fp);
// int main();

// int a = 0;

// void func(int pp, FILE *fp){
//     char str1[1000];
//     char c;
//     a=0;
//     int len = a;
//     while(1) {
//         c = getc(fp);
//         if(c == EOF) break;
//         str1[len] = c;
//         len=len + 1;
//         if(len >= 1000) break;
//     }
//     str1[len] = '\0';  // Add null terminator
//     printf("%s\n", str1);  // Removed the * operator
// }

// int main() {
//     FILE * ppp = fopen("file.txt", "r");
//     if (ppp == NULL) {  // Add error checking for file open
//         printf("Error opening file\n");
//         return 1;
//     }
//     func(5, ppp);
//     fclose(ppp);  // Don't forget to close the file
//     return 0;
// }

//////////////////////////////////////////////////////////////////////////////////////////

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