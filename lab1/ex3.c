#include <stdio.h>
#include <string.h>
#define MAX 100
int isLower(char c){return (c>= 'a'&& c<='z');}
int isUpper(char c){return (c>= 'A'&& c<='Z');}
int isDigit(char c){return (c>= '0'&& c<='9');}
char toUpper(char c){
if(isLower(c))
  return c- ('a'-'A');
else
  return c;
}
int main(void){
  char first[MAX],second[MAX],str[MAX];
  //int year;
  //name[MAX];
  printf("Enter your first name: ");
  scanf("%s", first);
  printf("Enter your Last name: ");
  scanf("%s", second);
  int i =0;
  while(second[i] != '\0'){
    str[i] = toUpper(second[i]);
    i++;
    }
  str[i]='\0';
  printf("second name upper case: %s\n",str);
return 0;
}
