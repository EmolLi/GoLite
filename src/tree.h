#ifndef __tree_h
#define __tree_h

#include <stdbool.h>

typedef struct TYPE TYPE;
typedef struct BASE_TYPE BASE_TYPE;
typedef struct FIELD_DECL FIELD_DECL;
typedef struct EXP EXP;
typedef struct EXP_LIST EXP_LIST;
typedef struct CASE_CLAUSE CASE_CLAUSE;
typedef struct FOR_CLAUSE FOR_CLAUSE;
typedef struct CASE_CLAUSE_LIST CASE_CLAUSE_LIST;
typedef struct STMT STMT;
typedef struct STMT_LIST STMT_LIST;
typedef struct ASSIGN ASSIGN;
typedef struct VAR_SPEC VAR_SPEC;
typedef struct TYPE_SPEC TYPE_SPEC;
typedef struct FUNC_SPEC FUNC_SPEC;
typedef struct CONST_SPEC CONST_SPEC;

typedef struct PARAMETERS PARAMETERS;
typedef struct PACKAGE PACKAGE;
typedef struct IDENTIFIER_LIST IDENTIFIER_LIST;
typedef struct DEC DEC;
typedef struct PROG PROG;

typedef struct SYMBOL SYMBOL;

typedef enum {
  varSymK, typeSymK, funcSymK, fieldSymK
} SYMBOL_Kind;


typedef enum {
  intK, float64K, runeK, stringK, boolK, structK, sliceK, arrayK, refK, inferK
} TYPE_Kind;

typedef enum {
  orK, andK, eqK, neqK, ltK, gtK, leqK, geqK, plusK, minusK, bitorK, xorK,
  multK, divideK, modK, lshiftK, rshiftK, bitandK, bitclearK, uminusK, uplusK, ubangK, uxorK,
  identifierK, float64valK, runevalK, intvalK, stringvalK, boolvalK, funcK, castK, appendK,
  arrayindexK, sliceindexK, structaccessK
} EXP_Kind;

typedef enum {
  emptyK, blockK, expressionK, assignK, assignopK, declareK, shortdeclareK, incK, decK, printK, printlnK, returnK,
  ifK, elseK, switchK, forK, breakK, continueK
} STMT_Kind;


typedef enum {
  pluseq,minuseq,bitoreq,xoreq,multeq,diveq,modeq,lshifteq,rshifteq,bitandeq,bitcleareq
} OP_ASSIGN;

typedef enum {
  caseK, defaultK
} CASE_CLAUSE_Kind;


typedef enum {
  typeDeclK, funcDeclK, varDeclK
} DEC_Kind;

typedef struct SYMBOL {
  char *identifier;
  SYMBOL_Kind kind;
  int shadowNum;
  union {
    TYPE_SPEC* typeSpec;
    VAR_SPEC* varSpec; // var decl and shortvardecl
    FUNC_SPEC* funcSpec;
    FIELD_DECL* structSpec;
  } val;
  struct SYMBOL *next;
} SYMBOL;

struct DEC {
    int lineno;
    DEC *next;
    DEC_Kind kind;
  union{
    TYPE_SPEC *typeSpecs;
    VAR_SPEC  *varSpecs;
    FUNC_SPEC *funcSpec;
  } val;
};


struct FUNC_SPEC {
    char* name;
    int lineno;
    PARAMETERS* parameters;
    STMT* block;
    TYPE* returnType;
};

struct TYPE_SPEC {
    int lineno;
    TYPE *type;
    char* name;
    TYPE_SPEC *next;
};



struct PACKAGE{
    int lineno;
    char *name;
};

struct PROG {
    int lineno;
    PACKAGE *package;
    DEC *root_dec;
};

PACKAGE *makePackage(char* name);
PROG *makeProgram(PACKAGE* package, DEC *dec);
DEC *makeDecls(DEC *cur, DEC *next);
DEC *makeTypeDecl(TYPE_SPEC *specs);
TYPE_SPEC *makeTypeSpec(TYPE *type, char* name);
TYPE_SPEC *makeTypeSpecList(TYPE_SPEC *specs, TYPE_SPEC *next);


struct IDENTIFIER_LIST {
  char *identifier;
  IDENTIFIER_LIST *next;
};

struct VAR_SPEC {
  int lineno;
  char* identifier;
  int shadowNum;
  EXP *rhs;  // optional
  TYPE *type; //Optional
  VAR_SPEC *next;
  bool declared;
};

struct FIELD_DECL {
  int lineno;
  char* identifier;
  TYPE *type;
  FIELD_DECL *next;
};

DEC *makeVarDecl(VAR_SPEC *specs);
IDENTIFIER_LIST *makeIdentifierList(IDENTIFIER_LIST *ilist, char* identifier);
VAR_SPEC *makeVarSpec(IDENTIFIER_LIST *ilist, TYPE *type, EXP_LIST* elist);
VAR_SPEC *insertVarSpecList(VAR_SPEC *list, VAR_SPEC *v);
FIELD_DECL *makeFieldDecl(IDENTIFIER_LIST *ilist, TYPE *type);
FIELD_DECL *insertFieldDeclList(FIELD_DECL *list, FIELD_DECL *v);

// Function decls
struct PARAMETERS{
    int lineno;
    char* identifier;
    int shadowNum;
    TYPE *type; //Optional
    PARAMETERS *next;
};

DEC *makeFunction(char* name, PARAMETERS* parameters, STMT* block, TYPE* returnType);
PARAMETERS* makeParameters(PARAMETERS* plist, IDENTIFIER_LIST* ilist, TYPE* type);


/* FOR_CLAUSE, CASE_CLAUSE & CASE_CLAUSE_LIST */
typedef struct FOR_CLAUSE {
  int lineno;
  STMT *init;
  EXP *cond;
  STMT *post;
} FOR_CLAUSE;
FOR_CLAUSE *makeForClause(STMT *init, EXP *cond, STMT *post);

typedef struct CASE_CLAUSE_LIST {
  CASE_CLAUSE *clause;
  CASE_CLAUSE_LIST *next;
} CASE_CLAUSE_LIST;
CASE_CLAUSE_LIST *makeCaseClauseList(CASE_CLAUSE *case_clause);
CASE_CLAUSE_LIST *insertCaseClauseList(CASE_CLAUSE_LIST *list, CASE_CLAUSE *case_clause);

typedef struct CASE_CLAUSE {
  int lineno;
  CASE_CLAUSE_Kind kind;
  union{
    struct {EXP_LIST *cases; STMT_LIST *clause;} case_clause;
    STMT_LIST *default_clause;
  } val;
} CASE_CLAUSE;
CASE_CLAUSE *makeCaseClause(EXP_LIST *cases, STMT_LIST *clause);
CASE_CLAUSE *makeDefaultClause(STMT_LIST *default_clause);



/* STMT & STMT_LIST */
typedef struct STMT_LIST {
  STMT *stmt;
  STMT_LIST *next;
} STMT_LIST;
STMT_LIST *makeStmtList(STMT *stmt);
STMT_LIST *insertStmtList(STMT_LIST *list, STMT *stmt);


typedef struct STMT {
  int lineno;
  STMT_Kind kind;
  union{
    DEC *declaration;
    EXP *exp; // simpleStmt
    EXP *inc_exp; // simpleStmt incdec
    EXP *dec_exp; // simpleStmt incdec
    EXP *ret_exp; // simpleStmt
    ASSIGN* assign;
    struct {EXP *lhs; EXP *rhs; OP_ASSIGN assign_type;} assignop;
    VAR_SPEC* shortdecvar; // simpleStmt
    struct {STMT *stmt; EXP *exp; CASE_CLAUSE_LIST *clauses; } switchstmt;
    struct {EXP *while_exp; FOR_CLAUSE *clause; STMT *block;} forstmt;
    EXP_LIST *print_expressions; /* print/println */
    STMT_LIST *block;
    struct {STMT *optional_stmt; EXP *exp; STMT *block; STMT *optional_else;} ifstmt;
    STMT *else_stmt;
  } val;
} STMT;

struct ASSIGN{
    EXP* lhs;
    EXP* rhs;
    ASSIGN* next;
};


STMT *makeEmptyStmt();
STMT *makeBlockStmt(STMT_LIST *block);
STMT *makeExpStmt(EXP *exp);
STMT *makeAssignOpStmt(OP_ASSIGN assign_type, EXP *lhs, EXP *rhs);
STMT *makeAssignStmt(EXP_LIST *lhs, EXP_LIST *rhs);
STMT *makeDeclareStmt(DEC *declaration);
STMT *makeShortdecStmt(IDENTIFIER_LIST *lhs, EXP_LIST *rhs);
STMT *makeIncStmt(EXP *inc_exp);
STMT *makeDecStmt(EXP *dec_exp);
STMT *makePrintStmt(EXP_LIST *print_expressions);
STMT *makePrintlnStmt(EXP_LIST *print_expressions);
STMT *makeReturnStmt(EXP *ret_exp);
STMT *makeIfStmt(STMT *optional_stmt, EXP *exp, STMT *block, STMT *optional_else);
STMT *makeElseStmt(STMT *else_stmt);
STMT *makeSwitchStmt(STMT *stmt, EXP *exp, CASE_CLAUSE_LIST *clauses);
STMT *makeForStmt(EXP *while_exp, FOR_CLAUSE *clause, STMT *block);
STMT *makeBreakStmt();
STMT *makeContinueStmt();



/* EXP & EXP_LIST */
typedef struct EXP_LIST {
  EXP *exp;
  EXP_LIST *next;
} EXP_LIST;
EXP_LIST *makeExpList(EXP *exp);
EXP_LIST *insertExpList(EXP_LIST *list, EXP *exp);


typedef struct EXP {
  int lineno;
  EXP_Kind kind;
  union {
    struct {char *identifier; bool shadowed; int shadowNum; SYMBOL* s;} identifier;
    char *stringval;
    char runeval;
    int intval;
    double float64val;
    int boolval;
    struct { EXP *lhs; EXP *rhs; } binary;
    struct { EXP *child; } unary;
    struct { EXP *func_exp; EXP_LIST *args; FUNC_SPEC *func_spec;} func; /* func_spec set in symbol phase */
    struct { TYPE *type; EXP *exp; } cast;
    struct { EXP *head; EXP *tail; } append;
    struct { EXP *array_exp; EXP *index; } array_index;
    struct { EXP *slice_exp; EXP *index; } slice_index;
    struct { EXP *struct_exp; char *field_name; } struct_access;
  } val;
  TYPE *type;
} EXP;

EXP *makeIdentifierExp(char *identifier);
EXP *makeStringExp(char *stringval);
EXP *makeRuneExp(char runeval);
EXP *makeIntExp(int intval);
EXP *makeFloat64Exp(double float64val);
EXP *makeBoolExp(int boolval);
EXP *makeBinaryExp(EXP_Kind kind, EXP *lhs, EXP *rhs);
EXP *makeUnaryExp(EXP_Kind kind, EXP *child);
EXP *makeFuncExp(EXP *func_exp, EXP_LIST *args);
EXP *makeCastExp(TYPE *type, EXP *exp);
EXP *makeAppendExp(EXP *head, EXP *tail);
EXP *makeArrayIndexExp(EXP *array_exp, EXP *index);
EXP *makeSliceIndexExp(EXP *slice_exp, EXP *index);
EXP *makeStructAccessExp(EXP *struct_exp, char *field_name);


/* TYPE */
typedef struct TYPE {
  int lineno;
  TYPE_Kind kind;
  union {
        char *name; // ref name
        struct {int size; TYPE *element;} array;
        TYPE* sliceElement;
        FIELD_DECL *fieldDecls;  // for struct definition
    } val;
  TYPE_SPEC *spec; /* symbol TODO: add this in symbol phase, store type definition*/
} TYPE;



TYPE *makeType(char *name);
TYPE *makeSliceType(TYPE* element);
TYPE *makeArrayType(TYPE* element, int size);
TYPE *makeStructType(FIELD_DECL *fieldDecls);
extern TYPE *runeTYPE, *intTYPE, *boolTYPE, *float64TYPE, *stringTYPE;
void initTypes();
#endif
