#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PPPD_CALL_3G "pppd"

char line[256];
FILE *pp;
char cmd[255];

int main()
{
//    sprintf(cmd, "ps");
    pp = popen("ps", "r");


    if (pp != NULL)
    {
        while (fgets(line, 256, pp))
        {
		if (strstr(line,"pppd call 3g") != NULL)
		{
			printf("%s\n",line);
		}


        }
        pclose(pp);
    }



    return 0;
}


