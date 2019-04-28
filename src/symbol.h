#ifndef SYMBOL_H
#define SYMBOL_H

#include "tree.h"

#define HashSize 317

typedef struct SymbolTable SymbolTable;

typedef struct SymbolTable {
  SYMBOL *table[HashSize];
  SymbolTable *parent;
} SymbolTable;


/* Symbol Functions & Helper Functions */
int hash(char *str);
SymbolTable *scopeSymbolTable(SymbolTable *table);

SYMBOL *putSymbol(SymbolTable *table, char *identifier, SYMBOL_Kind kind, int lineno);

SYMBOL *getSymbol(SymbolTable *table, char *identifier);
int defSymbol(SymbolTable *table, char *identifier);

void printType(TYPE *type);
void printPARAMETERS(PARAMETERS *params);
void printSymbol(SymbolTable *table, SYMBOL *symbol);

/* Symbol Table Generation Functions */
SymbolTable *initSymbolTable();
SymbolTable *genSymbolTable(PROG *prog, int print);


void genSymbolConst(SymbolTable *table, char* identifier, TYPE *type, EXP *exp, int lineno);
void genSymbolBaseType(SymbolTable *table, char *name, TYPE *type, int lineno);

void genSymbolsTypeDEC(SymbolTable *table, DEC *typedec);

void genSymbolsVarDEC(SymbolTable *table, DEC *vardec);

void genSymbolsFuncDEC(SymbolTable *table, DEC *funcdec);
void genSymbolsSpecialFUNCinit(SymbolTable *table, DEC *funcdec);
void genSymbolsSpecialFUNCmain(SymbolTable *table, DEC *funcdec);

void genSymbolsPARAMETERS(SymbolTable *table, PARAMETERS *params);

void genSymbolsSTMT(SymbolTable *table, STMT *stmt);
void genSymbolsSTMT_LIST(SymbolTable *table, STMT_LIST *stmt_list);
void genSymbolsASSIGN(SymbolTable *table, ASSIGN *assign_stmt);

void genSymbolsCASE_CLAUSE(SymbolTable *table, CASE_CLAUSE *case_clause);
SymbolTable *genSymbolsFOR_CLAUSE(SymbolTable *table, FOR_CLAUSE *for_clause);
void genSymbolsCASE_CLAUSE_LIST(SymbolTable *table, CASE_CLAUSE_LIST *case_clauses);

void genSymbolsEXP(SymbolTable *table, EXP *exp);
void genSymbolsEXP_LIST(SymbolTable *table, EXP_LIST *exp_list);

TYPE* genSymbolsTYPE(SymbolTable *table, TYPE *type, bool parameter);

void genSymbolsFIELD_DECL(SymbolTable *table, FIELD_DECL* fd);


#endif
