#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tree.h"
#include "pretty.h"
#include "comm.h"
#include "weed.h"
#include "symbol.h"
#include "type.h"
#include "code.h"

/* #include "pretty.h" */

void yyparse();
int yylex();

PROG *root = NULL;
SymbolTable *table;

enum modes mode;

int main(int argc, char *argv[])
{


    if (strcmp(argv[1], "scan") == 0) mode = scan;
    else if(strcmp(argv[1], "tokens") == 0) mode = tokens;
    else if(strcmp(argv[1], "parse") == 0) mode = parse;
    else if(strcmp(argv[1], "pretty") == 0) mode = pretty;
    else if (strcmp(argv[1], "symbol") == 0) mode = symbol;
    else if (strcmp(argv[1], "typecheck") == 0) mode = typecheck;
    else if (strcmp(argv[1], "codegen") == 0) mode = codegen;
    else{
        fprintf (stderr, "Error: Bad Command-Line option %s\n", argv[1]);
        exit(1);
    }

    initTypes();
    switch(mode){

    case scan:
      while(yylex()){}
      printf("OK\n");
      break;

    case tokens:
      while(yylex()){}
      break;

    case parse:
      yyparse();
      weedPROGRAM(root);
      printf("OK\n");
      break;

    case pretty:
      yyparse();
      weedPROGRAM(root);
      prettyPROG(root);
      break;

    case symbol:
      yyparse();
      weedPROGRAM(root);
      table = genSymbolTable(root, 1);
      break;

    case typecheck:
      yyparse();
      weedPROGRAM(root);
      table = genSymbolTable(root, 0);
      typeCheckPROG(root);
      printf("OK\n");
      break;

    case codegen:
        yyparse();
        weedPROGRAM(root);
        table = genSymbolTable(root, 0);
        typeCheckPROG(root);
        code(root, argv[2]);
        printf("OK\n");
        break;

    default:
      break;
    }

    return 0;
}
