/* main.c */
#include <stdio.h>
#include <string.h>
#include <syslog.h>



int main(int argc, char *argv[]) {
  
  FILE *fptr;

  setlogmask (LOG_UPTO (LOG_DEBUG));
  openlog("writer", 0, LOG_USER);
  
  if (argc != 3)
    {
      syslog(LOG_ERR,"missing arguments");
      return 1;
    }
  
  fptr = fopen(argv[1], "w");

  if (fptr == NULL)
    {
      syslog(LOG_ERR,"can't write file");
      return 1;
    }
  
  fprintf(fptr,"%s",argv[2]);

  fclose(fptr);
  
  syslog(LOG_DEBUG,"Writing %s to %s",argv[1],argv[2]);

  return 0;
}

