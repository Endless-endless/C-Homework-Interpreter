# include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "f.h"

void get_dir(char*path,char**argv){
	int i=0,k;
	for(;*(*argv+i)!=0;i++)
		if(*(*argv+i)=='\\')
			k=i;
	for(i=0;i<k;i++)
		*(path+i)=*(*argv+i);
	*(path+i)=0;
}


