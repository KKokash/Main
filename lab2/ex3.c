#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define frequncy 2
#define maxTemp 35.0
#define minTemp -10.0

double randNum(float min, float max)
{return min +(max-min)*rand()/(double)RAND_MAX;}

void main(void){
srand((unsigned)time(NULL));
while(1){
double temp = randNum(minTemp,maxTemp);
time_t t = time(NULL);
char *timestr = asctime(localtime(&t));
if(timestr){
// removes the \n from the end of
// the time string and replaces it with termintor
timestr[strcspn(timestr,"\n")] = '\0';}
printf("Temp = %1.2f at time %s\n", temp, timestr?timestr:"Unknown");
fflush(stdout);
sleep(frequncy);
}
return ;
}