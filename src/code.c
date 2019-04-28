#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "code.h"
#include "symbol.h"
#include "type.h"
#include "pretty.h"

// TODO: call by value

FILE* output;
const char* filename;
int indentation;
int tempCnt;

void PRINT(char* str){
    fprintf(output, "%s", str);
}

void indent(){
    for (int i = 0; i < indentation; i++){
        PRINT("\t");
    }
}

// js support optional semicolon, so we don't need to worry about adding semicolon
void nextLine(){
    PRINT("\n");
    indent();
}



void createFile(char* inputPath){
    char outPath[strlen(inputPath)+3];
	for (int i = strlen(inputPath)-1; i >= 0; i--) {
		if (inputPath[i]=='.') {
            memcpy(outPath, inputPath, i+1);
            outPath[i+1] = 'j';
            outPath[i+2] = 's';
            outPath[i+3] = '\0';
			break;
		}
	}

    output = fopen(outPath, "w");
	return;
}

void code(PROG *n, char* inputPath){
    createFile(inputPath);
    codeInit();
    codeProgram(n);
    char* execute =
        "if (typeof _main == \"function\") _main()\n\n";
    PRINT(execute);
    fclose(output);
}


void codeInit(){
    indentation = 0;
    tempCnt = 0;

    // helper functions

    const char* newSlice =
    "function __newSlice(){\n"
    "   return {\n"
    "		    isSlice: true,\n"
    "    	    val:[],\n"
    "           size:0\n"
    "       };\n"
    "}\n\n";

    const char* sliceAppend =
    "function __append(a, b){ \n"
    "   let _a =  __clone(a)\n"
    "   _a.val[_a.size] = b\n"
    "   _a.size++\n"
    "   return _a;\n"
    "}\n\n";

    const char* newArray =
    "function __newArray(len, instance){\n"
    "   return Array.from(new Array(len), x => instance)\n"
    "}\n\n";

    // o1, o2 has passed typecheck phase, so they are of same type and we only need to check the value
    const char* isEqual =
    "function __isEqual(o1, o2){\n"
    "   if (typeof o1 == \"object\"){\n"
    "       for (let key in o1){\n"
    "           if (!__isEqual(o1[key], o2[key])) return false;\n"
    "       }\n"
    "   return true;\n"
    "   }\n"
    "   return o1 == o2;\n"
    "}\n\n";


    const char* intDiv =
    "function __intDiv(a, b){\n"
    "   let result = a/b;\n"
    "   return result > 0? Math.floor(result) : Math.ceil(result);\n"
    "}\n\n";


    const char* clone =
    "function __clone(o){\n"
    "   if (typeof o == \"object\"){\n"
    "       if (o.isSlice) return {\n"
    "		    isSlice: true,\n"
    "    	    val:o.val,\n"
    "           size:o.size\n"
    "       };\n"
    "       if (Array.isArray(o)){\n"
    "           if (o[0] && typeof o[0] !=\"object\")\n"
    "                return o.slice(0)\n"
    "    	  let clone = [];\n"
    "           for (let key in o){\n"
    "               clone[key] = __clone(o[key]);\n"
    "           }\n"
    "           return clone;\n"
    "       }\n"
    "       else{\n"
    "           let clone = {};\n"
    "           for (let key in o){\n"
    "               clone[key] = __clone(o[key])\n"
    "           }\n"
    "           return clone;\n"
    "       }\n"
    "   }\n"
    "   return o;\n"
    "}\n\n";


    const char* floatToString =
    "function __floatToString(f){\n"
    "   let result = ((f = +f) || 1 / f) > 0 ? \"+\" : \"-\";\n"
    "   result += f.toExponential(6);\n"
    "   return result;\n"
    "}\n\n";


    const char* setArrayValue =
    "function __setArrayValue(a, index, value, size){\n"
    "   if (index >= size){\n"
    "       console.log(\"Error: out-of-bounds\")\n"
    "       process.exit(1)\n"
    "   }\n"
    "   a[index] = value\n"
    "}\n";


    const char* arrayIndex =
    "function __arrayIndex(a, size, i){\n"
    "   if (i >= size) {\n"
    "       console.log(\"Error: out-of-bounds\")\n"
    "       process.exit(1)\n"
    "   }\n"
    "   return a[i]\n"
    "}\n";

    const char* sliceIndex =
    "function __sliceIndex(s, i){\n"
    "   if (i >= s.size){\n"
    "       console.log(\"Error: out-of-bounds\")\n"
    "       process.exit(1)\n"
    "   }\n"
    "   return s.val[i]\n"
    "}\n";

    const char* setSliceValue =
    "function __setSliceValue(a, index, value){\n"
    "   if (index >= a.size){\n"
    "       console.log(\"Error: out-of-bound\")\n"
    "       process.exit(1)\n"
    "   }\n"
    "   a.val[index] = value\n"
    "}\n";



    fprintf(output, "%s", newSlice);
    fprintf(output, "%s", sliceAppend);
    fprintf(output, "%s", newArray);
    fprintf(output, "%s", isEqual);
    fprintf(output, "%s", intDiv);
    fprintf(output, "%s", clone);
    fprintf(output, "%s", floatToString);
    fprintf(output, "%s", setArrayValue);
    fprintf(output, "%s", arrayIndex);
    fprintf(output, "%s", sliceIndex);
    fprintf(output, "%s", setSliceValue);
    fprintf(output, "\n\n\n");

}





void codeProgram(PROG *p){
    if (p!=NULL) {
        // ignore package info
     codeDeclaration(p->root_dec);
 }
}


void codeDeclaration(DEC *d){
        if (d != NULL){
                switch (d->kind) {
                case typeDeclK:
                        // do not generate code for typeDecl
                        break;
                case varDeclK:
                        // rhs of assignment cannot be blank identifier
                        codeVarSpec(d->val.varSpecs);
                        break;
                case funcDeclK:
                        codeFuncSpec(d->val.funcSpec);
                        break;
                }
                codeDeclaration(d->next);
        }
}


bool isBlankIdentifier(char* ident){
    return strcmp(ident, "_") == 0;
}

void codeCloneEXP(EXP* e){
    if (e->type->kind == stringK ||
        e->type->kind == intK ||
        e->type->kind == float64K ||
        e->type->kind == runeK ||
        e->type->kind == boolK){
            codeEXP(e);
        }
    else{
        PRINT("__clone(");
        codeEXP(e);
        PRINT(")");
    }
}

void codeVarSpec(VAR_SPEC *vs){
    if (vs!= NULL){
        if (strcmp(vs->identifier, "_") == 0){
            codeEXP(vs->rhs);
        }
        else{
            if (vs->declared){
                // assign
                PRINT("_");
                PRINT(vs->identifier);
                fprintf(output, "%d", vs->shadowNum);
                PRINT(" = ");
                codeCloneEXP(vs->rhs);
                // PRINT(" = __clone(");
                // codeEXP(vs->rhs);
                // PRINT(")");
            }
            else{
                fprintf(output, "let _%s", vs->identifier);
                fprintf(output, "%d", vs->shadowNum);
                if (vs->type != NULL && vs->rhs == NULL){
                    PRINT(" = ");
                    codeTypeInstance(vs->type);
                }
                else if (vs->rhs != NULL){
                    PRINT(" = ");
                    codeCloneEXP(vs->rhs);
                    // PRINT(" = __clone(");
                    // codeEXP(vs->rhs);
                    // PRINT(")");
                }
            }
        }
        nextLine();
        codeVarSpec(vs -> next);
    }
}

/*
    an instance of this type
*/
void codeTypeInstance(TYPE* t){
    TYPE* rt = resolveType(t);
    switch (rt->kind) {
        case intK:
        case float64K:
            PRINT("0");
            break;
        case boolK:
            PRINT("false");
            break;
        case runeK:
            PRINT("0");
            break;
        case stringK:
            PRINT("\"\"");
            break;
        case arrayK:
            PRINT("__newArray(");
            fprintf(output, "%d", rt->val.array.size);
            PRINT(", ");
            codeTypeInstance(rt->val.array.element);
            PRINT(")");
            break;
        case sliceK:
            PRINT("__newSlice()");
            break;
        case structK:
            PRINT("{ ");
            indentation += 1;
            nextLine();
            codeTypeInstanceFieldDecl(rt->val.fieldDecls);
            nextLine();
            indentation -= 1;
            PRINT("}");
            break;
        default:
            printf("error\n");
            exit(-1);
    }
}
void codeTypeInstanceFieldDecl(FIELD_DECL* f){
    if (f != NULL){
        // do not generate this field if the field name is blank identifier
        if (strcmp(f->identifier, "_") != 0){
            fprintf(output, "_%s: ", f->identifier);
            codeTypeInstance(f->type);
            if (f->next != NULL) {
                PRINT(",");
                nextLine();
                codeTypeInstanceFieldDecl(f->next);
            }
        }
        else{
            codeTypeInstanceFieldDecl(f->next);
        }
    }
}



void codeFuncSpec(FUNC_SPEC *fs){
    if (!isBlankIdentifier(fs->name)){
        // init func
        if (strcmp(fs->name, "init") == 0){
            codeStatement(fs->block, NULL);
            nextLine();
        }
        // other func
        else{
            fprintf(output, "function _%s( ", fs->name);
            codeParameters(fs->parameters);
            PRINT(")");
            codeStatement(fs->block, NULL);
            nextLine();
        }

    } // don't generate func with name of blank identifier
}


void codeParameters(PARAMETERS *p){
    if (p != NULL){
        PRINT("_");
        PRINT(p->identifier);
        fprintf(output, "%d", p->shadowNum);
        if (p->next != NULL){
            PRINT(", ");
            codeParameters(p->next);
        }
    }
}


void codeStatementList(STMT_LIST *b, STMT* PostStmt){
    if (b!=NULL){
        codeStatement(b->stmt, PostStmt);
        codeStatementList(b->next, PostStmt);
    }
}



void codeStatement(STMT *s, STMT* PostStmt){
    if (s == NULL) return;
    switch (s->kind) {
        case emptyK:
            break;

        case blockK:
            PRINT("{");
            indentation++;
            nextLine();
            codeStatementList(s->val.block, PostStmt);
            PRINT("}");
            indentation--;
            break;

        case expressionK:
            // expression statement must be function call
            codeEXP(s->val.exp);
            break;

        case assignK:
            codeStatementAssign(s);
            break;

        case assignopK:
            codeStatementAssignOp(s);
            break;

        case shortdeclareK:
            codeStatementShortDecVar(s->val.shortdecvar);
            break;

        case declareK:
            codeDeclaration(s->val.declaration);
            break;

        case incK:
        case decK:
            codeStatement(incToAssign(s), false);
            break;

        case printK:
            codePrint(s->val.print_expressions, false);
            break;

        case printlnK:
            codePrint(s->val.print_expressions, true);
            break;

        case returnK:
            PRINT("return ");
            codeEXP(s->val.ret_exp);
            break;

        case ifK:
            PRINT("{");
            indentation++;
            nextLine();
            codeStatement(s->val.ifstmt.optional_stmt, NULL);
            PRINT("if (");
            codeEXP(s->val.ifstmt.exp);
            PRINT(")");
            codeStatement(s->val.ifstmt.block, PostStmt);
            if (s->val.ifstmt.optional_else != NULL){
                PRINT("else ");
                codeStatement(s->val.ifstmt.optional_else, PostStmt);
            }
            PRINT("}");
            indentation--;
            break;

        case elseK:
            codeStatement(s->val.else_stmt, PostStmt);
            break;

        case switchK:
            codeStatementSwitch(s);
            break;

        case forK:
            codeStatementFor(s);
            break;

        case breakK:
            PRINT("break");
            break;

        case continueK:
            codeStatement(PostStmt, NULL);
            PRINT("continue");
            break;

        }
        nextLine();

}

STMT* incToAssign(STMT* s){
    EXP* e = s->kind == incK? s->val.inc_exp : s->val.dec_exp;
    EXP* rhs = makeBinaryExp(s->kind == incK? plusK : minusK, e, makeIntExp(1));
    rhs->type = intTYPE;
    return makeAssignStmt(makeExpList(e), makeExpList(rhs));
}

void codeStatementShortDecVar(VAR_SPEC *vs){
    int localTempCnt = 0;
    VAR_SPEC* cur = vs;
    while(cur != NULL){
        if (strcmp(cur->identifier, "_") == 0){
            codeEXP(cur->rhs);
        }
        else{
            if (cur->declared){
                // assign
                // save evalued result to temp value
                if (localTempCnt ==  tempCnt){
                    // create new temp value
                    tempCnt++;
                    PRINT("let _temp_");
                    fprintf(output, "%d", localTempCnt);
                    nextLine();
                }

                PRINT("_temp_");
                fprintf(output, "%d", localTempCnt);
                PRINT(" = ");
                codeCloneEXP(cur->rhs);
                // PRINT(" = __clone(");
                // codeEXP(cur->rhs);
                // PRINT(")");
                localTempCnt++;
            }

            else{
                fprintf(output, "let _%s", cur->identifier);
                fprintf(output, "%d", cur->shadowNum);
                if (cur->type != NULL && cur->rhs == NULL){
                    PRINT(" = ");
                    codeTypeInstance(cur->type);
                }
                else if (cur->rhs != NULL){
                    PRINT(" = ");
                    codeCloneEXP(cur->rhs);
                    // PRINT(" = __clone(");
                    // codeEXP(cur->rhs);
                    // PRINT(")");
                }
            }
        }
        nextLine();
        cur = cur->next;
    }


    // flush temp value
    cur = vs;
    localTempCnt = 0;
    while (cur != NULL){
        if (strcmp(cur->identifier, "_") != 0 && cur->declared){
            fprintf(output, "_%s", cur->identifier);
            fprintf(output, "%d", cur->shadowNum);
            PRINT(" = _temp_");
            fprintf(output, "%d", localTempCnt);
            nextLine();
            localTempCnt++;
        }
        cur = cur->next;
    }

}


void codeStatementAssign(STMT *s){
    int localTempCnt = 0;
    ASSIGN* cur;
    cur = s->val.assign;
    while (cur != NULL){
        if (cur->lhs->kind == identifierK && isBlankIdentifier(cur->lhs->val.identifier.identifier)){
            // printf("222222\n");
            codeEXP(cur->rhs);
            nextLine();
            cur = cur->next;
            continue;
        }

        if (localTempCnt ==  tempCnt){
            // create new temp value
            tempCnt++;
            PRINT("let _temp_");
            fprintf(output, "%d", localTempCnt);
            nextLine();
        }

        // codeEXP(cur->lhs);
        // eval rhs, save value to temp value
        PRINT("_temp_");
        fprintf(output, "%d", localTempCnt);
        PRINT(" = ");
        codeCloneEXP(cur->rhs);
        // PRINT(" = __clone(");
        // codeEXP(cur->rhs);
        // PRINT(")");
        nextLine();
        cur = cur->next;
        localTempCnt++;
    }

    cur = s->val.assign;
    localTempCnt = 0;

    // flush temp value
    while(cur != NULL){
        if (cur->lhs->kind == identifierK && isBlankIdentifier(cur->lhs->val.identifier.identifier)){
            cur = cur->next;
            continue;
        }
        if (cur->lhs->kind == arrayindexK){
            PRINT("__setArrayValue(");
            codeEXP(cur->lhs->val.array_index.array_exp);
            PRINT(", ");
            codeEXP(cur->lhs->val.array_index.index);
            PRINT(", _temp_");
            fprintf(output, "%d", localTempCnt);
            PRINT(", ");
            fprintf(output, "%d", cur->lhs->val.array_index.array_exp->type->val.array.size);
            PRINT(")");
        }
        else if (cur->lhs->kind == sliceindexK){
            PRINT("__setSliceValue(");
            codeEXP(cur->lhs->val.slice_index.slice_exp);
            PRINT(", ");
            codeEXP(cur->lhs->val.slice_index.index);
            PRINT(", _temp_");
            fprintf(output, "%d", localTempCnt);
            PRINT(")");
        }
        else{
            codeEXP(cur->lhs);
            PRINT(" = _temp_");
            fprintf(output, "%d", localTempCnt);
        }

        nextLine();
        localTempCnt++;
        cur = cur->next;
    }
}


void codeStatementAssignOp(STMT *s){
    codeEXP(s->val.assignop.lhs);
    PRINT(" = ");
    switch (s->val.assignop.assign_type) {
        case pluseq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " + ");
            break;

        case minuseq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " - ");
            break;

        case bitoreq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " | ");
            break;

        case xoreq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " ^ ");
            break;

        case multeq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " * ");
            break;

        case diveq:
            if (resolveToIntegerBaseType(s->val.assignop.lhs->type)){
                PRINT("__intDiv(");
                codeEXP(s->val.assignop.lhs);
                PRINT(", ");
                codeEXP(s->val.assignop.rhs);
                PRINT(")");
            }
            else{
                codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " / ");
            }
            break;

        case modeq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " % ");
            break;

        case lshifteq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " << ");
            break;

        case rshifteq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " >> ");
            break;

        case bitandeq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " & ");
            break;

        case bitcleareq:
            codeBinary(s->val.assignop.lhs, s->val.assignop.rhs, " &~ ");
            break;
    }
}


void codePrint(EXP_LIST* el, bool format){
    EXP_LIST *cur = el;
    while(cur !=NULL){
        if (resolveType(cur->exp->type)->kind == stringK){
            PRINT("process.stdout.write(");
            codeEXP(cur->exp);
            // if (cur->exp != NULL) codeEXP(cur->exp);
            // else PRINT("\"\"");
            PRINT(")");
        }
        else if (resolveType(cur->exp->type)->kind == float64K){
            PRINT("process.stdout.write(__floatToString(");
            // if (cur->exp != NULL) codeEXP(cur->exp);
            // else PRINT("\"\"");
            codeEXP(cur->exp);
            PRINT("))");
        }
        else{
            PRINT("process.stdout.write(JSON.stringify(");
            codeEXP(cur->exp);
            PRINT("))");
        }

        nextLine();
        if (format && cur->next != NULL) PRINT("process.stdout.write(\"\\ \")");
        nextLine();
        cur = cur->next;
    }
    if (format) PRINT("process.stdout.write(\"\\n\")");
}


void codeStatementSwitch(STMT* s){
    if (s == NULL) return;

    if (s->val.switchstmt.exp == NULL) {
        // use if else
        PRINT("{");
        indentation++;
        nextLine();
        codeStatement(s->val.switchstmt.stmt, NULL);


        CASE_CLAUSE_LIST* cur = s->val.switchstmt.clauses;
        CASE_CLAUSE* def = NULL;
        while(cur != NULL){
            if (cur->clause->kind == defaultK) {
                def = cur->clause;
            }

            else{
                // FIXME: do not fall through
                // not default cases
                PRINT("if (");
                codeEXP_LIST(cur->clause->val.case_clause.cases, false);
                PRINT(") {");
                indentation++;
                nextLine();
                codeStatementList(cur->clause->val.case_clause.clause, NULL);
                PRINT("}");
                indentation--;
                nextLine();
            }
            cur = cur->next;
        }

        // print default
        if (def != NULL){
            PRINT("else {");
            indentation++;
            nextLine();
            codeStatementList(def->val.default_clause, NULL);
            PRINT("}");
            indentation--;
            nextLine();
        }

        indentation--;
        PRINT("}");
    }

    else {
        PRINT("{");
        indentation++;
        nextLine();
        codeStatement(s->val.switchstmt.stmt, NULL);
        PRINT("switch (");
        codeEXP(s->val.switchstmt.exp);
        PRINT("){");
        indentation++;
        nextLine();
        CASE_CLAUSE_LIST* cur = s->val.switchstmt.clauses;
        while(cur != NULL){
            if (cur->clause->kind == defaultK) {
                PRINT("default: {");
                indentation++;
                nextLine();
                // FIXME: block?
                codeStatementList(cur->clause->val.default_clause, NULL);
                PRINT("}");
                indentation--;
                nextLine();
            }

            else{
                EXP_LIST* el = cur->clause->val.case_clause.cases;
                while(el!= NULL){
                    PRINT("case ");
                    codeEXP(el->exp);
                    PRINT(": {");
                    indentation++;
                    nextLine();
                    codeStatementList(cur->clause->val.case_clause.clause, NULL);
                    PRINT("break");
                    indentation--;
                    nextLine();
                    PRINT("}");
                    nextLine();
                    el =  el->next;
                }
                // not default cases
                PRINT("case ");
                codeEXP_LIST(cur->clause->val.case_clause.cases, false);
                PRINT(": {");
                indentation++;
                nextLine();
                codeStatementList(cur->clause->val.case_clause.clause, NULL);
                PRINT("break");
                indentation--;
                nextLine();
                PRINT("}");
                nextLine();
            }
            cur = cur->next;
        }

        indentation--;
        PRINT("}");
        nextLine();
        indentation--;
        PRINT("}");
    }
}


void codeStatementFor(STMT* s){
    // while loop
    if (!hasForClause(s) ||
        (hasForClause(s) && s->val.forstmt.clause->cond ==  NULL)){
        PRINT("{");
        indentation++;
        nextLine();
        if (s->val.forstmt.clause != NULL) codeStatement(s->val.forstmt.clause->init, NULL);
        if (s->val.forstmt.while_exp != NULL){
            PRINT("while(");
            codeEXP(s->val.forstmt.while_exp);
            PRINT(")");
        }

        else{
            PRINT("while (true)");
        }
        nextLine();
        codeStatement(s->val.forstmt.block, NULL);
        PRINT("}");
        indentation--;
        nextLine();
    }

    // 3 part for loop
    else if (s->val.forstmt.while_exp == NULL && hasForClause(s)){
        PRINT("{");
        indentation++;
        nextLine();
        codeStatement(s->val.forstmt.clause->init, NULL);
        PRINT("while (");
        codeEXP(s->val.forstmt.clause->cond);
        PRINT("){");
        indentation++;
        nextLine();
        codeStatement(s->val.forstmt.block, s->val.forstmt.clause->post);
        codeStatement(s->val.forstmt.clause->post, NULL);
        PRINT("}");
        indentation--;
        nextLine();
        PRINT("}");
        indentation--;
        nextLine();
    }
}


void codeEXP(EXP* exp){
    if (exp == NULL) return;
    switch (exp->kind) {
        case orK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " || ");
            break;

        case andK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " && ");
            break;

        case eqK:
            PRINT("__isEqual(");
            codeEXP(exp->val.binary.lhs);
            PRINT(", ");
            codeEXP(exp->val.binary.rhs);
            PRINT(")");
            break;

        case neqK:
            PRINT("!__isEqual(");
            codeEXP(exp->val.binary.lhs);
            PRINT(", ");
            codeEXP(exp->val.binary.rhs);
            PRINT(")");
            break;

        case ltK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " < ");
            break;

        case gtK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " > ");
            break;

        case leqK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " <= ");
            break;

        case geqK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " >= ");
            break;

        case plusK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " + ");
            break;

        case minusK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " - ");
            break;

        case bitorK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " | ");
            break;

        case xorK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " ^ ");
            break;

        case multK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " * ");
            break;

        case divideK:
            if (resolveToIntegerBaseType(exp->val.binary.lhs->type)){
                PRINT("__intDiv(");
                codeEXP(exp->val.binary.lhs);
                PRINT(", ");
                codeEXP(exp->val.binary.rhs);
                PRINT(")");
            }
            else{
                codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " / ");
            }
            break;

        case modK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " % ");
            break;

        case lshiftK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " << ");
            break;

        case rshiftK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " >> ");
            break;

        case bitandK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " & ");
            break;


        case bitclearK:
            codeBinary(exp->val.binary.lhs, exp->val.binary.rhs, " &~ ");
            break;

        case uminusK:
            codeUnary(exp, "-");
            break;

        case uplusK:
            codeUnary(exp, "+");
            break;

        case ubangK:
            codeUnary(exp, "!");
            break;

        case uxorK:
            codeUnary(exp, "~");
            break;

        case identifierK:
            PRINT("_");
            PRINT(exp->val.identifier.identifier);
            if (exp->val.identifier.s != NULL && exp->val.identifier.s->kind == varSymK)
                fprintf(output, "%d", exp->val.identifier.shadowNum);
            // FIXME: blank identifer in struct
            break;

        case float64valK:
            fprintf(output, "%f", exp->val.float64val);
            break;
        case runevalK:
            {
                int runeVal = exp->val.runeval;
                fprintf(output, "%d", runeVal);
                break;
            }
        case intvalK:
            fprintf(output, "%d", exp->val.intval);
            break;
        case stringvalK:
            // PRINT("\"");
            PRINT(exp->val.stringval);
            // PRINT("\"");s
            break;
        case boolvalK:
            if (exp->val.boolval)  PRINT("true");
            else PRINT("false");
            break;

        case funcK:
            codeEXP(exp->val.func.func_exp);
            PRINT("(");
            codeEXP_LIST(exp->val.func.args, true);
            PRINT(")");
            break;


        case castK:
            codeCast(exp);
            break;

        case appendK:
            PRINT("__append(");
            codeEXP(exp->val.append.head);
            PRINT(", ");
            codeEXP(exp->val.append.tail);
            PRINT(")");
            break;

        case arrayindexK:
            // if ()
            PRINT("__arrayIndex(");
            codeEXP(exp->val.array_index.array_exp);
            PRINT(", ");
            fprintf(output, "%d", exp->val.array_index.array_exp->type->val.array.size);
            PRINT(", ");
            codeEXP(exp->val.array_index.index);
            PRINT(")");
            break;

        case sliceindexK:
            PRINT("__sliceIndex(");
            codeEXP(exp->val.slice_index.slice_exp);
            PRINT(", ");
            codeEXP(exp->val.slice_index.index);
            PRINT(")");
            break;

        case structaccessK:
            codeEXP(exp->val.struct_access.struct_exp);
            PRINT("._");
            PRINT(exp->val.struct_access.field_name);
            break;
    }
}

/* assume separated by comma */
void codeEXP_LIST(EXP_LIST *list, bool clone){
    EXP_LIST *cur = list;
    while(cur !=NULL){
        if (clone){
            // PRINT(" = ");
            codeCloneEXP(cur->exp);
            // PRINT("__clone(");
            // codeEXP(cur->exp);
            // PRINT(")");
        }
        else {
            codeEXP(cur->exp);
        }
        if (cur->next != NULL) PRINT(", ");
        cur = cur->next;
    }
}


void codeBinary(EXP* lhs, EXP* rhs, char* op){
    PRINT("(");
    codeEXP(lhs);
    PRINT(op);
    codeEXP(rhs);
    PRINT(")");
}


void codeUnary(EXP* exp, char* op){
    PRINT("(");
    PRINT(op);
    codeEXP(exp->val.unary.child);
    PRINT(")");
}


void codeCast(EXP* exp){
    TYPE *rct = resolveType(exp->type);
    TYPE *et = resolveType(exp->val.cast.exp->type);

    // type and expr resolve to identical underlying types ->no cast
    if (isEqualType(rct, et)) codeEXP(exp->val.cast.exp);

    else{
        if (resolveToNumbericBaseType(rct)){
            if (rct->kind == intK){
                // runeK -> intK: no cast, both int type
                if (et->kind == runeK) codeEXP(exp->val.cast.exp);
                // float64K -> intK
                else if (et->kind == float64K) {
                    PRINT("Math.floor(");
                    codeEXP(exp->val.cast.exp);
                    PRINT(")");
                }
            }
            else if (rct->kind == runeK){
                // intK -> runeK: no cast, both int type
                if (et->kind == intK) codeEXP(exp->val.cast.exp);
                else if (et->kind == float64K){
                    PRINT("Math.floor(");
                    codeEXP(exp->val.cast.exp);
                    PRINT(")");
                }
            }
            else if (rct->kind == float64K){
                if (et->kind == intK || et->kind == runeK) codeEXP(exp->val.cast.exp);
            }
        }

        //type resolves to a string type and expr resolves to an integer type (rune or int)
        if (rct->kind == stringK){
            // TODO: to string
            PRINT("String.fromCharCode(");
            codeEXP(exp->val.cast.exp);
            PRINT(")");
        }
    }

}
