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

static TreeNode *statement(void);

static TreeNode *if_stmt(void);

static TreeNode *repeat_stmt(void);

static TreeNode *assign_stmt(void);

static TreeNode *read_stmt(void);

static TreeNode *write_stmt(void);

static TreeNode *expr(void);

static TreeNode *simple_exp(void);

static TreeNode *term(void);

static TreeNode *factor(void);

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

/*stmt_sequence->stmt_sequence ; stmt | stmt
 * stmt_sequence=stmt(stmt_sequence ;)* */
TreeNode *stmt_sequence(void) {
    TreeNode *t = statement();
    TreeNode *p = t;
    while ((token != ENDFILE) && (token != END) &&
            (token != ELSE) && (token != UNTIL)) {
        TreeNode *q;
        match(SEMI);
        q = statement();
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

/*statement->if_stmt|repeat_stmt|assign_stmt|read_stmt|write_stmt*/
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
    if (t != NULL) t->child[0] = expr();
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
    if (t != NULL) t->child[1] = expr();
    return t;
}

/*assign_stmt->id:=exp*/
TreeNode *assign_stmt(void) {
    TreeNode *t = newStmtNode(AssignK);
    if ((t != NULL) && (token == ID))
        t->attr.name = copyString(tokenString);
    match(ID);
    match(ASSIGN);
    if (t != NULL) t->child[0] = expr();
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
    if (t != NULL) t->child[0] = expr();
    return t;
}

/*expr->simple_exp cop(< =) simple_exp | simple_exp
 * expr=simple_exp (cop simple_exp)* */
TreeNode *expr(void) {
    TreeNode *t = simple_exp();
    if ((token == LT) || (token == EQ)) {
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
            t = newExpNode(ConstK);
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

/****************************************/
/* the primary function of the parser   */
/****************************************/
/* Function parse returns the newly 
 * constructed syntax tree
 */
TreeNode *parse(void) {
    TreeNode *t;
    /*初始化token*/
    token = getToken();
    t = stmt_sequence();
    if (token != ENDFILE)
        syntaxError("Code ends before file\n");
    return t;
}
