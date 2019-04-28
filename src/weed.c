#include "tree.h"
#include "weed.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "comm.h"
#include "pretty.h"

// TODO: Additionally, for functions that return a value, the statements list should end in a terminating
// statement (weeding pass).



/**
Break and continue statements
break and continue must only appear withing a loop (or break also within a switch
case)
•**/

// switch : only one default case.


// blank identifier
// 1. The PackageName must not be the blank identifier -> check in weedPROGRAM

// expression operand
// The blank identifier cannot appear as an operand on the right-hand side of an assignment.


// selector
// x.f
// denotes the field or method f of the value x (or sometimes *x; see below). The identifier f is called the (field or method) selector; it must not be the blank identifier.


// assignment
// assignment operation x op= y
// and the left-hand expression must not be the blank identifier.

// - RHS of assignments/declarations
// - Type specifiers (in function signatures, declarations, etc)
// - Indexing/selection expressions
// - Function calls/append
// - Expressions which perform an op (unary/binary)
// - Expression statements (which is why the init of switch/for/if is not OK)
// - Conditions/switch expressions/cases



void weedPROGRAM(PROG *p){
    if (p!=NULL) {
        // The PackageName must not be the blank identifier.
        if (strcmp(p->package->name, "_") == 0){
            fprintf(stderr, "Error: (line %d) package name cannot be blank identifier\n", p->package->lineno);
            exit(1);
        }
     weedDeclaration(p->root_dec);
 }
}


void weedDeclaration(DEC *d){
    // nested function are handled by parser
        if (d != NULL){
                weedDeclaration(d->next);
                switch (d->kind) {
                case typeDeclK:
                        weedTypeSpec(d->val.typeSpecs);
                        break;
                case varDeclK:
                        // rhs of assignment cannot be blank identifier
                        weedVarSpec(d->val.varSpecs);
                        break;
                case funcDeclK:
                        weedParameters(d->val.funcSpec->parameters);
                        weedStatement(d->val.funcSpec->block, null);
                        weedTYPE(d->val.funcSpec->returnType, 0);/*
                        if(d->val.funcSpec->returnType != NULL && !weedStatementTerminate(d->val.funcSpec->block))
                            reportError("does not terminate", d->lineno);*/
                        break;
                }
        }
}




void weedTypeSpec(TYPE_SPEC *ts){
    if (ts!= NULL){
        weedTypeSpec(ts -> next);
        weedTYPE(ts->type, 0);
    }
}


void weedVarSpec(VAR_SPEC *vs){
    if (vs!= NULL){
        weedVarSpec(vs -> next);
        weedEXP(vs->rhs, 0);
        weedTYPE(vs->type, 0);
    }
}

// blank identifier:
// Parameter names -> ok
// type -> no
void weedParameters(PARAMETERS *p){
    if (p != NULL){
        weedParameters(p->next);
        weedTYPE(p->type, 0);
    }
}


void weedStatementList(STMT_LIST *b, weedSTMTTag label){
    if (b!=NULL){
        weedStatement(b->stmt, label);
        weedStatementList(b->next, label);
    }
}



void weedStatement(STMT *s, weedSTMTTag label){
    if (s == NULL) return;
    switch (s->kind) {
        case emptyK:
            break;
        case blockK:
            weedStatementList(s->val.block, label);
            break;
        case expressionK:
            // expression statement must be function call
            if (s->val.exp->kind != funcK){
                fprintf(stderr, "Error: (line %d) epression statement must be function call.\n", s->lineno);
                exit(1);
            }
            weedEXP(s->val.exp, 1);
            break;
        case assignK:
            weedStatementAssign(s, label);
            break;
        case assignopK:
            weedEXP(s->val.assignop.lhs, 0);
            weedEXP(s->val.assignop.rhs, 0);
            break;
        case shortdeclareK:
            weedStatementShortDecVar(s, 0);
            break;
        case declareK:
            weedDeclaration(s->val.declaration);
            break;
        case incK:
            weedEXP(s->val.inc_exp, 0);
            break;
        case decK:
            weedEXP(s->val.dec_exp, 0);
            break;
        case printK:
        case printlnK:
            weedEXP_LIST(s->val.print_expressions, 0);
            break;
        case returnK:
            weedEXP(s->val.ret_exp, 0);
            break;
        case ifK:
            weedStatement(s->val.ifstmt.optional_stmt, null);   // FIXME
            weedEXP(s->val.ifstmt.exp, 0);
            weedStatement(s->val.ifstmt.block, label);
            weedStatement(s->val.ifstmt.optional_else, label);
            break;
        case elseK:
            weedStatement(s->val.else_stmt, label);
            break;

        case switchK:
            weedStatementSwitch(s, label);
            break;
        case forK:
            weedEXP(s->val.forstmt.while_exp, 0);
            weedStatement(s->val.forstmt.block, InLoop);
            weedForClause(s->val.forstmt.clause);
            break;
        case breakK:
            if (label != InSwitch && label != InLoop){
                fprintf(stderr, "Error: (line %d) illegal use of break keyword.\n", s->lineno);
                exit(1);
            }
            break;
        case continueK:
            if (label !=InLoop){
                fprintf(stderr, "Error: (line %d) illegal use of continue keyword.\n", s->lineno);
                exit(1);
            }
            break;

        }

}

bool weedStatementListTerminates(STMT_LIST *b){
    if (b!=NULL){
        if (b->next == NULL) {
            // printf("=======%d\n", weedStatementTerminate(b->stmt));
            return weedStatementTerminate(b->stmt);
        }
        return weedStatementListTerminates(b->next);
    }
    return false;
}


bool weedStatementTerminate(STMT *s){
    if (s == NULL) return false;
    else{

        switch (s->kind) {
            case blockK:
                return weedStatementListTerminates(s->val.block);

            case emptyK:
            case expressionK:
            case assignK:
            case assignopK:
            case shortdeclareK:
            case declareK:
            case incK:
            case decK:
            case printK:
            case printlnK:
            case breakK:
                return false;

            case returnK:
                return true;

            case ifK:

                return (weedStatementTerminate(s->val.ifstmt.block) &&
                    (weedStatementTerminate(s->val.ifstmt.optional_else)));


            case elseK:
                return weedStatementTerminate(s->val.else_stmt);

            case switchK:
                return weedStatementSwitchTerminates(s);

            case forK:
            // return true;
                // printf("33333333\n");
                if (s->val.forstmt.while_exp == NULL
                    && (s->val.forstmt.clause == NULL || (s->val.forstmt.clause != NULL && s->val.forstmt.clause->cond == NULL))) {
                        // printf("11111111111\n");
                        // prettyFOR_CLAUSE(s->val.forstmt.clause);
                        // prettyEXP(s->val.forstmt.clause->cond);
                        // return true;
                        return weedStatementForBlockTerminate(s->val.forstmt.block);
                    }
/*
                    prettySTMT(s);
                    if (s->val.forstmt.while_exp == NULL){
                        printf("444444444444\n");
                    }
                    if ()*/
                    // printf("222222222222\n");
                return false;
                // return weedStatementTerminate(s->val.forstmt.block);

            case continueK:
                return false;
            }
    }
    return false;
}


bool weedStatementListForBlockTerminates(STMT_LIST *b){
    if (b!=NULL){
        if (b->next == NULL) {
            // prettySTMT(b->stmt);
            return weedStatementForBlockTerminate(b->stmt);
        }
        return  weedStatementForBlockTerminate(b->stmt) && weedStatementListForBlockTerminates(b->next);
    }
    return true;
}

// there are no "break" statements referring to the "for" statement, and
// the loop condition is absent.
bool weedStatementForBlockTerminate(STMT *s){
    if (s == NULL) return true;
    switch (s->kind) {
        case blockK:
            // prettySTMT(s);
            // printf("222222\n");
            return weedStatementListForBlockTerminates(s->val.block);

        case ifK:
            return (weedStatementForBlockTerminate(s->val.ifstmt.block) &&
                (s->val.ifstmt.optional_else == NULL || weedStatementForBlockTerminate(s->val.ifstmt.optional_else)));


        case elseK:
            return weedStatementTerminate(s->val.else_stmt);

        case switchK:
            weedStatementSwitchTerminates(s);
            break;
        case forK:
            return weedStatementTerminate(s);
        case breakK:
            return false;
        default:
            // printf("333333333\n");
            return true;
        }
    return true;
}



/*
there are no "break" statements referring to the "switch" statement,
there is a default case, and
the statement lists in each case, including the default, end in a terminating statement
*/
bool weedStatementSwitchTerminates(STMT* s){
    // default case
    CASE_CLAUSE_LIST* c =  s->val.switchstmt.clauses;
    bool hasDefaultCase = false;
    while(c != NULL){
        // default case
        if (c->clause->kind == defaultK) {
            hasDefaultCase = true;
            break;
        }

        // break
        STMT_LIST* sl = c->clause->kind == defaultK ?
            c->clause->val.default_clause : c->clause->val.case_clause.clause;

        if (sl == NULL) return false;
        while(sl!=NULL){
            if (sl->stmt->kind == breakK) return false;
            //  end in a terminating statement
            if (sl->next == NULL && !weedStatementTerminate(sl->stmt)) return false;
            sl = sl->next;

        }
        c = c->next;
    }
    if (!hasDefaultCase) return false;
    return true;
}


void weedForClause(FOR_CLAUSE* c){
    if (c != NULL){
        weedEXP(c->cond, 0);
        weedStatement(c->init, null);
        weedStatement(c->post, null);
    }

}
/**
emptyK, blockK, expressionK, assignK, assignopK, declareK, shortdeclareK, incK, decK, printK, printlnK, returnK,
ifK, elseK, switchK, forK, breakK, continueK
**/
void weedStatementAssign(STMT *s, weedSTMTTag label){
    ASSIGN* cur;
    cur = s->val.assign;
    while (cur != NULL){
        weedEXP(cur->lhs, 1);
        weedEXP(cur->rhs, 0);
        cur = cur->next;
    }

}

void weedStatementShortDecVar(STMT *s, weedSTMTTag label){
    weedVarSpec(s->val.shortdecvar);
}





void weedStatementSwitch(STMT* s, weedSTMTTag label){
    if (s == NULL) return;
    weedStatement(s->val.switchstmt.stmt, null);    // FIXME
    weedEXP(s->val.switchstmt.exp, 0);

    int defaultCaseCnt = 0;
    CASE_CLAUSE_LIST* cur = s->val.switchstmt.clauses;
    while(cur != NULL){
        if (cur->clause->kind == defaultK) {
            defaultCaseCnt ++;
            weedStatementList(cur->clause->val.default_clause, InSwitch);
            if (defaultCaseCnt >1) {
                fprintf(stderr, "Error: (line %d) more then one default cases\n", cur->clause->lineno);
                exit(1);
            }
        }

        else{
            // not default cases
            weedEXP_LIST(cur->clause->val.case_clause.cases, 0);
            weedStatementList(cur->clause->val.case_clause.clause, InSwitch);
        }
        cur = cur->next;
    }
}


/*
 1-> allowBlankIdentifier
 0 -> dont allow
*/
void weedEXP(EXP* exp, int allowBlankIdentifier){
    if (exp == NULL) return;
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
            weedEXP(exp->val.binary.lhs, 0);
            weedEXP(exp->val.binary.rhs, 0);
            break;

        case uminusK:
        case uplusK:
        case ubangK:
        case uxorK:
            weedEXP(exp->val.unary.child, 0);
            break;

        case identifierK:
            if (allowBlankIdentifier == 0){
                if (strcmp(exp->val.identifier.identifier, "_") == 0){
                    fprintf(stderr, "Error: (line %d) Illegal use of blank identifier.\n", exp->lineno);
                    exit(1);
                }
            }
            break;

        case float64valK:
        case runevalK:
        case intvalK:
        case stringvalK:
        case boolvalK:
            break;

        case funcK:
            weedEXP(exp->val.func.func_exp, 0);
            weedEXP_LIST(exp->val.func.args, 0);
            break;


        case castK:
            weedTYPE(exp->val.cast.type, 0);
            weedEXP(exp->val.cast.exp, 0);
            break;

        case appendK:
            weedEXP(exp->val.append.head, 0);
            weedEXP(exp->val.append.tail, 0);
            break;

        case arrayindexK:
            weedEXP(exp->val.array_index.array_exp, 0);
            weedEXP(exp->val.array_index.index, 0);
            break;

        case sliceindexK:
            weedEXP(exp->val.slice_index.slice_exp, 0);
            weedEXP(exp->val.slice_index.index, 0);
            break;

        case structaccessK:
            weedEXP(exp->val.struct_access.struct_exp, 0);
            if (allowBlankIdentifier == 0){
                if (strcmp(exp->val.struct_access.field_name, "_") == 0){
                    fprintf(stderr, "Error: (line %d) Illegal use of blank identifier.\n", exp->lineno);
                    exit(1);
                }
            }
            break;
    }
}

void weedEXP_LIST(EXP_LIST *list, int allowBlankIdentifier){
    EXP_LIST *cur = list;
    while(cur !=NULL){
        weedEXP(cur->exp, allowBlankIdentifier);
        cur = cur->next;
    }
}


void weedTYPE(TYPE* t, int allowBlankIdentifier){
    if (t== NULL) return;
    switch (t->kind) {
        case intK:
        case float64K:
        case runeK:
        case stringK:
        case boolK:
        case inferK:
            break;

        case structK:
            weedFieldDecl(t->val.fieldDecls);
            break;

        case sliceK:
            break;

        case arrayK:
            break;
        case refK:
            if (allowBlankIdentifier == 0){
                if (strcmp(t->val.name, "_") == 0){
                    fprintf(stderr, "Error: (line %d) Illegal use of blank identifier.\n", t->lineno);
                    exit(1);
                }
            }
    }
}
void weedFieldDecl(FIELD_DECL *vs){
    if (vs!= NULL){
        weedFieldDecl(vs -> next);
        weedTYPE(vs->type, 0);
    }
}
