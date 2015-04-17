#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <alloca.h>
#include<sys/time.h>
#include<string.h>
struct Model
{
int number;
char* name;
};

int main() 
{
  int n = 0;
  while(n < 50) 
  {

   char *str;
  
   /* Initial memory allocation */
   str = (char *) malloc(4096);
   strcpy(str, "tutorialspoint");

   /* Reallocating memory */
   str = (char *) realloc(str, 4096);
   strcat(str, ".com");

    struct Model *sub_model1;
    struct Model *sub_model2;

    sub_model1 = (struct Model *)malloc(8192); /*For HEAP size */
    sub_model2 = (struct Model *)alloca(8192); /*For STACK size*/
    
    sub_model1->number=2014;
    sub_model1->name="Lamborgini Aventendo";

    sub_model2->number=2015;
    sub_model2->name="Porsche cayman";
    n++;
  
    usleep(500000);
  }
  return 0;
}

