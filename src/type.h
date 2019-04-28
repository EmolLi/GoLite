#ifndef TYPE_H
#define TYPE_H

#include "tree.h"
#include "symbol.h"


void typeCheckPROG(PROG *p);
void initTypes();
TYPE* resolveType(TYPE* t);
bool isEqualType(TYPE* t1, TYPE* t2);
bool resolveToNumbericBaseType(TYPE *t);
bool resolveToIntegerBaseType(TYPE* t);
void typeCheckDEC(DEC* dec);
void typeCheckVarDEC(VAR_SPEC *vs);
void typeCheckTypeDEC(TYPE_SPEC *ts);
void typeCheckFuncDEC(FUNC_SPEC *fs);
void typeCheckSTMT_LIST(STMT_LIST *sl, TYPE *returnType);
void typeCheckSTMT(STMT *s, TYPE *returnType);
void typeCheckEXP_LIST(EXP_LIST *l);
void typeCheckEXP(EXP *exp);
TYPE* typeCheckEXPBinaryOp(EXP* e);
TYPE* typeCheckBinaryOp(EXP *lhs, EXP *rhs, EXP_Kind op);
void typeCheckPrint(STMT* s);
void typeCheckInc(STMT* s);
void typeCheckCASE_CLAUSE_LIST(CASE_CLAUSE_LIST* c, TYPE* eType, TYPE* returnType);
void checkIsLValue(EXP *e);
void typeCheckASSIGN(ASSIGN* a);
bool hasForClause(STMT* s);
#endif
