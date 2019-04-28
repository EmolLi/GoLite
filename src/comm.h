#define NEW(type) (type *)Malloc(sizeof(type))

void *Malloc(unsigned n);

enum modes{scan, tokens, parse, pretty, symbol, typecheck, codegen};


void reportError(char *s, int lineno);

void reportStrError(char *s, char *name, int lineno);

void reportGlobalError(char *s);

void reportStrGlobalError(char *s, char *name);

void noErrors();

extern int lineno;
