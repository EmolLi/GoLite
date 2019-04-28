#include "tree.h"
#include "pretty.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comm.h"


extern int yylineno;




void *Malloc(unsigned n);


// default base types
TYPE *runeTYPE, *intTYPE, *boolTYPE, *float64TYPE, *stringTYPE;
void initTypes(){

	TYPE *intType = makeType("int");
    intType->lineno = 0;
    intType->kind = intK;
    intType->spec = makeTypeSpec(intType, "int");
	intTYPE = intType;

    TYPE *float64Type = makeType("float64");
    float64Type->lineno = 0;
    float64Type->kind = float64K;
    float64Type->spec = makeTypeSpec(float64Type, "float64");
	float64TYPE = float64Type;

    TYPE *boolType = makeType("bool");
    boolType->lineno = 0;
    boolType->kind = boolK;
    boolType->spec = makeTypeSpec(boolType, "bool");
	boolTYPE = boolType;

    TYPE *runeType = makeType("rune");
    runeType->lineno = 0;
    runeType->kind = runeK;
    runeType->spec = makeTypeSpec(runeType, "rune");
	runeTYPE = runeType;

    TYPE *stringType = makeType("string");
    stringType->lineno = 0;
    stringType->kind = stringK;
    stringType->spec = makeTypeSpec(stringType, "string");
	stringTYPE = stringType;
}





PROG *makeProgram(PACKAGE* package, DEC *dec)
{
	PROG *p = (PROG*) malloc(sizeof(PROG));
	p->lineno = yylineno;
	p->package = package;
	p->root_dec = dec;
	return p;
}


PACKAGE *makePackage(char* name){
	PACKAGE *p = (PACKAGE*) malloc(sizeof(PACKAGE));
	p->lineno = yylineno;
	p->name = name;
	return p;
}

/* Add a new declaration to the end of prog's DEC linked list
 dec must already be malloc'd*/
DEC *makeDecls(DEC *decls, DEC *next){
  if(decls == NULL){
    decls = next;
  }
  else{
    DEC *currentDec = decls;
    while(currentDec->next != NULL)
      currentDec = currentDec->next;
    currentDec->next = next;
  }
  return decls;
}



// type decl
DEC *makeTypeDecl(TYPE_SPEC *specs){
	DEC *d = (DEC*) malloc(sizeof(DEC));
	d->lineno = yylineno;
	d->kind = typeDeclK;
	d->val.typeSpecs = specs;
	return d;
}


TYPE_SPEC *makeTypeSpecList(TYPE_SPEC *specs, TYPE_SPEC *next){
	if(specs == NULL){
          specs = next;
	}
	else{
			TYPE_SPEC *current = specs;
			while(current->next != NULL)
				current = current->next;
			current->next = next;
		}
	return specs;
}

TYPE_SPEC *makeTypeSpec(TYPE *type, char* name){
	TYPE_SPEC *s = (TYPE_SPEC*) malloc(sizeof(TYPE_SPEC));
	s->lineno = yylineno;
	s->type = type;
	s->name = name;
	return s;
}


// var decl
DEC *makeVarDecl(VAR_SPEC *specs){
	DEC *d = (DEC*) malloc(sizeof(DEC));
	d->lineno = yylineno;
	d->kind = varDeclK;
	d->val.varSpecs = specs;
	return d;
}


IDENTIFIER_LIST *makeIdentifierList(IDENTIFIER_LIST *ilist, char* identifier){
	IDENTIFIER_LIST *l = (IDENTIFIER_LIST*) malloc(sizeof(IDENTIFIER_LIST));
	l->identifier = identifier;
	l->next = NULL;

	if (ilist == NULL) return l;


	IDENTIFIER_LIST* current = ilist;
	while(current->next != NULL)
		current = current->next;
	current->next = l;
	return ilist;
}


VAR_SPEC *insertVarSpecList(VAR_SPEC *list, VAR_SPEC *v){
  if (list == NULL) return v;

  VAR_SPEC *cur = list;
  while(cur->next != NULL){
    cur = cur->next;
  }
  cur->next = v;
  return list;

}


VAR_SPEC *newVarSpec(char* identifier, EXP *rhs, TYPE *type){
	VAR_SPEC *vs = (VAR_SPEC*) malloc(sizeof(VAR_SPEC));
	vs->lineno = yylineno;
	vs->identifier = identifier;
	vs->rhs = rhs;
	vs->type = type;
	return vs;
}


 VAR_SPEC *makeVarSpec(IDENTIFIER_LIST *ilist, TYPE *type, EXP_LIST* elist){
	 IDENTIFIER_LIST* i = ilist;
	 EXP_LIST*	e = elist;

	 // check lhs.len == rhs.len
	 int iLen = 0;
	 int eLen = 0;
	 while(i != NULL){
		 iLen ++;
		 i = i->next;
	 }
	 while(e != NULL){
		 eLen ++;
		 e = e->next;
	 }
	 if (iLen != eLen && eLen != 0){
	 	fprintf(stderr, "Error: (line %d) lhs and rhs have different length\n", yylineno);
	 	exit(1);
 	}

	 // create a list of VarSpec based on identifierList and EXP_LIST
	 i = ilist;
	 e = elist;
	 VAR_SPEC *head = newVarSpec(i->identifier, e == NULL ? NULL : e->exp, type);
	 i = i->next;
	 e = e == NULL ? NULL :e->next;
	 VAR_SPEC *cur = head;
	 VAR_SPEC *next;
	 while(i != NULL){
		 next = newVarSpec(i->identifier, e == NULL ? NULL : e->exp, type);
		 cur->next = next;
		 cur = next;
		 i = i->next;
		 e = e == NULL ? NULL :e->next;
	 }
	 return head;
 }


// type stuct decls


FIELD_DECL *newFieldDecl(char* identifier, TYPE *type){
	FIELD_DECL *vs = (FIELD_DECL*) malloc(sizeof(FIELD_DECL));
	vs->lineno = yylineno;
	vs->identifier = identifier;
	vs->type = type;
	return vs;
}


 FIELD_DECL *makeFieldDecl(IDENTIFIER_LIST *ilist, TYPE *type){
	 IDENTIFIER_LIST* i = ilist;

	 FIELD_DECL *head = newFieldDecl(i->identifier, type);
	 i = i->next;
	 FIELD_DECL *cur = head;
	 FIELD_DECL *next;
	 while(i != NULL){
		 next = newFieldDecl(i->identifier, type);
		 cur->next = next;
		 cur = next;
		 i = i->next;
	 }
	 return head;
 }


FIELD_DECL *insertFieldDeclList(FIELD_DECL *list, FIELD_DECL *v){
  if (list == NULL) return v;

  FIELD_DECL *cur = list;
  while(cur->next != NULL){
    cur = cur->next;
  }
  cur->next = v;
  return list;

}





// funcDecl
// FIXME: not sure here block is STMT or STMT_LIST
DEC *makeFunction(char* name, PARAMETERS* parameters, STMT* block, TYPE* returnType){
	DEC *f = (DEC*) malloc(sizeof(DEC));
	f->lineno = yylineno;
	f->kind = funcDeclK;
	(f->val.funcSpec) = (FUNC_SPEC*) malloc(sizeof(FUNC_SPEC));
	(f->val.funcSpec)->name = name;
	(f->val.funcSpec)->lineno = yylineno;
	(f->val.funcSpec)->parameters = parameters;
	(f->val.funcSpec)->block = block;
	(f->val.funcSpec)->returnType = returnType;
	return f;
}

// helper function
PARAMETERS* newParameter(char* identifier, TYPE* type){
	PARAMETERS* p = (PARAMETERS*) malloc(sizeof(PARAMETERS));
	p->lineno = yylineno;
	p->next = NULL;
	p->identifier = identifier;
	p->type = type;
	return p;
}

PARAMETERS* makeParameters(PARAMETERS* plist, IDENTIFIER_LIST* ilist, TYPE* type){
	PARAMETERS *phead = newParameter(ilist->identifier, type);
	PARAMETERS *cur = phead;
	IDENTIFIER_LIST* i;
	i = ilist->next;
	while (i!=NULL){
		cur->next = newParameter(i->identifier, type);
		cur = cur->next;
		i = i->next;
	}

	if (plist == NULL) return phead;

	// else append new parameters list to the previous list
	cur = plist;
	while(cur->next !=NULL){
		cur = cur->next;
	}
	cur->next = phead;
	return plist;
}


/********** Statements **************/
/* FOR_CLAUSE */
FOR_CLAUSE *makeForClause(STMT *init, EXP *cond, STMT *post){
  FOR_CLAUSE *f = malloc(sizeof(FOR_CLAUSE));
  f->lineno = yylineno;
  f->init = init;
  f->cond = cond;
  f->post = post;
  return f;
}


/* CASE_CLAUSE(_LIST) */
CASE_CLAUSE_LIST *makeCaseClauseList(CASE_CLAUSE *case_clause){
  CASE_CLAUSE_LIST *cc_list = malloc(sizeof(CASE_CLAUSE_LIST));
  cc_list->clause = case_clause;
  cc_list->next = NULL;
  return cc_list;
}

CASE_CLAUSE_LIST *insertCaseClauseList(CASE_CLAUSE_LIST *list, CASE_CLAUSE *case_clause){
	if (list == NULL) return makeCaseClauseList(case_clause);

  CASE_CLAUSE_LIST *cur = list;
  while(cur->next != NULL){
    cur = cur->next;
  }
  cur->next = makeCaseClauseList(case_clause);

  return list;
}

CASE_CLAUSE *makeCaseClause(EXP_LIST *cases, STMT_LIST *clause){
  CASE_CLAUSE *c = malloc(sizeof(CASE_CLAUSE));
  c->lineno = yylineno;
  c->kind = caseK;
  c->val.case_clause.cases = cases;
  c->val.case_clause.clause = clause;
  return c;
}

CASE_CLAUSE *makeDefaultClause(STMT_LIST *default_clause){
  CASE_CLAUSE *c = malloc(sizeof(CASE_CLAUSE));
  c->lineno = yylineno;
  c->kind = defaultK;
  c->val.default_clause = default_clause;
  return c;
}


/********** Statements  **************/
/* STMT_LIST */
STMT_LIST *makeStmtList(STMT *stmt){
  STMT_LIST *s_list = malloc(sizeof(STMT_LIST));
  s_list->stmt = stmt;
  s_list->next = NULL;
  return s_list;
}

STMT_LIST *insertStmtList(STMT_LIST *list, STMT *stmt){
  if (list == NULL) return makeStmtList(stmt);
  STMT_LIST *cur = list;
  while(cur->next != NULL){
    cur = cur->next;
  }
  cur->next = makeStmtList(stmt);

  return list;

}

/* STMT */
STMT *makeEmptyStmt(){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = emptyK;
  return s;
}

STMT *makeBlockStmt(STMT_LIST *block){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = blockK;
  s->val.block = block;
  return s;
}

STMT *makeExpStmt(EXP *exp){
	if (exp->kind != funcK){
		fprintf(stderr, "Error: (line %d) invalid simple statement\n", yylineno);
		exit(1);
	}
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = expressionK;
  s->val.exp = exp;
  return s;
}

STMT *makeAssignStmt(EXP_LIST *lhs, EXP_LIST *rhs){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = assignK;

  // EXP_LIST* ll;
  // EXP_LIST* rl;
  // ll = lhs;
  // rl = rhs;


  EXP_LIST* l = lhs;
  EXP_LIST*	r = rhs;

  // check lhs.len == rhs.len
  int iLen = 0;
  int eLen = 0;
  while(l != NULL){
	  iLen ++;
	  l = l->next;
  }
  while(r != NULL){
	  eLen ++;
	  r = r->next;
  }
  if (iLen != eLen && eLen != 0){
	 fprintf(stderr, "Error: (line %d) lhs and rhs have different length\n", yylineno);
	 exit(1);
 }

  // create a list of VarSpec based on identifierList and EXP_LIST
  l = lhs;
  r = rhs;
  s->val.assign = NEW(ASSIGN);
  s->val.assign->lhs = lhs->exp;
  s->val.assign->rhs = rhs->exp;
  l = l->next;
  r = r == NULL ? NULL :r->next;
  ASSIGN *cur = s->val.assign;
  ASSIGN *next;
  while(l != NULL){
	  next = NEW(ASSIGN);
	  next->lhs = l->exp;
	  next->rhs = r->exp;
	  cur->next = next;
	  cur = next;
	  l = l->next;
	  r = r == NULL ? NULL :r->next;
  }
  return s;
}

STMT *makeAssignOpStmt(OP_ASSIGN assign_type, EXP *lhs, EXP *rhs){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = assignopK;
  s->val.assignop.assign_type = assign_type;
  s->val.assignop.lhs = lhs;
  s->val.assignop.rhs = rhs;
  return s;
}

STMT *makeDeclareStmt(DEC *declaration){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = declareK;
  s->val.declaration = declaration;
  return s;
}

STMT *makeShortdecStmt(IDENTIFIER_LIST *lhs, EXP_LIST *rhs){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = shortdeclareK;
  VAR_SPEC* vs = makeVarSpec(lhs, NULL, rhs);
  s->val.shortdecvar = vs;
  return s;
}

STMT *makeIncStmt(EXP *inc_exp){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = incK;
  s->val.inc_exp = inc_exp;
  return s;
}

STMT *makeDecStmt(EXP *dec_exp){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = decK;
  s->val.dec_exp = dec_exp;
  return s;
}

STMT *makePrintStmt(EXP_LIST *print_expressions){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = printK;
  s->val.print_expressions = print_expressions;
  return s;
}

STMT *makePrintlnStmt(EXP_LIST *print_expressions){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = printlnK;
  s->val.print_expressions = print_expressions;
  return s;
}

STMT *makeReturnStmt(EXP *ret_exp){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = returnK;
  s->val.ret_exp = ret_exp;
  return s;
}

STMT *makeIfStmt(STMT *optional_stmt, EXP *exp, STMT *block, STMT *optional_else){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = ifK;
  s->val.ifstmt.optional_stmt = optional_stmt;
  s->val.ifstmt.exp = exp;
  s->val.ifstmt.block = block;
  s->val.ifstmt.optional_else = optional_else;
  return s;
}

STMT *makeElseStmt(STMT *else_stmt){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = elseK;
  s->val.else_stmt = else_stmt;
  return s;
}

STMT *makeSwitchStmt(STMT *stmt, EXP *exp, CASE_CLAUSE_LIST *clauses){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = switchK;
  s->val.switchstmt.stmt = stmt;
  s->val.switchstmt.exp = exp;
  s->val.switchstmt.clauses = clauses;
  return s;
}

STMT *makeForStmt(EXP *while_exp, FOR_CLAUSE *clause, STMT *block){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = forK;
  s->val.forstmt.while_exp = while_exp;
  s->val.forstmt.clause = clause;
  s->val.forstmt.block = block;
  return s;
}

STMT *makeBreakStmt(){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = breakK;
  return s;
}

STMT *makeContinueStmt(){
  STMT *s = malloc(sizeof(STMT));
  s->lineno = yylineno;
  s->kind = continueK;
  return s;
}

/********** Expressions **************/
/* EXP_LIST */
EXP_LIST *makeExpList(EXP *exp){
  EXP_LIST *e_list = malloc(sizeof(EXP_LIST));
  e_list->exp = exp;
  e_list->next = NULL;
  return e_list;
}

EXP_LIST *insertExpList(EXP_LIST *list, EXP *exp){

  EXP_LIST *e = makeExpList(exp);
  if (list == NULL) return e;

  EXP_LIST *cur = list;
  while(cur->next != NULL){
    cur = cur->next;
  }
  cur->next = e;

  return list;

}
/* EXP constructors */

EXP *makeIdentifierExp(char *identifier){
  if(strcmp(identifier, "true") == 0)
    return makeBoolExp(1);
  else if (strcmp(identifier, "false") == 0)
    return makeBoolExp(0);

  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = identifierK;
  e->val.identifier.identifier = malloc((strlen(identifier)+1)*sizeof(char));
  strcpy(e->val.identifier.identifier, identifier);
  return e;
}

EXP *makeStringExp(char *stringval){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = stringvalK;
  e->val.stringval = malloc((strlen(stringval)+1)*sizeof(char));
  strcpy(e->val.stringval, stringval);
  e->type = stringTYPE;
  return e;
}

EXP *makeRuneExp(char runeval){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = runevalK;
  e->val.runeval = runeval;
  e->type = runeTYPE;
  return e;
}

EXP *makeIntExp(int intval){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = intvalK;
  e->val.intval = intval;
  e->type = intTYPE;
  return e;

}

EXP *makeFloat64Exp(double float64val){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = float64valK;
  e->val.float64val = float64val;
  e->type = float64TYPE;
  return e;

}

EXP *makeBoolExp(int boolval){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = boolvalK;
  e->val.boolval = boolval;
  e->type = boolTYPE;
  return e;

}

EXP *makeBinaryExp(EXP_Kind kind, EXP *lhs, EXP *rhs){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = kind;
  e->val.binary.lhs = lhs;
  e->val.binary.rhs = rhs;
  return e;
}

EXP *makeUnaryExp(EXP_Kind kind, EXP *child){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = kind;
  e->val.unary.child = child;
  return e;
}

EXP *makeFuncExp(EXP *func_exp, EXP_LIST *args){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = funcK;
  e->val.func.func_exp = func_exp;
  e->val.func.args = args;
  return e;
}

EXP *makeCastExp(TYPE *type, EXP *exp){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = castK;
  e->val.cast.type = type;
  e->val.cast.exp = exp;
  return e;
}

EXP *makeAppendExp(EXP *head, EXP *tail){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = appendK;
  e->val.append.head = head;
  e->val.append.tail = tail;
  return e;
}

EXP *makeArrayIndexExp(EXP *array_exp, EXP *index){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = arrayindexK;
  e->val.array_index.array_exp = array_exp;
  e->val.array_index.index = index;
  return e;
}

EXP *makeSliceIndexExp(EXP *slice_exp, EXP *index){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = arrayindexK;
  e->val.slice_index.slice_exp = slice_exp;
  e->val.slice_index.index = index;
  return e;
}

EXP *makeStructAccessExp(EXP *struct_exp, char *field_name){
  EXP *e = malloc(sizeof(EXP));
  e->lineno = yylineno;
  e->kind = structaccessK;
  e->val.struct_access.struct_exp = struct_exp;
  e->val.struct_access.field_name = malloc((strlen(field_name)+1)*sizeof(char));
  strcpy(e->val.struct_access.field_name, field_name);
  // e->val.struct_access.struct_exp = struct_exp;
  // strcpy(e->val.struct_access.field_name, field_name);
  return e;
}

/********** Types **************/
TYPE *makeType(char *name){
    TYPE *t = malloc(sizeof(TYPE));
    t->lineno = yylineno;
    if (strcmp(name, "infer") == 0)
      t->kind = inferK;
    else{
      t->kind = refK;
      t->val.name = name;
    }

    return t;
}


TYPE *makeSliceType(TYPE* element){
    TYPE *t = malloc(sizeof(TYPE));
    t->lineno = yylineno;
    t->val.sliceElement = element;
	t->kind = sliceK;
    return t;
}

TYPE *makeArrayType(TYPE* element, int size){
    TYPE *t = malloc(sizeof(TYPE));
    t->lineno = yylineno;
    t->val.array.element = element;
    t->val.array.size = size;
	t->kind = arrayK;
    return t;
}

TYPE *makeStructType(FIELD_DECL *fieldDecls){
    TYPE *t = malloc(sizeof(TYPE));
    t->lineno = yylineno;
	t->kind = structK;
	t->val.fieldDecls = fieldDecls;
    return t;
}
