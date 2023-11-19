#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

// Handler function for SIGFPE
void handle_sigfpe(int signal) {
    printf("Division by zero error!\n");
    exit(1); // Exit the program with a status code of 1
}

int main() {
    // Set up the signal handler for SIGFPE
    signal(SIGFPE, handle_sigfpe);

    // Your original expression
    printf("%u\n", (unsigned)(unsigned)2069029000+(unsigned)(unsigned)1534132498/(unsigned)((unsigned)(unsigned)((unsigned)1105820506)/(unsigned)((unsigned)(unsigned)949227992+(unsigned)((unsigned)2897407458))));

    return 0;
}
