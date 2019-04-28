#include <stdio.h>
#include <stdlib.h>

void *Malloc(unsigned n)
{ void *p;

  if (!(p = malloc(n))) {
     fprintf(stderr,"Malloc(%d) failed.\n",n);
     fflush(stderr);
     abort();
   }
   return p;
}



void reportError(char *s, int lineno)
{
fprintf(stderr, "Error: %s at line %i \n",s,lineno);
    exit(1);
  // errors++;
}



void reportStrError(char *s, char *name, int lineno)
{
  fprintf(stderr, s,name);
  fprintf(stderr, " at line %i\n",lineno);
  exit(1);
  // errors++;
}

void reportGlobalError(char *s)
{ fprintf(stderr, "%s\n",s);
  exit(1);
  // errors++;
}

void reportStrGlobalError(char *s, char *name)
{
  fprintf(stderr, s,name);
  fprintf(stderr, "\n");
  exit(1);
  // errors++;
}
/*
void noErrors()
{ if (errors!=0) {
     ffprintf(stderr, stderr,"*** compilation terminated\n");
     exit(1);
  }
}*/
