#include <stdio.h>
#include <stdlib.h> 

long int fib(int n) {
    if (n == 0 || n == 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}
int main(int argc, char *argv[]) {
    for (int i=0;i<argc;i++) {
        printf("%s\n",argv[i]);
    }
    int n = atoi(argv[1]);  
    // atoi converts a C-style string to an int. It parses the leading numeric portion of the string (including an optional sign) 
    // and returns the result, but it does not provide error reporting and returns 0 if the conversion fails.

    

    long int t = fib(n);
    printf("Fibonacci(%d) = %ld\n", n, t);

    return 0;
}