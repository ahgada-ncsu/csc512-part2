// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>


// int main() {
//    int id;
//    int n;
//    scanf("%d, %d", &id, &n);
//    int s = 0;
//    for (int i = 0; i < n; i++) {
//       s += rand();
//    }
//    printf("id=%d; sum=%d\n", id, n);
//    return 0;
// }


#include<stdio.h>

void fun(int a){
    for(int i = 0; i < a; i++) {
        printf("Hello World\n");
    }
}

int main(){
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