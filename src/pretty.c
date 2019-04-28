#include <stdio.h>
#include <stdlib.h>

#include "pretty.h"

int indentation;
#define PRINT_INDENT(...) for(int i=0; i<indentation; i++) printf("   "); printf(__VA_ARGS__);
#define PRINT_BLANK() for(int i=0; i<indentation; i++) printf("   ");

void prettyPROG(PROG *p)
{
  indentation = 0;

  if(p->package != NULL){
    prettyPACKAGE(p->package);
    printf("\n");
  }



  DEC *nextDec = p->root_dec;
  while(nextDec != NULL){
    prettyDEC(nextDec);
    nextDec = nextDec->next;
    printf("\n");
  }

}

void prettyDEC(DEC *d)
{
  if (d == NULL)
    {
      return;
    }
  switch (d->kind)
    {
    case varDeclK:
      prettyVAR_SPEC(d->val.varSpecs);
      break;
    case typeDeclK:
      prettyTYPE_SPEC(d->val.typeSpecs);
      break;
    case funcDeclK:
      PRINT_INDENT("func %s(", d->val.funcSpec->name);
      prettyPARAMETERS(d->val.funcSpec->parameters);
      printf(") ");
      prettyTYPE(d->val.funcSpec->returnType);
      printf("{\n");
      indentation++;
      PRINT_BLANK();
      prettySTMT(d->val.funcSpec->block);
      indentation--;
      printf("\n");
      PRINT_INDENT("}\n");
      break;
    }
}


void prettyVAR_SPEC(VAR_SPEC *vs){
  if (vs != NULL){

    printf("var %s", vs->identifier);
    if (vs->type != NULL){
      printf(" ");
      prettyTYPE(vs->type);
    }
    if (vs->rhs != NULL){
      printf(" = ");
      prettyEXP(vs->rhs);
    }

    if(vs->next){
      printf("\n");
      PRINT_BLANK();
      prettyVAR_SPEC(vs->next);
    }
  }
}



void prettyPACKAGE(PACKAGE *package){
  if (package == NULL)
    {
      return;
    }
  /* printf("package %s\n", package->name); */
  PRINT_INDENT("package %s\n", package->name);
}


void prettyPARAMETERS(PARAMETERS *params){
  if(params == NULL)
    return;

  printf("%s ",params->identifier);
  if(params->type)
    prettyTYPE(params->type);

  if(params->next){
    printf(", ");
    prettyPARAMETERS(params->next);
  }
}

void prettyTYPE_SPEC(TYPE_SPEC *spec){
  if (spec != NULL){
    printf("type %s ", spec->name);
    prettyTYPE(spec->type);

    if(spec->next){
      printf("\n");
      PRINT_BLANK();
      prettyTYPE_SPEC(spec->next);
    }
  }
}



void prettyFOR_CLAUSE(FOR_CLAUSE *clause){
  if(clause == NULL)
    return;
  prettySTMT(clause->init);
  printf("; ");
  prettyEXP(clause->cond);
  printf("; ");
  prettySTMT(clause->post);
}

void prettyCASE_CLAUSE(CASE_CLAUSE *clause){
  if(clause == NULL)
    return;

  switch(clause->kind){
  case caseK:
    PRINT_INDENT("case ");
    prettyEXP_LIST(clause->val.case_clause.cases);
    printf(":\n");
    indentation++;
    prettySTMT_LIST(clause->val.case_clause.clause);
    indentation--;
    break;

  case defaultK:
    PRINT_INDENT("default:\n");
    indentation++;
    prettySTMT_LIST(clause->val.case_clause.clause);
    indentation--;
    break;
  }
}

void prettyCASE_CLAUSE_LIST(CASE_CLAUSE_LIST *clauses){

  if (clauses != NULL){
    prettyCASE_CLAUSE(clauses->clause);
    if (clauses->next != NULL){
      printf("\n");
    }
    prettyCASE_CLAUSE_LIST(clauses->next);
  }
}

void prettySTMT_LIST(STMT_LIST *s_list){
  if (s_list != NULL){
    STMT_LIST *next = s_list->next;
    STMT *s_current = s_list->stmt;
    while(s_current != NULL){
      prettySTMT(s_current);
      if(next == NULL){
        break;
      }
      else{
        printf("\n");
        PRINT_BLANK();
        s_current = next->stmt;
        next = next->next;
      }
    }
  }
}

void prettySTMT(STMT *s)
{
  if (s == NULL)
    return;

  VAR_SPEC *cur;
  ASSIGN *curA;
  /* PRINT_BLANK(); */
  switch (s->kind)
    {
    case emptyK:
      break;
    case blockK:
      if(s->val.block)
        prettySTMT_LIST(s->val.block);
      break;
      prettyEXP(s->val.exp);
      break;

    case assignK:
        curA = s->val.assign;
        while (curA != NULL){
            prettyEXP(curA->lhs);
            if (curA->next != NULL) printf(", ");
            curA = curA->next;
        }

      printf(" = ");

      curA = s->val.assign;
      while (curA != NULL){
          prettyEXP(curA->rhs);
          if (curA->next != NULL) printf(", ");
          curA = curA->next;
      }
      break;

    case expressionK:
      break;

    case assignopK:
      prettyEXP(s->val.assignop.lhs);
      switch(s->val.assignop.assign_type){
      case pluseq:
        printf(" += ");
        break;
      case minuseq:
        printf(" -= ");
        break;
      case bitoreq:
        printf(" |= ");
        break;
      case xoreq:
        printf(" ^= ");
        break;
      case multeq:
        printf(" *= ");
        break;
      case diveq:
        printf(" /= ");
        break;
      case modeq:
        printf(" %%= ");
        break;
      case lshifteq:
        printf(" <<= ");
        break;
      case rshifteq:
        printf(" >>= ");
        break;
      case bitandeq:
        printf(" &= ");
        break;
      case bitcleareq:
        printf(" &^= ");
        break;
      }
      prettyEXP(s->val.assignop.rhs);
      break;
    case declareK:
      prettyDEC(s->val.declaration);
      break;

    case shortdeclareK:
      cur = s->val.shortdecvar;
      while (cur != NULL){
          printf("%s", cur->identifier);
          if (cur->next != NULL) printf(", ");
          cur = cur->next;
      }
      printf(" := ");
      cur = s->val.shortdecvar;
      while (cur != NULL){
          prettyEXP(cur->rhs);
          if (cur->next != NULL) printf(", ");
          cur = cur->next;
      }
      break;

    case incK:
      prettyEXP(s->val.inc_exp);
      printf("++");
      break;
    case decK:
      prettyEXP(s->val.dec_exp);
      printf("--");
      break;
    case printK:
      printf("print(");
      prettyEXP_LIST(s->val.print_expressions);
      printf(")");
      break;
    case printlnK:
      printf("println(");
      prettyEXP_LIST(s->val.print_expressions);
      printf(")");
      break;
    case returnK:
      printf("return ");
      prettyEXP(s->val.ret_exp);
      break;
    case ifK:
      printf("if ");
      prettySTMT(s->val.ifstmt.optional_stmt);
      if (s->val.ifstmt.optional_stmt != NULL) printf(";");
      prettyEXP(s->val.ifstmt.exp);
      printf(" {\n");
      indentation++;
      PRINT_BLANK();
      prettySTMT(s->val.ifstmt.block);
      printf("\n");
      indentation--;
      PRINT_INDENT("}");
      prettySTMT(s->val.ifstmt.optional_else);
      break;
    case elseK:
      printf("else {\n");
      indentation++;
      PRINT_BLANK();
      prettySTMT(s->val.else_stmt);
      indentation--;
      printf("\n");
      PRINT_INDENT("}\n");
      /* for(int i=0; i<indentation; i++) printf("   "); */
      break;
    case switchK:
      printf("switch ");
      prettySTMT(s->val.switchstmt.stmt);
      if (s->val.switchstmt.stmt != NULL){
        printf(";");
      }
      prettyEXP(s->val.switchstmt.exp);
      printf(" {\n");
      indentation++;
      prettyCASE_CLAUSE_LIST(s->val.switchstmt.clauses);
      printf("\n");
      PRINT_INDENT("}");
      break;
    case forK:
      printf("for ");
      if(s->val.forstmt.while_exp)
        prettyEXP(s->val.forstmt.while_exp);
      else if(s->val.forstmt.clause)
        prettyFOR_CLAUSE(s->val.forstmt.clause);
      printf(" {\n");
      indentation++;
      PRINT_BLANK();
      prettySTMT(s->val.forstmt.block);
      printf("\n");
      indentation--;
      PRINT_INDENT("}");
      break;
    case breakK:
      printf("break");
      break;
    case continueK:
      printf("continue");
      break;
    }
}

void prettyEXP_LIST(EXP_LIST *e_list){
  if (e_list == NULL) return;
  EXP_LIST *next = e_list->next;
  EXP *e_current = e_list->exp;
  while(e_current != NULL){
    prettyEXP(e_current);
    if(next == NULL){
      break;
    }
    else{
      printf(", ");
      e_current = next->exp;
      next = next->next;
    }
  }
}

void prettyEXP(EXP *e)
{
  if (e == NULL)
    {
      return;
    }
  switch (e->kind) {
  case orK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" || ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case andK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" && ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case eqK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" == ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case neqK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" != ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case ltK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" < ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case gtK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" > ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case leqK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" <= ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case geqK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" >= ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case plusK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" + ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case minusK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" - ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case bitorK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" | ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case xorK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" ^ ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case multK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" * ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case divideK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" / ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case modK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" %% ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case lshiftK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" << ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case rshiftK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" >> ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case bitandK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" & ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case bitclearK:
    printf("(");
    prettyEXP(e->val.binary.lhs);
    printf(" &^ ");
    prettyEXP(e->val.binary.rhs);
    printf(")");
    break;
  case uminusK:
    printf("-");
    prettyEXP(e->val.unary.child);
    break;
  case uplusK:
    printf("+");
    prettyEXP(e->val.unary.child);
    break;
  case ubangK:
    printf("!");
    prettyEXP(e->val.unary.child);
    break;
  case uxorK:
    printf("^");
    prettyEXP(e->val.unary.child);
    break;
  case identifierK:
    printf("%s", e->val.identifier.identifier);
    break;
  case float64valK:
    printf("%f", e->val.float64val);
    break;
  case runevalK:
    printf("%c", e->val.runeval);
    break;
  case intvalK:
    printf("%i", e->val.intval);
    break;
  case stringvalK:
    printf("%s", e->val.stringval);
    break;
  case boolvalK:
    e->val.boolval ? printf("true") : printf("false");
    break;
  case funcK:
    prettyEXP(e->val.func.func_exp);
    printf("(");
    prettyEXP_LIST(e->val.func.args);
    printf(")");
    break;
  case castK:
    // printf("%s(", e->val.cast.type->name);
    // type could be a base type, which does not have attribute name (name is actually ref name for ref type)
    prettyTYPE(e->val.cast.type);
    printf("(");
    prettyEXP(e->val.cast.exp);
    printf(")");
    break;
  case appendK:
    printf("append(");
    prettyEXP(e->val.append.head);
    printf(", ");
    prettyEXP(e->val.append.tail);
    printf(")");
    break;

  case arrayindexK:
    prettyEXP(e->val.array_index.array_exp);
    printf("[");
    prettyEXP(e->val.array_index.index);
    printf("]");
    break;
  case sliceindexK:
    prettyEXP(e->val.slice_index.slice_exp);
    printf("[");
    prettyEXP(e->val.slice_index.index);
    printf("]");
    break;
  case structaccessK:
    prettyEXP(e->val.struct_access.struct_exp);
    printf(".%s",e->val.struct_access.field_name);
    break;
  default:
    printf("Error (line %d): Unrecognized Expression", e->lineno);
    exit(1);
  }
}

void prettyTYPE(TYPE *t){
  if(t == NULL)
    return;

  switch(t->kind){
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
  case refK:    // change: use refK for reference type
    printf("%s", t->val.name);
    break;
  case sliceK:
    printf("[]");
    prettyTYPE(t->val.sliceElement);
    break;
  case arrayK:
    printf("[%d]", t->val.array.size);
    prettyTYPE(t->val.array.element);
    break;
  case structK:
    printf(" struct {\n");
    indentation++;
    prettyFIELD_DECL_LIST(t->val.fieldDecls);
    indentation--;
    PRINT_INDENT("}\n");
  case inferK:
    printf("<infer>");
    break;
  }

}

void prettyFIELD_DECL_LIST(FIELD_DECL *list){
  if(list != NULL){
    PRINT_BLANK();
    printf("%s ", list->identifier);
    prettyTYPE(list->type);
    printf("\n");
    prettyFIELD_DECL_LIST(list->next);
  }
}

void prettyIDENTIFIER_LIST(IDENTIFIER_LIST *id_list){
  if(id_list == NULL)
    return;
  if(id_list->identifier)
    printf("%s ",id_list->identifier);

  if(id_list->next){
    printf(", ");
    prettyIDENTIFIER_LIST(id_list->next);
  }

}
