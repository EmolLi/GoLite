#ifndef PRETTY_H
#define PRETTY_H

#include "tree.h"

void prettyPROG(PROG *p);
void prettyPACKAGE(PACKAGE *package);
void prettyDEC(DEC *d);
void prettyVAR_SPEC(VAR_SPEC *vs);
void prettyPARAMETERS(PARAMETERS *params);
void prettyTYPE_SPEC(TYPE_SPEC *spec);
void prettySTMT(STMT *s);
void prettySTMT_LIST(STMT_LIST *s_list);
void prettyFOR_CLAUSE(FOR_CLAUSE *clause);
void prettyCASE_CLAUSE(CASE_CLAUSE *clause);
void prettyCASE_CLAUSE_LIST(CASE_CLAUSE_LIST *clauses);
void prettyEXP_LIST(EXP_LIST *e_list);
void prettyEXP(EXP *e);
void prettyTYPE(TYPE *t);
void prettyIDENTIFIER_LIST(IDENTIFIER_LIST *id_list);
void prettyFIELD_DECL_LIST(FIELD_DECL *list);

#endif /* !PRETTY_H */
