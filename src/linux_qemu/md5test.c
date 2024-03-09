#include "md5_copy.h"
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
int main(){
	int times = 10000;
	    struct timeval tv;
    unsigned long long timestamp1, timestamp2;

    gettimeofday(&tv, NULL);
 timestamp1 = (unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000;
	char *content = "abcdef";
	unsigned char output[16], temp[16];
	md5(content, 6, temp);
	for(int i=0; i < times-1; i++) {
		md5(temp, 16, output);
		memcpy(temp, output, 16);
	}
    gettimeofday(&tv, NULL);
timestamp2 = (unsigned long long)tv.tv_sec * 1000 + (unsigned long long)tv.tv_usec / 1000;
printf("time use: %lld ms\n", timestamp2 - timestamp1);	
return 0;

}

