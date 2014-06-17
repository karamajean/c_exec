#include <stdio.h>
#include <string.h>

void printfHex(unsigned char *str)
{
	int i,j;
	int len = strlen(str);
	for( i=0,j=0; i<len/2; i++, j+=2)
    {
		printf("0x");
	    printf("%c%c",str[j],str[j+1]);			
		if(j!=len-2) printf(", ");
    }
	printf("\n");
}

int  main()
{
	unsigned char msg[]="0103202BB6000F1778380003A600068F0000003A000000000000003900000000000006B251";

	unsigned char msg2[]="FFAD800211112222333344445555666677778888";

	unsigned char input[100];


	scanf("%s",input);
	printfHex(input);


	return;
}
