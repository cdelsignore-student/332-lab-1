#include <stdio.h>
#include <time.h>

void calculate_usleep(int window_sec, int countMax, unsigned int *sec, unsigned int *usec){
    int utime = window_sec*1000000;
    int usleepInterval = utime/countMax;
    while(usleepInterval >= 1000000){
        *sec += 1;
        usleepInterval -= 1000000;
    }
    *usec = usleepInterval;
    
}

int main(){
    unsigned int sec;
    unsigned int usec;

    calculate_usleep(60, 53, &sec, &usec);
    printf("%u : %u \n", sec, usec);
    return 0;
}