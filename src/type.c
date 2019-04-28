#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "comm.h"
#include "tree.h"

#include "type.h"
#include "weed.h"
#include "symbol.h"
#include "pretty.h"


/**  TODO:Constants belong to the variable category (can be used in expressions), but cannot be
assigned without first being shadowed.
**/


void typeCheckPROG(PROG *p){
    if (p!= NULL){
        initTypes();
        typeCheckDEC(p->root_dec);
    }
}

/*
void initTypes()
{
    runeTYPE = NEW(TYPE);
    runeTYPE->kind = runeK;
    intTYPE = NEW(TYPE);
    intTYPE->kind = intK;
    float64TYPE = NEW(TYPE);
    float64TYPE->kind = float64K;
    boolTYPE = NEW(TYPE);
    stringTYPE = NEW(TYPE);
    boolTYPE->kind = boolK;
}
*/


// ================== helper functions ==============
/*
bool isSpecialFunc(char* name){
    return (strcmp(name, "main") == 0 || strcmp(name, "init") == 0);
}*/
// resolve defined type, specification 1.2.1
TYPE* resolveType(TYPE* t){/*
    if (t == NULL){
        printf("fsafdagnull\n");
    }*/
    if (t->kind  == intK ||
        t->kind  == float64K ||
        t->kind  == runeK ||
        t->kind  == stringK ||
        t->kind  == boolK ||
        t->kind  == arrayK ||
        t->kind  == sliceK ||
        t->kind == structK) {
            return t;
        }
/*
    else{
        printf("========2222222200\n");
        printf("%d vs %d\n", t->kind, stringK);
        prettyTYPE_SPEC(t->spec);
        printf("====123456677\n");
        // prettyTYPE(t->spec->type);
        return t;
    }*/
    // defined type RT(T, t) = RT(T, t') given "type t t'""
    return resolveType(t->spec->type);
}


// compare t1 and t2, if equal return 1, else return 0
bool isEqualType(TYPE* t1, TYPE* t2){
  /* printf("t1: %d", t1->kind); */
  /* printf("t2: %d", t2); */
    if (t1 == NULL || t2 == NULL){
        reportError("at least one exp has no return value", t1 == NULL ? t2->lineno : t1->lineno);
    }


    if (t1->kind == t2->kind){
        // Two array types are identical if identical element types and the same array length
        if (t1->kind == arrayK){
            if ((t1->val.array.size != t2->val.array.size) ||
                (!isEqualType(t1->val.array.element, t2->val.array.element))){
                    return false;
                }
        }

        if (t1->kind == sliceK){
            // Two slice types are identical if they have identical element types.
            if (!isEqualType(t1->val.sliceElement, t2->val.sliceElement)){
                return false;
            }
        }

        if (t1->kind  == structK){
            // Two struct types are identical if they have the same sequence of fields, and if corresponding fields have the same names, and identical types, and identical tags.
            FIELD_DECL* f1 = t1->val.fieldDecls;
            FIELD_DECL* f2 = t2->val.fieldDecls;
            while (f1 != NULL && f2 != NULL){
                if (!isEqualType(f1->type, f2->type)) return false;
                if (strcmp(f1->identifier, f2->identifier) != 0) return false;
                f1 = f1->next;
                f2 = f2->next;
            }
            if (f1 != NULL || f2 != NULL){
                // fieldDecls length not equal
                return false;
            }
        }


        if (t1->kind == refK){
            // identical if they point to the same type specification
            if (t1->spec != t2->spec) return false;
        }

        return true;
    }


    return false;
}



bool isComparable(TYPE* ot){
    TYPE* t = resolveType(ot);
    if (t->kind == boolK ||
        t->kind == intK ||
        t->kind == runeK ||
        t->kind == float64K ||
        t->kind == stringK) return true;

    if (t->kind == structK){
        FIELD_DECL* f = t->val.fieldDecls;
        while(f!= NULL){
            if (!isComparable(f->type)) return false;
            f = f->next;
        }
        return true;
    }

    if (t->kind == arrayK){
        return isComparable(t->val.array.element);
    }
    return false;
}




bool isOrdered(TYPE* t){
    if (t->kind == intK ||
        t->kind == runeK ||
        t->kind == float64K ||
        t->kind == stringK){
            return true;
        }
    return false;
}

bool resolveToBaseType(TYPE *t){
    TYPE* rt = resolveType(t);
    if (rt->kind == intK ||
        rt->kind == float64K ||
        rt->kind == runeK ||
        rt->kind == stringK ||
        rt->kind == boolK) return true;
    return false;
}

bool resolveToNumbericBaseType(TYPE *t){
    TYPE* rt = resolveType(t);
    if (rt->kind == intK ||
        rt->kind == float64K ||
        rt->kind == runeK) return true;
    return false;
}

bool resolveToIntegerBaseType(TYPE* t){
    TYPE* rt = resolveType(t);
    if (rt->kind == intK ||
        rt->kind == runeK) return true;
    return false;
}

bool isConstantVariable(char* name){
    if (strcmp(name, "true") == 0 ||
        strcmp(name, "false") == 0 ||
        strcmp(name, "int") == 0 ||
        strcmp(name, "float64") == 0 ||
        strcmp(name, "rune") == 0 ||
        strcmp(name, "bool") == 0 ||
        strcmp(name, "string") == 0){
            return true;
        }
    return false;
}


EXP_Kind getOp(OP_ASSIGN o){
  switch (o) {
  case pluseq:
    return plusK;
  case minuseq:
    return minusK;
  case bitoreq:
    return bitorK;
  case xoreq:
    return xorK;
  case multeq:
    return multK;
  case diveq:
    return divideK;
  case modeq:
    return modK;
  case lshifteq:
    return lshiftK;
  case rshifteq:
    return rshiftK;
  case bitandeq:
    return bitandK;
  case bitcleareq:
    return bitclearK;
  default:
    fprintf(stderr, "Error: Unrecognized assignment operator.\n");
    exit(1);
  }
}





// =============== type checks =========================
void typeCheckDEC(DEC* dec){
    if (dec != NULL){
        switch (dec->kind) {
            case typeDeclK:
                typeCheckTypeDEC(dec->val.typeSpecs);
                break;
            case funcDeclK:
                typeCheckFuncDEC(dec->val.funcSpec);
                break;
            case varDeclK:
                typeCheckVarDEC(dec->val.varSpecs);
                break;
        }
        typeCheckDEC(dec->next);
    }
}

void typeCheckVarDEC(VAR_SPEC *vs){
    if (vs != NULL){

        // prettyVAR_SPEC(vs);
        // prettyTYPE(vs->type);

        // var x T
        if (vs->type->kind != inferK && vs->rhs == NULL){
            // printf("55555\n");
            return;
        }

        // var x T = expr
        // If expr is well-typed and its type is T1, and T1=T
        else if (vs->type->kind != inferK && vs->rhs!=NULL){
            // printf("666666666\n");
            typeCheckEXP(vs->rhs);
            // prettyTYPE(vs->rhs->type);
            // prettyTYPE(vs->type);
            // printf("%d\n", vs->rhs->type->kind);
            // printf("%d\n", vs->type->kind);

            if (!isEqualType(vs->type, vs->rhs->type)) {
                // prettyTYPE(vs->type);
                // prettyVAR_SPEC(vs);
                // prettyTYPE(vs->type);
                // printf("%d\n", vs->type->kind);
                // printf("55555555555\n");
               reportError("illegal assignment",vs->lineno);
            }
        }

        // var x = expr
        // If expr is well-typed and its type is T
        else if (vs->type->kind == inferK && vs->rhs != NULL){
            // printf("444444444444\n");
            typeCheckEXP(vs->rhs);
            // prettyTYPE(vs->rhs->type);
            if (vs->rhs->type == NULL) reportError("righ hand side does not return value", vs->rhs->lineno);
            vs->type = vs->rhs->type;
        }

        // printf("888888888\n");
        typeCheckVarDEC(vs->next);
        // TODO: redeclaration is already checked in symbol phase
    }
}

void typeCheckTypeDEC(TYPE_SPEC *ts){
    // type T1 T2
    // TODO: redeclaration already checked in symbol phase
    return;
}





void typeCheckFuncDEC(FUNC_SPEC *fs){
    if (fs!= NULL){
        // special func
        /*
        if (isSpecialFunc(fs->name)){
            if (fs->parameters != NULL || fs->returnType != NULL){
                reportError("special function cannot have parameters or return value",fs->lineno);
            }
            // printf("%s\n", fs->name);
        }*/

        if(fs->returnType != NULL && !weedStatementTerminate(fs->block))
            reportError("does not terminate", fs->lineno);


        // same parameter name & parameter redeclaration checked in symbol phase
        // printf("func dec%d\n", fs->returnType->kind);
        typeCheckSTMT(fs->block, fs->returnType);
    }
}





void typeCheckSTMT_LIST(STMT_LIST *sl, TYPE *returnType){
    if (sl!= NULL){
        // printf("%s\n", sl->stmt->kind);
        typeCheckSTMT(sl->stmt, returnType);
        typeCheckSTMT_LIST(sl->next, returnType);
    }
}

bool hasForClause(STMT* s){
    return s->val.forstmt.clause != NULL && (s->val.forstmt.clause->cond != NULL ||s->val.forstmt.clause->init != NULL || s->val.forstmt.clause->post != NULL);
}

void typeCheckSTMT(STMT *s, TYPE *returnType){
    VAR_SPEC *curVS;
    if (s != NULL){
        // if (s->kind){
            // printf("7\n");
         // }
        // else{
            // printf("anc\n");
        // }
        switch (s->kind) {
            case emptyK:
                break;
            case blockK:
                typeCheckSTMT_LIST(s->val.block, returnType);
                // weedStatementList(s->val.block, label);
                break;
            case expressionK:
                if (s->val.exp->kind != funcK) reportError("expression statement must be function call", s->lineno);
                typeCheckEXP(s->val.exp);
                break;
            case assignK:
                // FIXME: address operators??
                typeCheckASSIGN(s->val.assign);
                break;
            case assignopK:
                typeCheckEXP(s->val.assignop.lhs);
                typeCheckEXP(s->val.assignop.rhs);
                if (!isEqualType(s->val.assignop.lhs->type, typeCheckBinaryOp(s->val.assignop.lhs, s->val.assignop.rhs, getOp(s->val.assignop.assign_type)))){
                    reportError("illegal assigment", s->lineno);
                }
                checkIsLValue(s->val.assignop.lhs);
                // weedEXP(s->val.assignop.rhs, 0);
                break;


            case shortdeclareK:
                // 1. All the expressions on the right-hand side are well-typed;
                // typeCheckEXP_LIST(s->val.shortdecvar);
                // 2. already checked in symbol phase
                // 3. The variables already declared in the current scope are assigned expressions of the
                // same type.
                // printf("555555555\n");
                curVS = s->val.shortdecvar;
                while (curVS != NULL){
                    typeCheckEXP(curVS->rhs);
                    // prettyTYPE(curVS->type);
                    // prettyTYPE(curVS->rhs->type);
                    // printf("%d\n", curVS->rhs->type->kind);
                    if (curVS->type != NULL && curVS->type->kind != inferK) {
                        // printf("fffffff\n");
                        // prettyTYPE(curVS->type);
                        // prettyTYPE(curVS->rhs->type);
                        if (!isEqualType(curVS->type, curVS->rhs->type)){
                            reportError("illegal assignment", curVS->lineno);
                        }
                    }
                    else {
                        // printf("zzzzzzzzzzzzz000\n");
                        // cur -> type == null
                        if (curVS->rhs->type == NULL){
                            reportError("right handside does not return value", curVS->rhs->lineno);
                        }
                        curVS->type = curVS->rhs->type;
                        // prettyTYPE(curVS->type);
                    }
                    curVS = curVS->next;
                }

                break;


            case declareK:
                typeCheckDEC(s->val.declaration);
                break;

            case incK:
            case decK:
                typeCheckInc(s);
                break;

            case printK:
            case printlnK:
                typeCheckPrint(s);
                break;


            case returnK:
                if (s->val.ret_exp!=NULL) {
                   if (returnType == NULL){
                       reportError("return value not allowed",s->lineno);
                   }


                   typeCheckEXP(s->val.ret_exp);
                   // printf("%d\n", s->val.ret_exp->type->kind);
                   // printf("%d\n", returnType->kind);

                   if (!isEqualType(s->val.ret_exp->type, returnType)){
                       reportError("illegal type of expression",s->lineno);
                   }
                }

                if (s->val.ret_exp == NULL && returnType != NULL) {
                   reportError("return value expected",s->lineno);
                }
                break;



            case ifK:
              typeCheckSTMT(s->val.ifstmt.optional_stmt, returnType);
                if (s->val.ifstmt.exp != NULL){
                    typeCheckEXP(s->val.ifstmt.exp);
                    if (resolveType(s->val.ifstmt.exp->type)->kind != boolK){
                        reportError("exp doesn't resolve to bool",s->lineno);
                    }
                }
                typeCheckSTMT(s->val.ifstmt.block, returnType);
                typeCheckSTMT(s->val.ifstmt.optional_else, returnType);
                break;

            case elseK:
              typeCheckSTMT(s->val.else_stmt, returnType);
                break;


            case switchK:
              typeCheckSTMT(s->val.switchstmt.stmt, returnType);
                if (s->val.switchstmt.exp != NULL){
                    typeCheckEXP(s->val.switchstmt.exp);
                    if (s->val.switchstmt.exp->type == NULL) reportError("invalid switch expression", s->val.switchstmt.exp->lineno);
                    // prettyTYPE(s->val.switchstmt.exp->type);
                    if(!isComparable(s->val.switchstmt.exp->type)) reportError("switch expr should be comparable", s->val.switchstmt.exp->lineno);
                    typeCheckCASE_CLAUSE_LIST(s->val.switchstmt.clauses, s->val.switchstmt.exp->type, returnType);

                }

                else{
                    typeCheckCASE_CLAUSE_LIST(s->val.switchstmt.clauses, boolTYPE, returnType);
                }

                // FIXME: The statements under the different alternatives type check.?????

                break;


            case forK:
                if (s->val.forstmt.while_exp != NULL && !hasForClause(s)){
                    typeCheckEXP(s->val.forstmt.while_exp);
                    if (resolveType(s->val.forstmt.while_exp->type)->kind != boolK){
                        reportError("while exp not resolve to bool",s->lineno);
                    }
                }
                else if (s->val.forstmt.while_exp == NULL && hasForClause(s)){
                    // prettyFOR_CLAUSE(s->val.forstmt.clause);
                    typeCheckSTMT(s->val.forstmt.clause->init, NULL);
                    // printf("4444444444\n");
                    typeCheckEXP(s->val.forstmt.clause->cond);
                    // prettyTYPE(s->val.forstmt.clause->cond->type);
                    if (s->val.forstmt.clause->cond!=NULL && resolveType(s->val.forstmt.clause->cond->type)->kind != boolK){
                        reportError("expr not resolve to bool",s->lineno);
                    }
                    typeCheckSTMT(s->val.forstmt.clause->post, NULL);
                 }
                 // printf("5555555555555\n");

                 // prettySTMT(s);
                 // if (s->val.forstmt.while_exp == NULL){
                     // printf("6666666666666\n");
                 // }
                 // prettySTMT(s);
                 // if (s->val.forstmt.clause == NULL) printf("8888888\n");
                 // if (hasForClause(s)) printf("77777777\n");
                typeCheckSTMT(s->val.forstmt.block, returnType);
                break;


            case breakK:
                break;

            case continueK:
                break;

            default:
                // FIXME
                // printf("fsdfsd\n");
                exit(1);
            }

    }
}


void typeCheckInc(STMT* s){
  EXP* e = s->kind == incK ? s->val.inc_exp : s->val.dec_exp;
  typeCheckEXP(e);
  // prettyTYPE(e->type);
  if (!resolveToNumbericBaseType(e->type)){
    reportError("cannot resolve to numeric base type", s->lineno);
}
    // prettyEXP(e);
    // printf("%d vs %d\n", e->kind, identifierK);
    checkIsLValue(e);

      // break;
  // checkIsLValue(e);
}


void typeCheckCASE_CLAUSE_LIST(CASE_CLAUSE_LIST* c, TYPE* eType, TYPE* returnType){
    if (c!= NULL){
        if (c->clause->kind == caseK){
            typeCheckEXP_LIST(c->clause->val.case_clause.cases);
            EXP_LIST* cur = c->clause->val.case_clause.cases;
            while (cur != NULL){
                // printf("111111111111100\ncurExp: \n");
                // prettyTYPE(cur->exp->type);
                // printf("\n2222222222\n");
                // prettyTYPE(eType);
                if (cur->exp->type == NULL) reportError("invalid case expression", cur->exp->lineno);
              if (!isEqualType(cur->exp->type, eType)){
                  reportError("switch case type incorrect", cur->exp->lineno);
              }
                cur = cur->next;
            }
            typeCheckSTMT_LIST(c->clause->val.case_clause.clause, returnType);
        }
        else{
            typeCheckSTMT_LIST(c->clause->val.default_clause, returnType);
        }

        typeCheckCASE_CLAUSE_LIST(c->next, eType, returnType);
    }
}


void checkIsLValue(EXP *e){
    switch (e->kind) {
        case identifierK:
            if (isConstantVariable(e->val.identifier.identifier)){
                if (!e->val.identifier.shadowed){
                    reportError("unshadowed constant is not lvalue", e->lineno);
                }
            }
            break;
        case arrayindexK:
            // prettyEXP(e);
            // prettyTYPE(e->type);
            checkIsLValue(e->val.array_index.array_exp);
            break;
            /*if ((e->val.array_index.array_exp->type->kind == arrayK) && (e->val.array_index.index->type->kind == intK))
                break;
            reportError("not lvalue", e->lineno);*/
        case sliceindexK:
            if ((e->val.slice_index.slice_exp->type->kind == sliceK) && (e->val.slice_index.index->type->kind == intK))
                break;
            reportError("not lvalue", e->lineno);

        case structaccessK:
            checkIsLValue(e->val.struct_access.struct_exp);
            break;

        default:
            reportError("not lvalue", e->lineno);
    }

}


void typeCheckASSIGN(ASSIGN* a){
    if (a != NULL){
        typeCheckEXP(a->lhs);
        // prettyEXP(a->lhs);
        // prettyEXP(a->rhs);
        typeCheckEXP(a->rhs);
        // prettyTYPE(a->lhs->type);
        // printf("%d\n", a->lhs->type->kind);
        // prettyTYPE(a->rhs->type);
        // printf("%d\n", a->rhs->type->kind);
        if (a->lhs->type!=NULL && !isEqualType(a->lhs->type, a->rhs->type)){
            reportError("illegal assignment", a->lhs->lineno);
        }

        if (a->lhs->type != NULL) checkIsLValue(a->lhs);
        typeCheckASSIGN(a->next);
    }
}




void typeCheckPrint(STMT* s){
    typeCheckEXP_LIST(s->val.print_expressions);
    EXP_LIST* cur = s->val.print_expressions;
    while (cur != NULL){
        if (cur->exp->type == NULL) reportError("print exp does not return value.", cur->exp->lineno);
        if (!resolveToBaseType(cur->exp->type))
            // printf("%s\n", );
            reportError("does not resolve to base type", cur->exp->lineno);
        cur = cur->next;
    }
}


void typeCheckEXP_LIST(EXP_LIST* l){
    if (l != NULL){
        typeCheckEXP(l->exp);
        typeCheckEXP_LIST(l->next);
    }
}

void typeCheckEXP(EXP *exp){
    // implement type
    //exp->type =...
    if (exp != NULL){
        switch (exp->kind) {
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
                // prettyEXP(exp);
                // printf("%d vs %d vs %d\n", exp->kind, andK, plusK);
                // printf("%d vs %d vs %d\n", andK, exp->kind, plusK);
                exp->type = typeCheckEXPBinaryOp(exp);

                break;

            case uminusK:
                typeCheckEXP(exp->val.unary.child);
                exp->type = exp->val.unary.child->type;
                if (!resolveToNumbericBaseType(exp->type)){
                        reportError("cannot resolve to numeric base type", exp->lineno);
                    }
                break;

            case uplusK:
                typeCheckEXP(exp->val.unary.child);
                exp->type = exp->val.unary.child->type;
                if (!resolveToNumbericBaseType(exp->type)){
                        reportError("cannot resolve to numeric base type", exp->lineno);
                    }
                break;

            case ubangK:
                // prettyEXP(exp);
                typeCheckEXP(exp->val.unary.child);
                exp->type = exp->val.unary.child->type;
                TYPE* rt = resolveType(exp->type);
                if (rt->kind != boolK){
                        reportError("cannot resolve to bool type", exp->lineno);
                    }
                break;

            case uxorK:
                typeCheckEXP(exp->val.unary.child);
                exp->type = exp->val.unary.child->type;
                if (!resolveToIntegerBaseType(exp->type)){
                        reportError("cannot resolve to integer type", exp->lineno);
                    }
                break;

            case identifierK:
                // prettyEXP(exp);
                // type already implemented in symbol phase
                // exp->type = typeVar(exp->val.identifier.s);
                if (strcmp(exp->val.identifier.identifier, "_") == 0) break;
                if (exp->type->kind == inferK) {
                    exp->type = exp->val.identifier.s->val.varSpec->type;
                }
                // prettyEXP(exp);
                // printf(":    \n");
                // prettyTYPE(exp->type);
                break;

            case float64valK:
            case runevalK:
            case intvalK:
            case stringvalK:
            case boolvalK:
                break;

            case funcK:
              // printf("FUNC\n");

                if (exp->val.func.func_exp->kind != identifierK) reportError("Invalid function call", exp->val.func.func_exp->lineno);
                if (strcmp(exp->val.func.func_exp->val.identifier.identifier, "init") == 0)
                    reportError("special func init cannot be called.", exp->val.func.func_exp->lineno);
                // SYMBOL* s = exp->val.func.func_exp->val.identifier.s;
                // if (s->kind != funcDeclK) reportError("not a function.");
                // TODO:

                // check parameter types

                typeCheckEXP_LIST(exp->val.func.args);

                PARAMETERS* p = exp->val.func.func_spec->parameters;
                EXP_LIST* e = exp->val.func.args;
                while (p != NULL && e != NULL){
                    if (!isEqualType(e->exp->type, p->type)) reportError("parameter not equal type", e->exp->lineno);
                    p = p->next;
                    e = e->next;
                }
                if (p != NULL || e != NULL) reportError("Too many or too few parameters", exp->val.func.func_exp->lineno);
                // prettyTYPE(exp->type);
                // exp->type = s->val.funcSpec->returnType;
                break;


            case castK:
                // disguish cast and func is done in symbol phase
                {
                    // prettyTYPE(exp->type);
                    // exp->type = exp->val.cast.exp->type;
                    // prettyEXP(exp);
                    typeCheckEXP(exp->val.cast.exp);
                    // if (exp->val.cast.type == NULL) printf("1 is nulll\n");
                    // if (exp->val.cast.exp->type == NULL) printf("2 is nulll\n");
                    TYPE* t = resolveType(exp->val.cast.type);
                    // prettyTYPE_SPEC(exp->val.cast.exp->type->spec);
                    TYPE* e = resolveType(exp->val.cast.exp->type);

                    // printf("================\n");
                    // prettyTYPE(exp->type);
                    // prettyTYPE(exp->val.cast.exp->type);

                    if (!resolveToBaseType(t))
                        reportError("cannot resolve to base type", exp->lineno);
                    if (isEqualType(t, e) ||
                        (resolveToNumbericBaseType(t) && resolveToNumbericBaseType(e)) ||
                        (t->kind == stringK && resolveToIntegerBaseType(e))) return;
                    reportError("conversion between incompatible types", exp->lineno);

                }

                break;

            case appendK:
                typeCheckEXP(exp->val.append.head);
                typeCheckEXP(exp->val.append.tail);
                TYPE* at = resolveType(exp->val.append.head->type);
                if (at->kind != sliceK) reportError("cannot append to this type", exp->val.append.head->lineno);
                TYPE* itemType = at->val.sliceElement;
                if(!isEqualType(itemType, exp->val.append.tail->type)) reportError("cannot append an object of inconsistent type.", exp->val.append.tail->lineno);
                exp->type = exp->val.append.head->type;
                break;

            case arrayindexK:
                {
                    // printf("%s\n", exp->val.array_index.array_exp->type->kind);
                    // printf("fsdffds\n");
                    typeCheckEXP(exp->val.array_index.array_exp);
                    TYPE* rt = resolveType((exp->val.array_index.array_exp)->type);
                    // distinguish between array and slice
                    if (rt->kind == sliceK){
                        EXP* tempS = exp->val.array_index.array_exp;
                        EXP* tempI = exp->val.array_index.index;
                        exp->val.array_index.array_exp = tempS;
                        exp->val.array_index.index = tempI;
                        exp->kind = sliceindexK;
                        typeCheckEXP(exp);
                        break;
                    }

                    typeCheckEXP(exp->val.array_index.index);
                    TYPE* it = resolveType(exp->val.array_index.index->type);
                    if (rt->kind != arrayK) reportError("is not addressable array", (exp->val.array_index.array_exp)->lineno);
                    if (it->kind != intK) reportError("index should be integer", (exp->val.array_index.index)->lineno);
                    exp->type = rt->val.array.element;
                    break;

                }

            case sliceindexK:
                // printf("fsdffds\n");
                {
                    typeCheckEXP(exp->val.slice_index.slice_exp);
                    typeCheckEXP(exp->val.slice_index.index);
                    TYPE* rt = resolveType((exp->val.array_index.array_exp)->type);
                    TYPE* it = resolveType((exp->val.array_index.index)->type);
                    if (rt->kind != sliceK) reportError("is not addressable slice", (exp->val.slice_index.slice_exp)->lineno);
                    if (it->kind != intK)  reportError("index should be integer", (exp->val.slice_index.index)->lineno);
                    exp->type = rt->val.sliceElement;
                    break;

                }

        case structaccessK:
                // if (exp->val.struct_access.struct_exp->kind != identifierK) reportError("not struct", exp->val.struct_access.struct_exp->lineno);
                {
                    typeCheckEXP(exp->val.struct_access.struct_exp);
                    TYPE* rt = resolveType(exp->val.struct_access.struct_exp->type);
                    if (rt->kind != structK) reportError("not struct", exp->lineno);
                    FIELD_DECL* fd = rt->val.fieldDecls;
                    // prettyFIELD_DECL_LIST(fd);
                    while(fd != NULL){
                        if (strcmp(fd->identifier, exp->val.struct_access.field_name) == 0){
                            exp->type = fd->type;
                            return;
                        }
                        fd = fd->next;
                    }
                    reportError("field not found", exp->lineno);
                }

                // typeCheckEXP(exp->val.struct_access.field_name);
                // if (exp->val.struct_access.struct_exp->kind != structK) reportError("is not addressable struct", exp->val.struct_access.struct_exp->lineno);
                // weedEXP(exp->val.struct_access.struct_exp, 0);

                break;
        }
    }
}



TYPE* typeCheckEXPBinaryOp(EXP* e){
  return typeCheckBinaryOp(e->val.binary.lhs, e->val.binary.rhs, e->kind);
}

TYPE* typeCheckBinaryOp(EXP *lhs, EXP *rhs, EXP_Kind op){
    typeCheckEXP(lhs);
    typeCheckEXP(rhs);

    if (!isEqualType(lhs->type, rhs->type)){
        // prettyTYPE(lhs->type);
        // printf("%d\n", lhs->type->kind);
        // prettyTYPE(rhs->type);
        // printf("%d\n", rhs->type->kind);
      reportError("Type not the same", lhs->lineno);
    }
    TYPE* rt1 = resolveType(lhs->type);
    switch (op) {
        case orK:
        case andK:
            if (rt1->kind != boolK){
                reportError("type cannot resolve to bool", lhs->lineno);
            }
            return lhs->type;

        case eqK:
        case neqK:
            if (!isComparable(rt1)){
                reportError("type not comparable", lhs->lineno);
            }
            return boolTYPE;

        case ltK:
        case gtK:
        case leqK:
        case geqK:
            if (!isOrdered(rt1)){
                reportError("type not ordered", lhs->lineno);
            }
            return boolTYPE;

        case plusK:
            if (!resolveToNumbericBaseType(lhs->type) && rt1->kind != stringK){
                reportError("type not numeric or string", lhs->lineno);
            }
            return lhs->type;

        case minusK:
        case multK:
        case divideK:
            if (!resolveToNumbericBaseType(lhs->type) ){
                reportError("type not numeric", lhs->lineno);
            }
            return lhs->type;


        case modK:
        case bitorK:
        case xorK:
        case lshiftK:
        case rshiftK:
        case bitandK:
        case bitclearK:
        if (!resolveToIntegerBaseType(lhs->type) ){
            reportError("type not integer", lhs->lineno);
        }
        return lhs->type;

        default:
            printf("error");
            exit(1);
    }
    return NULL;
}
