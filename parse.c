/****************************************************/
/* File: parse.c                                    */
/* The parser implementation for the TINY compiler  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

static TokenType token; /* holds current token */

/* function prototypes for recursive calls */
/*递归调用的函数原型*/
static TreeNode *stmt_sequence(void);

static TreeNode *declarations(void);

static TreeNode *decl(void);

static TreeNode *type_specifier(void);

static TreeNode *varList(void);

static TreeNode *statement(void);

static TreeNode *if_stmt(void);

static TreeNode *repeat_stmt(void);

static TreeNode *assign_stmt(void);

static TreeNode *read_stmt(void);

static TreeNode *write_stmt(void);

static TreeNode *while_stmt(void);

static TreeNode *expr(void);

static TreeNode *simple_exp(void);

static TreeNode *term(void);

static TreeNode *factor(void);

static TreeNode *andTerm(void);

static TreeNode *orTerm(void);

static TreeNode *notTerm(void);

static TreeNode *boolFactor(void);

/*判断是否是正则运算还是布尔运算，1时代表正则，0代表布尔*/
static int inExp = 0;

/*输出语法错误*/
static void syntaxError(char *message) {
    fprintf(listing, "\n>>> ");
    fprintf(listing, "Syntax error at line %d: %s", lineno, message);
    Error = TRUE;
}

/*获取下一个token*/
static void match(TokenType expected) {
    if (token == expected) token = getToken();
    else {
        syntaxError("unexpected token (from match)-> ");
        printToken(token, tokenString);
        fprintf(listing, "      ");
    }
}

/*declarations->decl ; declarations | ε
 * 声明语句*/
TreeNode *declarations(void) {
    TreeNode *t = NULL;
    TreeNode *p;
    /*当token是int或string或bool时说明该语句是声明语句*/
    while (token == INT || token == STRING || token == BOOL) {
        TreeNode *q;
        if (t == NULL) {
            /*t等于null说明此时是第一条声明语句*/
            t = decl();
            p = t;
        } else {
            /*此时非第一条声明语句，需要不断挂靠在兄弟节点上*/
            q = decl();
            if (q != NULL) {
                p->sibling = q;
                p = q;
            }
        }
        /*匹配分号*/
        match(SEMI);
    }
    return t;
}

/*decl->type-specifier varlist
 * 声明语句*/
TreeNode *decl(void) {
    TreeNode *t = NULL;
    /*数据类型*/
    t = type_specifier();
    /*变量*/
    if (t != NULL)
        t->child[0] = varList();
    return t;
}

/*PARSE+    type_specifier->int | bool | string
 * 变量类型，只能匹配int、bool和string*/
TreeNode *type_specifier(void) {
    TreeNode *t = newStmtNode(TypeK);
    switch (token) {
        case INT:
            t->attr.name = "int";
            match(INT);
            break;
        case BOOL:
            t->attr.name = "bool";
            match(BOOL);
            break;
        case STRING:
            t->attr.name = "string";
            match(STRING);
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
    }
    return t;
}

/*PARSE+    varList->id(,id)*
 * 变量*/
TreeNode *varList(void) {
    TreeNode *t = newExpNode(IdK);
    /*当token是ID时*/
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    /*当token为逗号时，说明声明语句不止一个变量*/
    if (token == COMMA) {
        match(COMMA);
        /*将变量挂靠在前一个变量的兄弟节点*/
        t->child[0] = varList();
    }
    return t;
}

/*stmt_sequence->stmt_sequence ; stmt | stmt
 * stmt_sequence=stmt(stmt_sequence ;)*
 * 在这里改变了产生式为stmt_sequence->stmt;stmt_sequence|stmt;*/
TreeNode *stmt_sequence(void) {
    TreeNode *t = statement();
    match(SEMI);
    TreeNode *p = t;
    /*需要加一个token!=while*/
    while ((token != ENDFILE) && (token != END) &&
           (token != ELSE) && (token != UNTIL) && (token != WHILE)) {
        TreeNode *q;
        q = statement();
        if (token != ENDFILE)
            match(SEMI);
        else break;
        if (q != NULL) {
            if (t == NULL) t = p = q;
            else /* now p cannot be NULL either */
            {
                p->sibling = q;
                p = q;
            }
        }
    }
    return t;
}

/*statement->if_stmt | repeat_stmt | assign_stmt | read_stmt | write_stmt*/
TreeNode *statement(void) {
    TreeNode *t = NULL;
    switch (token) {
        case IF :
            t = if_stmt();
            break;
        case REPEAT :
            t = repeat_stmt();
            break;
        case ID :
            t = assign_stmt();
            break;
        case READ :
            t = read_stmt();
            break;
        case WRITE :
            t = write_stmt();
            break;
        case DO:
            /*do-while语句*/
            t = while_stmt();
            break;
        default :
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
    } /* end case */
    return t;
}

/*if_stmt->if exp then stmt_seq end | if exp then stmt_seq else stmt_seq end*/
TreeNode *if_stmt(void) {
    TreeNode *t = newStmtNode(IfK);
    match(IF);
    if (t != NULL) t->child[0] = orTerm();
    match(THEN);
    if (t != NULL) t->child[1] = stmt_sequence();
    if (token == ELSE) {
        match(ELSE);
        if (t != NULL) t->child[2] = stmt_sequence();
    }
    match(END);
    return t;
}

/*repeat_stmt->repeat stmt_seq until exp*/
TreeNode *repeat_stmt(void) {
    TreeNode *t = newStmtNode(RepeatK);
    match(REPEAT);
    if (t != NULL) t->child[0] = stmt_sequence();
    match(UNTIL);
    t->child[1] = expr();
    return t;
}

/*assign_stmt->id:=exp*/
TreeNode *assign_stmt(void) {
    TreeNode *t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    match(ASSIGN);
    if (t != NULL) {
        switch (token) {
            case STR:
                /*当token是str时，说明该值是字符串*/
                t->child[0] = newExpNode(ConstStrK);
                if ((t != NULL) && (token == STR))
                    t->child[0]->attr.string = copyString(tokenString);
                match(STR);
                break;
            case NUM:
            case ID:
            case T_TRUE:
            case T_FALSE:
                t->child[0] = orTerm();
                break;
            default:
                break;
        }
    }
    return t;
}

/*read_stmt->read ID*/
TreeNode *read_stmt(void) {
    TreeNode *t = newStmtNode(ReadK);
    match(READ);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    return t;
}

/*write_stmt->write exp*/
TreeNode *write_stmt(void) {
    TreeNode *t = newStmtNode(WriteK);
    match(WRITE);
    if (t != NULL) {
        inExp = 1;
        t->child[0] = simple_exp();
        inExp = 0;
    }
    return t;
}

/* while_stmt->do stmt_sequence while bool_exp
 * do-while语句 */
TreeNode *while_stmt(void) {
    TreeNode *t = newStmtNode(WhileK);
    match(DO);
    if (t != NULL) t->child[0] = stmt_sequence();
    match(WHILE);
    if (t != NULL) t->child[1] = orTerm();
    return t;
}

/*expr->simple_exp cop(< =) simple_exp | simple_exp
 * expr=simple_exp (cop simple_exp)* */
TreeNode *expr(void) {
    TreeNode *t = simple_exp();
    /*除了<,=外，还需要加上>,<=,>=*/
    if ((token == LT) || (token == EQ) || (token == GT) || (token == LTE) || (token == GTE)) {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
        }
        match(token);
        if (t != NULL)
            t->child[1] = simple_exp();
    }
    return t;
}

/*simple_exp->simple_exp add_op(+ -) term|term
 * simple_exp=term (add_op term)+ */
TreeNode *simple_exp(void) {
    TreeNode *t = term();
    while ((token == PLUS) || (token == MINUS)) {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            t->child[1] = term();
        }
    }
    return t;
}

/*term->term|term mul_op(* /) factor
 * term=factor(mul_op factor)+*/
TreeNode *term(void) {
    TreeNode *t = factor();
    /*token is * or /*/
    while ((token == TIMES) || (token == OVER)) {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = factor();
        }
    }
    return t;
}

/*(exp)|num|id*/
TreeNode *factor(void) {
    TreeNode *t = NULL;
    switch (token) {
        case NUM :
            t = newExpNode(ConstNumK);
            /*atoi函数将字符串转换成数字*/
            if ((t != NULL) && (token == NUM))
                t->attr.val = atoi(tokenString);
            match(NUM);
            break;
        case ID :
            t = newExpNode(IdK);
            if ((t != NULL) && (token == ID))
                t->attr.name = copyString(tokenString);
            match(ID);
            break;
        case LPAREN :
            /*左括号(*/
            match(LPAREN);
            if (inExp == 1)
                t = simple_exp();
            else
                t = expr();
            /*右括号)*/
            match(RPAREN);
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
    }
    return t;
}

TreeNode *orTerm(void) {
    TreeNode *t = andTerm();
    while (token == OR) {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = andTerm();
        }
    }
    return t;
}

TreeNode *andTerm(void) {
    TreeNode *t = notTerm();
    while (token == AND) {
        TreeNode *p = newExpNode(OpK);
        if (p != NULL) {
            p->child[0] = t;
            p->attr.op = token;
            t = p;
            match(token);
            p->child[1] = notTerm();
        }
    }
    return t;
}

TreeNode *notTerm(void) {
    TreeNode *t = NULL;
    switch (token) {
        case NOT:
            t = newExpNode(OpK);
            t->attr.op = token;
            match(NOT);
            t->child[0] = notTerm();
            break;
        case T_TRUE:
            t = newExpNode(BoolK);
            if ((t != NULL) && (token == T_TRUE))t->attr.string = copyString(tokenString);
            match(T_TRUE);
            break;
        case T_FALSE:
            t = newExpNode(BoolK);
            if ((t != NULL) && (token == T_FALSE))t->attr.string = copyString(tokenString);
            match(T_FALSE);
            break;
        case LPAREN:
            match(LPAREN);
            t = orTerm();
            match(RPAREN);
            break;
        case NUM:
        case ID:
            t = expr();
            break;
        default:
            syntaxError("unexpected token -> ");
            printToken(token, tokenString);
            token = getToken();
            break;
    }
    return t;
}

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly 
 * constructed syntax tree
 */
TreeNode *parse(void) {
    TreeNode *t, *p;
    token = getToken();
    t = declarations();
    if (t == NULL) {
        t = stmt_sequence();
    } else {
        p = t;
        while (p->sibling != NULL)
            p = p->sibling;
        p->sibling = stmt_sequence();
    }
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
