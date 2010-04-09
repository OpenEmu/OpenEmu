#include <stdio.h>

#define MAX	(256/2)

main()
{
 char buf[256];
 int count=0;

 while(fgets(buf,256,stdin)>0)
 {
  double p;
  if(buf[0] == ';' || buf[0] == '\r' || buf[0] == '\n') continue;
  printf("%s,\n",buf);
  count++;
  if(count==MAX) break;

 }

}
