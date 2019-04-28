#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "symbol.h"
#include "pretty.h"
#include "comm.h"

/* The symbol table consists of a tree of SymbolTable structs, each with a (possibly NULL)
   pointer to the parent table. Each of these SymbolTable structs contains a hash
*/


/* Symbol Functions & Helper Functions */

/* TODO: in symbol phase, we already checked:
   1. if a type is already defined/not defined
   2. Redefinition of an identifier (including method name)/undefined
   3. formal name already defined/undefined
   4. when use symVar() to get the symbol of a variable, symVar will check if the input is indeed a local variable (see JOOS symbol.c 393 line)

   These will not be checked in type.c
   basically report same amount of  errors discovered in Symbol phase as JOOS did
*/

int print_symbols;
int indentation;
#define PRINT_INDENT(...) for(int i=0; i<indentation; i++) printf("   "); printf(__VA_ARGS__);
#define PRINT_BLANK() for(int i=0; i<indentation; i++) printf("   ");

int hash(char *str){
  unsigned int hash = 0;
  while (*str) hash = (hash << 1) + *str++;
  return hash % HashSize;
}

SymbolTable *scopeSymbolTable(SymbolTable *table){
  SymbolTable *t = initSymbolTable();
  t->parent = table;
  return t;
}

/*
  Insert a new symbol into the SymbolTable *table
*/
SYMBOL *putSymbol(SymbolTable *table, char *identifier, SYMBOL_Kind kind, int lineno){
  int i = hash(identifier);
  for (SYMBOL *s = table->table[i]; s; s = s->next) {
    if (strcmp(s->identifier, identifier) == 0) { // Throw Error
      fprintf (stderr, "Error: (line %d) Illegal redeclaration of symbol '%s'\n",
               lineno, identifier);
      exit(1);
    }
  }

  SYMBOL *s = malloc(sizeof(SYMBOL));
  s->identifier = malloc(sizeof(char)*(strlen(identifier)+1));
  strcpy(s->identifier, identifier);
  s->kind = kind;
  s->next = table->table[i];

  // check for shadowing
  SYMBOL *ss = getSymbol(table, identifier);
  if (ss != NULL){
      s->shadowNum  = ss->shadowNum + 1;
  }
  else s->shadowNum = 0;

  table->table[i] = s;
  return s;
}


// TODO: if symbol not in table, try searching for symbol in parent table
// only return NULL if it's not in current scope AND all parent scopes
SYMBOL *getSymbol(SymbolTable *table, char *identifier){
  int i = hash(identifier);
  for (SYMBOL *s = table->table[i]; s; s = s->next) { // Check the current scope
    if (strcmp(s->identifier, identifier) == 0) return s;
  }
  // Check for existence of a parent scope
  if (table->parent == NULL)
    return NULL;
  // Check the parent scopes
  return getSymbol(table->parent, identifier);
}


/* Return 1 if symbol defined in table (at current level),
   0 otherwise (regardless of hash collision)
*/
int isDefSymbol(SymbolTable *table, char *identifier){
  int i = hash(identifier);

  for (SYMBOL *s = table->table[i]; s; s = s->next)
    if (strcmp(s->identifier, identifier) == 0)
      return 1;

  return 0;
}

void printType(TYPE *type){
  switch(type->kind){
  case intK:
    printf("int");
    break;
  case float64K:
    printf("float64");
    break;
  case runeK:
    printf("rune");
    break;
  case stringK:
    printf("string");
    break;
  case boolK:
    printf("bool");
    break;
  case structK:
    printf("struct { ");
    FIELD_DECL *fields = type->val.fieldDecls;
    while(fields != NULL){
      printf("%s ",fields->identifier);
      printType(fields->type);
      printf("; ");
      fields = fields->next;
    }
    printf("}");
    break;
  case sliceK:
    printf("[]");
    printType(type->val.sliceElement);
    break;
  case arrayK:
    printf("[%d]", type->val.array.size);
    printType(type->val.array.element);
    break;
  case refK:
    printf("%s",type->val.name);
    break;
  case inferK:
    printf("<infer>");
    break;
  }
}

void printPARAMETERS(PARAMETERS *params){
  PARAMETERS *current = params;
  while(current){
    printType(current->type);
    if(current->next)
      printf(", ");
    current = current->next;
  }
}

void printSymbol(SymbolTable *table, SYMBOL *symbol){
  switch(symbol->kind){
  /*case constSymK:
    PRINT_INDENT("%s [constant] = ", symbol->identifier);
    printType(symbol->val.constSpec->type);
    break;*/
  case typeSymK:
    PRINT_INDENT("%s [type] = %s -> ", symbol->identifier, symbol->identifier);
    printType(symbol->val.typeSpec->type);
    break;
  case varSymK:
    PRINT_INDENT("%s [variable] = ", symbol->identifier);
    printType(symbol->val.varSpec->type);
    break;
  case fieldSymK:
    // TODO:
    break;
    // PRINT_INDENT("%s [struct] = ", symbol->identifier);
  case funcSymK:
    PRINT_INDENT("%s [function] = (", symbol->identifier);
    if(symbol->val.funcSpec->parameters)
      printPARAMETERS(symbol->val.funcSpec->parameters);
    printf(") -> ");
    if(symbol->val.funcSpec->returnType)
      printType(symbol->val.funcSpec->returnType);
    else
      printf("void");

    printf("\n");
    PRINT_INDENT("{\n");
    indentation++;

    PARAMETERS *currentParam = symbol->val.funcSpec->parameters;
    while(currentParam != NULL){
      SYMBOL *s = getSymbol(table, currentParam->identifier);
      printSymbol(table, s);
      currentParam = currentParam->next;
    }

    break;
  }
  printf("\n");
}


/* Symbol Table Generation Functions */
SymbolTable *initSymbolTable(){
  SymbolTable *t = malloc(sizeof(SymbolTable));

  for (int i = 0; i < HashSize; i++) {
    t->table[i] = NULL;
  }
  t->parent = NULL;

  return t;
}


/* Initialize a new SymbolTable, then add all var/type/func definitions in AST (prog) */
/* Auto-Fills top-level with base type and const(true/false) definitions, returns child table */
SymbolTable *genSymbolTable(PROG *prog, int print){
  print_symbols = print;
  indentation = 0;
  SymbolTable *t = initSymbolTable();

  if (prog == NULL)
    return t;


  /* Add base type and bool const definitions to top of table */
/*
  TYPE *intType = makeType("int");
  intType->lineno = 0;
  intType->kind = intK;
  intType->spec = makeTypeSpec(intType, "int");

  TYPE *float64Type = makeType("float64");
  float64Type->lineno = 0;
  float64Type->kind = float64K;
  float64Type->spec = makeTypeSpec(float64Type, "float64");


  TYPE *boolType = makeType("bool");
  boolType->lineno = 0;
  boolType->kind = boolK;
  boolType->spec = makeTypeSpec(boolType, "bool");

  TYPE *runeType = makeType("rune");
  runeType->lineno = 0;
  runeType->kind = runeK;
  runeType->spec = makeTypeSpec(runeType, "rune");

  TYPE *stringType = makeType("string");
  stringType->lineno = 0;
  stringType->kind = stringK;
  stringType->spec = makeTypeSpec(stringType, "string");
*/

  if(print_symbols){
    PRINT_INDENT("{\n");
    indentation++;
  }
  genSymbolBaseType(t, "int", intTYPE, 0);
  genSymbolBaseType(t, "float64", float64TYPE, 0);
  genSymbolBaseType(t, "bool", boolTYPE, 0);
  genSymbolBaseType(t, "rune", runeTYPE, 0);
  genSymbolBaseType(t, "string", stringTYPE, 0);


  EXP *trueExp = malloc(sizeof(EXP));
  trueExp->lineno = 0;
  trueExp->kind = boolvalK;
  trueExp->val.boolval = 1;

  EXP *falseExp = malloc(sizeof(EXP));
  falseExp->lineno = 0;
  falseExp->kind = boolvalK;
  falseExp->val.boolval = 1;

  genSymbolConst(t, "true", boolTYPE, trueExp, 0);
  genSymbolConst(t, "false", boolTYPE, falseExp, 0);


  /* malloc a new table t2, and make it a child of t */
  SymbolTable *t2 = initSymbolTable();
  t2->parent = t;


  DEC *nextDec = prog->root_dec;

  if(print_symbols && nextDec){
    PRINT_INDENT("{\n");
    indentation++;
  }
  while (nextDec != NULL){

    switch(nextDec->kind){
    case typeDeclK:
      genSymbolsTypeDEC(t2, nextDec);
      break;
    case varDeclK:
      genSymbolsVarDEC(t2, nextDec);
      break;
    case funcDeclK:
      genSymbolsFuncDEC(t2, nextDec);
      break;
    }
    nextDec = nextDec->next;
    if(print_symbols && nextDec == NULL){
      indentation--;
      PRINT_INDENT("}\n");
    }
  }

  if(print_symbols){
    indentation--;
    printf("}\n");
  }

  return t;
}

bool isSpecialFunc(char* name){
    return (strcmp(name, "main") == 0 || strcmp(name, "init") == 0);
}



void checkRecursiveType(TYPE* t, char* prev){
    switch (t->kind) {
        case intK:
        case float64K:
        case runeK:
        case stringK:
        case boolK:
            break;

        case structK:
            {

                FIELD_DECL* fd = t->val.fieldDecls;
                while(fd != NULL){
                    if (fd->type != NULL) {
                        if (prev!=NULL) checkRecursiveType(fd->type, prev);
                        // checkRecursiveType(fd->type, t);
                    }
                    fd = fd->next;
                }
            }
            break;
        case sliceK:
            // recursive type allowed in slice
            // checkRecursiveType(t->val.sliceElement, NULL);
            break;
        case arrayK:
            if (prev != NULL) checkRecursiveType(t->val.array.element, prev);
            // type->val.array.element = genSymbolsTYPE(table, type->val.array.element);
        case inferK:	//for short declarations, when type is inferred
          break;

        case refK:
            if (prev != NULL && strcmp(t->val.name, prev) == 0) reportError("invalid recusive type", t->lineno);
            break;

    }
}





/*
  Insert a new type symbol into the symbol table and fill in its implementation (typespec)
*/
void genSymbolsTypeDEC(SymbolTable *table, DEC *typedec){
  TYPE_SPEC *nextSpec = typedec->val.typeSpecs;
  while(nextSpec != NULL){
      // init and main may only be declared as functions at the top-level scope
    if (table->parent->parent == NULL && isSpecialFunc(nextSpec->name))
        reportError("init and main may only be declared as functions at the top-level scope", nextSpec->lineno);
    if (strcmp(nextSpec->name, "_") != 0){
            SYMBOL *typeSym = putSymbol(table, nextSpec->name, typeSymK, nextSpec->lineno);
            typeSym->val.typeSpec = nextSpec;
            nextSpec->type = genSymbolsTYPE(table, nextSpec->type, false);


            // check for recursive type defination
            // printf("=================\n");
            // prettyTYPE(nextSpec->type);
            // prettyTYPE(nextSpec->type->spec->type);

            checkRecursiveType(nextSpec->type, nextSpec->name);
            if(print_symbols)
              printSymbol(table, typeSym);
        }
    else{
        nextSpec->type = genSymbolsTYPE(table, nextSpec->type, false);
    }
    // printf("typedec%d\n", typeSym->val.typeSpec->type->kind);
    // nextSpec->type->spec = nextSpec;
    nextSpec = nextSpec->next;

  }
}

/*
  Insert a new type symbol into the symbol table and fill in its implementation
  Instead of using a DEC type declaration,
*/
void genSymbolBaseType(SymbolTable *table, char *name, TYPE *type, int lineno){
  SYMBOL *typeSym = putSymbol(table, name, typeSymK, lineno);
  typeSym->val.typeSpec = type->spec;
  // typeSym->val.typeSpec->lineno = 0;
  if(print_symbols != 0)
    printSymbol(table, typeSym);
}



/*
  Insert a new const symbol into the symbol table and fill in its implementation (type and value)
*/
void genSymbolConst(SymbolTable *table, char* identifier, TYPE *type, EXP *exp, int lineno){
  SYMBOL *constSym = putSymbol(table, identifier, varSymK, lineno);
  constSym->val.varSpec = NEW(VAR_SPEC);
  constSym->val.varSpec->identifier = identifier;
  constSym->val.varSpec->type = type;
  constSym->val.varSpec->rhs = exp;
  constSym->val.varSpec->lineno = 0;
  // constSym->val.constSpec->type = genSymbolsTYPE(table, constSym->val.constSpec->type);
  if(print_symbols)
    printSymbol(table, constSym);
}


/*
  Insert a new var into the symbol table and fill in its implementation
  For each item in the VAR_SPEC (corresponding to an identifier-rhs pair), add the symbol to the
  table and set its val.
*/
void genSymbolsVarDEC(SymbolTable *table, DEC *vardec){
  VAR_SPEC *nextSpec = vardec->val.varSpecs;
  while(nextSpec != NULL){
      genSymbolsEXP(table, nextSpec->rhs);
      // init and main may only be declared as functions at the top-level scope
    if (table->parent->parent == NULL && isSpecialFunc(nextSpec->identifier))
        reportError("init and main may only be declared as functions at the top-level scope", nextSpec->lineno);

    if (strcmp(nextSpec->identifier, "_") == 0){
        // printf("44444444444\n");
        if(nextSpec->type == NULL)
          nextSpec->type = makeType("infer");
        nextSpec->type = genSymbolsTYPE(table, nextSpec->type, false);
        nextSpec = nextSpec->next;
        continue;
    }

    else{
        // prettyDEC(vardec);
        SYMBOL *varSym = putSymbol(table, nextSpec->identifier, varSymK, nextSpec->lineno);
        varSym->val.varSpec = nextSpec;
        if (nextSpec->type != NULL && nextSpec->type->kind == refK && strcmp(nextSpec->identifier, nextSpec->type->val.name) == 0)
            nextSpec->type = genSymbolsTYPE(table, nextSpec->type, true);
        else nextSpec->type = genSymbolsTYPE(table, nextSpec->type, false);
        nextSpec->shadowNum = varSym->shadowNum;
        if(varSym->val.varSpec->type == NULL)
          varSym->val.varSpec->type = makeType("infer");
        if(print_symbols)
          printSymbol(table, varSym);
        nextSpec = nextSpec->next;

    }
}
}

/*
  Like genSymbolsVarDEC. Used for short variable declarations.
  Makes sure that at least 1 variable on lhs is undeclared
*/
void genSymbolsVAR_SPEC(SymbolTable *table, VAR_SPEC *varspec){
  int num_undeclared = 0;
  VAR_SPEC *nextSpec = varspec;
  while(nextSpec != NULL){
      genSymbolsEXP(table, nextSpec->rhs);
    if(strcmp(nextSpec->identifier, "_") != 0 && !isDefSymbol(table, nextSpec->identifier)){
      num_undeclared++;
      SYMBOL *varSym = putSymbol(table, nextSpec->identifier, varSymK, nextSpec->lineno);
      varSym->val.varSpec = nextSpec;
      varSym->val.varSpec->type = makeType("infer");
      nextSpec->shadowNum = varSym->shadowNum;
      if(print_symbols)
        printSymbol(table, varSym);
    }
    else if (isDefSymbol(table, nextSpec->identifier)){
        // prettyVAR_SPEC(nextSpec);
        SYMBOL* s = getSymbol(table, nextSpec->identifier);
        // prettyVAR_SPEC(s->val.varSpec);
        nextSpec->shadowNum = s->shadowNum;
        if (s->val.varSpec->lineno == nextSpec->lineno) reportError("variable redeclared", nextSpec->lineno);
        if (s->kind != varSymK) reportError("not a variable", nextSpec->lineno);
        nextSpec->type = s->val.varSpec->type;
        nextSpec->declared = true;
    }

    nextSpec->type = genSymbolsTYPE(table, nextSpec->type, false);
    nextSpec = nextSpec->next;
  }
  if(num_undeclared < 1){
    fprintf (stderr, "Error: (line %d) At least 1 var in short declaration must be undeclared in current scope'\n",
             varspec->lineno);
    exit(1);
  }
}


/*
  Insert a new func into the symbol table and fill in its implementation
  Implementation: the FUNC_SPEC corresponding to the functions parameters and return type
  Additionally,
*/
void genSymbolsFuncDEC(SymbolTable *table, DEC *funcdec){
  if(strcmp(funcdec->val.funcSpec->name, "main") == 0){
    genSymbolsSpecialFUNCmain(table, funcdec);
    return;
  }
  else if(strcmp(funcdec->val.funcSpec->name, "init") == 0){
    genSymbolsSpecialFUNCinit(table, funcdec);
    return;
  }

  else if (strcmp(funcdec->val.funcSpec->name, "_") == 0){
    FUNC_SPEC *spec = funcdec->val.funcSpec;
      spec->returnType = genSymbolsTYPE(table, spec->returnType, false);
      SymbolTable *functionSymTable = scopeSymbolTable(table);
      genSymbolsPARAMETERS(functionSymTable, spec->parameters);
      if(spec->block->val.block){
        genSymbolsSTMT(functionSymTable, spec->block->val.block->stmt);
        genSymbolsSTMT_LIST(functionSymTable, spec->block->val.block->next);
      }
      return;
  }

    FUNC_SPEC *spec = funcdec->val.funcSpec;
  SYMBOL *funcSym = putSymbol(table, spec->name, funcSymK, spec->lineno);
  funcSym->val.funcSpec = spec;
  spec->returnType = genSymbolsTYPE(table, spec->returnType, false);
  SymbolTable *functionSymTable = scopeSymbolTable(table);
  genSymbolsPARAMETERS(functionSymTable, spec->parameters);
  if(print_symbols)
    printSymbol(functionSymTable, funcSym);
  if(spec->block->val.block){
    genSymbolsSTMT(functionSymTable, spec->block->val.block->stmt);
    genSymbolsSTMT_LIST(functionSymTable, spec->block->val.block->next);
  }
  if(print_symbols){
    indentation--;
    PRINT_INDENT("}\n");
  }
}

void genSymbolsSpecialFUNCmain(SymbolTable *table, DEC *funcdec){
  FUNC_SPEC *spec = funcdec->val.funcSpec;
  SYMBOL *funcSym = putSymbol(table, spec->name, funcSymK, spec->lineno);
  funcSym->val.funcSpec = spec;

  if(spec->returnType != NULL){
    fprintf(stderr, "Error: (line %d) Special function %s cannot have return type.\n", spec->lineno, spec->name);
    exit(1);
  }
  if(spec->parameters != NULL){
    fprintf(stderr, "Error: (line %d) Special function %s cannot have parameters.\n", spec->lineno, spec->name);
    exit(1);
  }

  SymbolTable *functionSymTable = scopeSymbolTable(table);
  genSymbolsPARAMETERS(functionSymTable, spec->parameters);
  if(print_symbols)
    printSymbol(functionSymTable, funcSym);
  if(spec->block->val.block){
    genSymbolsSTMT(functionSymTable, spec->block->val.block->stmt);
    genSymbolsSTMT_LIST(functionSymTable, spec->block->val.block->next);
  }
  if(print_symbols){
    indentation--;
    PRINT_INDENT("}\n");
  }
}

void genSymbolsSpecialFUNCinit(SymbolTable *table, DEC *funcdec){
  FUNC_SPEC *spec = funcdec->val.funcSpec;
  SymbolTable *orphan_table = initSymbolTable();
  SYMBOL *funcSym = putSymbol(orphan_table, spec->name, funcSymK, spec->lineno);

  funcSym->val.funcSpec = spec;

  if(spec->returnType != NULL){
    fprintf(stderr, "Error: (line %d) Special function %s cannot have return type.\n", spec->lineno, spec->name);
    exit(1);
  }
  if(spec->parameters != NULL){
    fprintf(stderr, "Error: (line %d) Special function %s cannot have parameters.\n", spec->lineno, spec->name);
    exit(1);
  }

  SymbolTable *functionSymTable = scopeSymbolTable(table);
  genSymbolsPARAMETERS(functionSymTable, spec->parameters);
  if(print_symbols)
    printSymbol(functionSymTable, funcSym);
  if(spec->block->val.block){
    genSymbolsSTMT(functionSymTable, spec->block->val.block->stmt);
    genSymbolsSTMT_LIST(functionSymTable, spec->block->val.block->next);
  }
  if(print_symbols){
    indentation--;
    PRINT_INDENT("}\n");
  }
}

/*
  Insert a symbol into the SymbolTable for each parameter in the list params
*/
void genSymbolsPARAMETERS(SymbolTable *table, PARAMETERS *params){
  PARAMETERS *nextParam = params;
  while(nextParam != NULL){
      if (strcmp(nextParam->identifier, "_") == 0){
        nextParam->type = genSymbolsTYPE(table, nextParam->type, true);
      }
      else{
          SYMBOL *paramSym = putSymbol(table, nextParam->identifier, varSymK, nextParam->lineno);
          nextParam->shadowNum = paramSym->shadowNum;
          nextParam->type = genSymbolsTYPE(table, nextParam->type, true);
          paramSym->val.varSpec = malloc(sizeof(VAR_SPEC));
          paramSym->val.varSpec->identifier = nextParam->identifier;
          paramSym->val.varSpec->type = nextParam->type;

      }
    nextParam = nextParam->next;
  }
}


void genSymbolsSTMT(SymbolTable *table, STMT *stmt){
  // TODO: for shortVarDecl, check condition 2 in symbol phase
  // 2. At least one variable on the left-hand side is not declared in the current scope;

  if(stmt == NULL)
    return;

  SymbolTable *scoped_table = scopeSymbolTable(table);

  switch(stmt->kind){
  case emptyK:
    break;
  case blockK:
    if(print_symbols){
      PRINT_INDENT("{\n");
      indentation++;
    }
    genSymbolsSTMT_LIST(scoped_table, stmt->val.block);
    if(print_symbols){
      indentation--;
      printf("\n");
      PRINT_INDENT("}\n");
    }
    break;
  case expressionK:
    genSymbolsEXP(table, stmt->val.exp);
    break;
  case assignK:
    genSymbolsASSIGN(table, stmt->val.assign);
    break;
  case assignopK:
    genSymbolsEXP(table, stmt->val.assignop.lhs);
    genSymbolsEXP(table, stmt->val.assignop.rhs);
    break;
  case declareK:
    switch(stmt->val.declaration->kind){
    case typeDeclK:
      genSymbolsTypeDEC(table, stmt->val.declaration);
      break;
    case varDeclK:
      genSymbolsVarDEC(table, stmt->val.declaration);
      break;
    case funcDeclK:
      genSymbolsFuncDEC(table, stmt->val.declaration);
      break;
    }
    break;
  case shortdeclareK:
    genSymbolsVAR_SPEC(table, stmt->val.shortdecvar);
    break;
  case incK:
    genSymbolsEXP(table, stmt->val.inc_exp);
    break;
  case decK:
    genSymbolsEXP(table, stmt->val.dec_exp);
    break;
  case printK:
    genSymbolsEXP_LIST(table, stmt->val.print_expressions);
    break;
  case printlnK:
    genSymbolsEXP_LIST(table, stmt->val.print_expressions);
    break;
  case returnK:
    genSymbolsEXP(table, stmt->val.ret_exp);
    break;
  case ifK:
    if(print_symbols){
      PRINT_INDENT("{\n");
      indentation++;
    }
    genSymbolsSTMT(scoped_table, stmt->val.ifstmt.optional_stmt);
    genSymbolsEXP(scoped_table, stmt->val.ifstmt.exp);
    scoped_table = scopeSymbolTable(scoped_table);
    genSymbolsSTMT(scoped_table, stmt->val.ifstmt.block);
    genSymbolsSTMT(scoped_table, stmt->val.ifstmt.optional_else);
    if(print_symbols){
      indentation--;
      printf("\n");
      PRINT_INDENT("}\n");
    }


    break;
  case elseK:
    if(print_symbols){
      PRINT_INDENT("{\n");
      indentation++;
    }
    genSymbolsSTMT(table, stmt->val.else_stmt);
    if(print_symbols){
      indentation--;
      printf("\n");
      PRINT_INDENT("}\n");
    }
    break;
  case switchK:
    if(print_symbols){
      PRINT_INDENT("{\n");
      indentation++;
    }
    genSymbolsSTMT(scoped_table, stmt->val.switchstmt.stmt);
    genSymbolsEXP(scoped_table, stmt->val.switchstmt.exp);
    genSymbolsCASE_CLAUSE_LIST(scoped_table, stmt->val.switchstmt.clauses);
    if(print_symbols){
      indentation--;
      printf("\n");
      PRINT_INDENT("}\n");
    }
    break;
  case forK:
    if(stmt->val.forstmt.while_exp){
      genSymbolsEXP(table, stmt->val.forstmt.while_exp);
      genSymbolsSTMT(table, stmt->val.forstmt.block);
    }
    else{
        if (stmt->val.forstmt.clause){
            if(print_symbols){
              PRINT_INDENT("{\n");
              indentation++;
            }
            scoped_table = genSymbolsFOR_CLAUSE(table, stmt->val.forstmt.clause);
            genSymbolsSTMT(scoped_table, stmt->val.forstmt.block);
            if(print_symbols){
              indentation--;
              printf("\n");
              PRINT_INDENT("}\n");
            }
        }
        else{
            genSymbolsSTMT(scoped_table, stmt->val.forstmt.block);
        }
    }
    break;
  case breakK:
    break;
  case continueK:
    break;
  }

}

void genSymbolsSTMT_LIST(SymbolTable *table, STMT_LIST *stmt_list){
  STMT_LIST *current = stmt_list;
  while (current != NULL)
    {
      genSymbolsSTMT(table, current->stmt);
      current = current->next;
    }
}


void genSymbolsASSIGN(SymbolTable *table, ASSIGN *assign_stmt){
  genSymbolsEXP(table, assign_stmt->lhs);
  genSymbolsEXP(table, assign_stmt->rhs);
  if(assign_stmt->next)
    genSymbolsASSIGN(table, assign_stmt->next);
}


// A new scope needs to be opened for each case statement as well as the default statement
void genSymbolsCASE_CLAUSE(SymbolTable *table, CASE_CLAUSE *case_clause){
  if (case_clause != NULL)
    {
      if(print_symbols){
        PRINT_INDENT("{\n");
        indentation++;
      }

      SymbolTable *new_st;

      switch(case_clause->kind)
        {
        case caseK:
          genSymbolsEXP_LIST(table, case_clause->val.case_clause.cases);
          new_st = scopeSymbolTable(table);
          genSymbolsSTMT_LIST(new_st, case_clause->val.case_clause.clause);
          break;
        case defaultK:
          new_st = scopeSymbolTable(table);
          genSymbolsSTMT_LIST(new_st, case_clause->val.default_clause);
          break;
        }

      if(print_symbols){
        indentation--;
        printf("\n");
        PRINT_INDENT("}\n");
      }
    }
}

void genSymbolsCASE_CLAUSE_LIST(SymbolTable *table, CASE_CLAUSE_LIST *case_clauses){
  CASE_CLAUSE_LIST *current = case_clauses;

  while (current != NULL)
    {
      genSymbolsCASE_CLAUSE(table, current->clause);
      current = current->next;
    }
}

// A new symbol table needs to be generated here because the init statement
// 	can shadow variables declared in the same scope as the for statement.
//
// This function also must return the new symbol table so that the block that follows
// 	can use any new variables.
SymbolTable *genSymbolsFOR_CLAUSE(SymbolTable *table, FOR_CLAUSE *for_clause){
  SymbolTable *new_st = scopeSymbolTable(table);
  genSymbolsSTMT(new_st, for_clause->init);
  genSymbolsEXP(new_st, for_clause->cond);
  genSymbolsSTMT(new_st, for_clause->post);

  return new_st;
}

void genSymbolsEXP(SymbolTable *table, EXP *exp){
  if (exp != NULL)
    {
      switch (exp->kind)
        {
          // TODO: What to do with the bool shadowed and SYMBOL s variables?
        case identifierK:
          if (strcmp(exp->val.identifier.identifier, "_") == 0) return;
          if (getSymbol(table, exp->val.identifier.identifier) == NULL)
            {
              fprintf(stderr, "Error: Variable %s on line %d has not been declared.\n", exp->val.identifier.identifier, exp->lineno);
              exit(1);
            }
          else{
              SYMBOL* idenSym = getSymbol(table, exp->val.identifier.identifier);

              if (idenSym->kind != varSymK) {
                  // printf("%d\n", idenSym->kind);
                  // printf("%s\n", idenSym->identifier);
                  reportError("not variable", exp->lineno);
              }
              exp->val.identifier.s = idenSym;
              exp->type = idenSym->val.varSpec->type;
              exp->val.identifier.shadowed = idenSym->shadowNum > 0;
              exp->val.identifier.shadowNum = idenSym->shadowNum;

              // printf("fsdfasfd\n");
              // printf("\n");
              // prettyTYPE(exp->type);
              // if (idenSym->val.varSpec->type == NULL) printf("safsadf\n");
              // if (idenSym->val.varSpec->type != NULL) printf("123\n");
          }
          break;

          // Binary operators
        case orK:
        case andK:
        case eqK:
        case neqK:
        case ltK:
        case gtK:
        case leqK:
        case geqK:
        case plusK:
        case minusK:
        case bitorK:
        case xorK:
        case multK:
        case divideK:
        case modK:
        case lshiftK:
        case rshiftK:
        case bitandK:
        case bitclearK:
          genSymbolsEXP(table, exp->val.binary.lhs);
          genSymbolsEXP(table, exp->val.binary.rhs);
          break;
          // Unary operators
        case uminusK:
        case uplusK:
        case ubangK:
        case uxorK:
          genSymbolsEXP(table, exp->val.unary.child);
          break;
          // Other expressions

          /* Determine if func or cast */
        case funcK:
          if(exp->val.func.func_exp->val.identifier.identifier != NULL){
            SYMBOL *funcCastSym = getSymbol(table, exp->val.func.func_exp->val.identifier.identifier);

            if(funcCastSym == NULL){
              fprintf(stderr, "Error: Func or Cast %s on line %d has not been declared.\n", exp->val.identifier.identifier, exp->lineno);
              exit(1);
            }
            switch(funcCastSym->kind){
            case typeSymK:
               // printf("cast %s\n", exp->val.func.func_exp->val.identifier.identifier);
              exp->kind = castK;

              // exp->type = funcCastSym->val.typeSpec->type;
              exp->type = makeType(exp->val.func.func_exp->val.identifier.identifier);
              exp->type->spec = funcCastSym->val.typeSpec;
              if (exp->val.func.args == NULL) reportError("conversion expects 1 argument", exp->lineno);
              if (exp->val.func.args->next != NULL) reportError("conversion expects 1 argument", exp->lineno);
              exp->val.cast.exp = exp->val.func.args->exp;
              exp->val.cast.type = exp->type;

              exp->type = genSymbolsTYPE(table, exp->val.cast.type, false);
              exp->val.cast.type = genSymbolsTYPE(table, exp->val.cast.type, false);
              genSymbolsEXP(table, exp->val.cast.exp);

              break;
            case funcSymK:
              /* printf("func\n"); */
              exp->kind = funcK;
              /* exp->type = exp->val.func.func_exp->type; */
              exp->val.func.func_spec = funcCastSym->val.funcSpec;
              exp->type = funcCastSym->val.funcSpec->returnType;
              // genSymbolsEXP(table, exp->val.func.func_exp);
              genSymbolsEXP_LIST(table, exp->val.func.args);
              break;
            default:
                reportError("not a function", exp->lineno);
            }
          }
          break;
        case castK:
          exp->val.cast.type = genSymbolsTYPE(table, exp->val.cast.type, false);
          genSymbolsEXP(table, exp->val.cast.exp);
          break;
        case appendK:
          genSymbolsEXP(table, exp->val.append.head);
          genSymbolsEXP(table, exp->val.append.tail);
          break;
        case arrayindexK:
          genSymbolsEXP(table, exp->val.array_index.array_exp);
          genSymbolsEXP(table, exp->val.array_index.index);
          break;
        case sliceindexK:
          genSymbolsEXP(table, exp->val.slice_index.slice_exp);
          genSymbolsEXP(table, exp->val.slice_index.index);
          break;
        case structaccessK:
          genSymbolsEXP(table, exp->val.struct_access.struct_exp);
          break;
          //Default should catch stringval, runeval, intval, float64val and boolval
        default:
          break;
        }
    }
}

void genSymbolsEXP_LIST(SymbolTable *table, EXP_LIST *exp_list){
  EXP_LIST *current = exp_list;

  while (current != NULL)
    {
      genSymbolsEXP(table, current->exp);
      current = current->next;
    }
}



void printTypeSym(SYMBOL* s){
    TYPE* t = s->val.typeSpec->type;
    printf("Kind: %d\n", s->val.typeSpec->type->kind);
    while (t!=NULL && t->kind == refK){
        prettyTYPE(t);
        printf("K: %d\n",t->kind);
        t = t->spec->type;
    }
    if (t== NULL) printf("\nNULLLLLLL\n");
    else{
        printf("K: %d\n",t->kind);
    }

}
TYPE* genSymbolsTYPE(SymbolTable *table, TYPE *type, bool allowSearchInParent) {
  if (type != NULL)
    {
      switch(type->kind)
        {
        case intK:
        case float64K:
        case runeK:
        case stringK:
        case boolK:
            break;

        case structK:
            {
                SymbolTable* structSymTable = scopeSymbolTable(table);
                genSymbolsFIELD_DECL(structSymTable, type->val.fieldDecls);
            }
            break;
        case sliceK:
            type->val.sliceElement = genSymbolsTYPE(table, type->val.sliceElement, false);
        case arrayK:
            type->val.array.element = genSymbolsTYPE(table, type->val.array.element, false);
        case inferK:	//for short declarations, when type is inferred
          break;

        case refK:
            {
                SYMBOL* typeSym = getSymbol(table, type->val.name);
                if (typeSym == NULL)
                {
                    fprintf(stderr, "Error: Type %s on line %d has not been declared.\n", type->val.name, type->lineno);
                    exit(1);
                }

                // TODO: bug in struct scoping. this fix is ugly
                if (typeSym->kind == fieldSymK) {
                    return genSymbolsTYPE(table->parent, type, false);
                }

                if (typeSym->kind != typeSymK) {
                    if (allowSearchInParent) return genSymbolsTYPE(table->parent, type, false);
                    reportError("not a type", type->lineno);
                }
                // printf("fsdfdsagd1232f\n");
                // prettyTYPE(type);


                // if (str)
                type->spec = typeSym->val.typeSpec;

                if (strcmp(type->val.name, "int") == 0 && type->spec->type == intTYPE) return intTYPE;
                if (strcmp(type->val.name, "float64") == 0 && type->spec->type == float64TYPE) return float64TYPE;
                if (strcmp(type->val.name, "string") == 0 && type->spec->type == stringTYPE) return stringTYPE;
                if (strcmp(type->val.name, "rune") == 0 && type->spec->type == runeTYPE) return runeTYPE;
                if (strcmp(type->val.name, "bool") == 0 && type->spec->type == boolTYPE) return boolTYPE;

                // printf("SYMBOL %d\n", typeSym->val.typeSpec->type->kind);
                // prettyTYPE_SPEC(typeSym->val.typeSpec);
                // printTypeSym(typeSym);
            }
        }
    }
    return type;
}





void genSymbolsFIELD_DECL(SymbolTable *table, FIELD_DECL* fd){
    if (fd != NULL){
        fd->type = genSymbolsTYPE(table, fd->type, false);
        if (strcmp(fd->identifier, "_") != 0){
            SYMBOL *fieldDeclSym = putSymbol(table, fd->identifier, fieldSymK, fd->lineno);
            fieldDeclSym->val.structSpec = fd;
        }
        // prettyTYPE(fd->type);
        // printf("%d\n", fd->type->kind);
        // FIXME: fd->
        genSymbolsFIELD_DECL(table, fd->next);
    }
}
