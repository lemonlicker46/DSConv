#include <stdio.h>

unsigned char IsSeparator[256] = { ['\0']=2,['\t']=1,['\r']=2,['\n']=2,[' ']=1,[';']=1 };


int main()
{
    unsigned char line[4096];
    while (1) {
        fgets(line,4096,stdin);
        unsigned char* pzStart = line, *pzEnd = line, OrgChar=1;
        while (1) {
            if (IsSeparator[OrgChar = *pzEnd]) {
              *pzEnd = 0;
              if (pzStart != pzEnd) {
                printf("Token: '%s'\n",pzStart);
              }
              if (OrgChar==';') { putchar(';'); }
              *pzEnd = OrgChar; pzStart = pzEnd+1;
              if (IsSeparator[OrgChar]==2) break;
            }
            pzEnd++;
        }
    }
    return 0;
}



