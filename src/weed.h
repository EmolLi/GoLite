
typedef enum {
    InSwitch, InLoop, null
} weedSTMTTag;
void weedPROGRAM(PROG *p);
void weedDeclaration(DEC *d);
void weedTypeSpec(TYPE_SPEC *ts);
void weedVarSpec(VAR_SPEC *vs);
void weedParameters(PARAMETERS *p);
void weedStatementList(STMT_LIST *b, weedSTMTTag label);
void weedStatement(STMT *s, weedSTMTTag label);
void weedStatementSwitch(STMT* s, weedSTMTTag label);
void weedStatementAssign(STMT *s, weedSTMTTag label);
void weedStatementShortDecVar(STMT *s, weedSTMTTag label);
void weedEXP(EXP* exp, int allowBlankIdentifier);
void weedEXP_LIST(EXP_LIST *list, int allowBlankIdentifier);
void weedTYPE(TYPE* t, int allowBlankIdentifier);
void weedFieldDecl(FIELD_DECL *vs);
void weedForClause(FOR_CLAUSE* c);

bool weedStatementListTerminates(STMT_LIST *b);
bool weedStatementTerminate(STMT *s);
bool weedStatementListForBlockTerminates(STMT_LIST *b);
bool weedStatementForBlockTerminate(STMT *s);
bool weedStatementSwitchTerminates(STMT* s);
