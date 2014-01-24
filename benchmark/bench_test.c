#include <stdio.h>
#include <string.h>

#define MUL_NUM  100

int int_overflow(int size)
{
	return size * MUL_NUM;
}

int main(int argc, char *argv[])
{
  char str[3];
  str[0] = 'a';
  str[1] = 'a';

  int argv1; //a int from user
  char argv2[16]; // a array char from user
  
  argv1 = atoi(argv[1]); //simulate assign the value
  strcpy(argv2, argv[2] );

//  klee_make_symbolic(argv2, 4, "argv2");

  if(argc < 3 ) 
	  return -1;

  if(strcmp(argv2, "hello") == 0 )
	  printf("hello body \n");
  else if(strcmp(argv2, "bug") == 0){
	  int_overflow(argv1);
	  printf("Congratulations, a bug here\n");
  }

  return 0;
}

