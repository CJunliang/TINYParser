/****************************************************/
/* File: util.c                                     */
/* Utility function implementation                  */
/* for the TINY compiler                            */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"
#include "util.h"


/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType token, const char *tokenString) {
    switch (token) {
        case IF:
        case THEN:
        case ELSE:
        case END:
        case REPEAT:
        case UNTIL:
        case READ:
        case WRITE:
        case T_TRUE:
        case T_FALSE:
        case OR:
        case AND:
        case NOT:
        case INT:
        case BOOL:
        case STRING:
        case DO:
        case WHILE:
            fprintf(listing,
                    "reserved word: %s\n", tokenString);
            break;
        case ASSIGN:
            fprintf(listing, ":=\n");
            break;
        case LT:
            fprintf(listing, "<\n");
            break;
        case EQ:
            fprintf(listing, "=\n");
            break;
        case GT:
            fprintf(listing, ">\n");
            break;
        case LTE:
            fprintf(listing, "<=\n");
            break;
        case GTE:
            fprintf(listing, ">=\n");
            break;
        case LPAREN:
            fprintf(listing, "(\n");
            break;
        case RPAREN:
            fprintf(listing, ")\n");
            break;
        case SEMI:
            fprintf(listing, ";\n");
            break;
        case COMMA:
            fprintf(listing, ",\n");
            break;
        case SQM:
            fprintf(listing, "\'\n");
            break;
        case PLUS:
            fprintf(listing, "+\n");
            break;
        case MINUS:
            fprintf(listing, "-\n");
            break;
        case TIMES:
            fprintf(listing, "*\n");
            break;
        case OVER:
            fprintf(listing, "/\n");
            break;
        case ENDFILE:
            fprintf(listing, "EOF\n");
            break;
        case NUM:
            fprintf(listing,
                    "NUM, val= %s\n", tokenString);
            break;
        case ID:
            fprintf(listing,
                    "ID, name= %s\n", tokenString);
            break;
        case STR:
            fprintf(listing, "STR, name= %s\n", tokenString);
            break;
        case ERROR:
            fprintf(listing, "ERROR %s: %s\n", errorMsg[errorCode], tokenString);
            break;
        default: /* should never happen */
            fprintf(listing, "Unknown token: %d\n", token);
    }
}

/* Function newStmtNode creates a new statement
 * node for syntax tree construction
 * 函数newStmtNode创建一个新语句节点，用于语法树构建
 * IfK, RepeatK, AssignK, ReadK, WriteK
 */
TreeNode *newStmtNode(StmtKind kind) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = StmtK;
        t->kind.stmt = kind;
        t->lineno = lineno;
    }
    return t;
}

/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 * 函数newExpNode创建一个新的表达式节点以构建语法树
 * OpK, ConstK, IdK
 */
TreeNode *newExpNode(ExpKind kind) {
    TreeNode *t = (TreeNode *) malloc(sizeof(TreeNode));
    int i;
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else {
        for (i = 0; i < MAXCHILDREN; i++) t->child[i] = NULL;
        t->sibling = NULL;
        t->nodekind = ExpK;
        t->kind.exp = kind;
        t->lineno = lineno;
        t->type = Void;
    }
    return t;
}

/* Function copyString allocates and makes a new
 * copy of an existing string
 * 函数copyString分配并创建现有字符串的新副本
 */
char *copyString(char *s) {
    int n;
    char *t;
    if (s == NULL) return NULL;
    n = strlen(s) + 1;
    t = malloc(n);
    if (t == NULL)
        fprintf(listing, "Out of memory error at line %d\n", lineno);
    else strcpy(t, s);
    return t;
}

/* Variable indentno is used by printTree to
 * store current number of spaces to indent
 * printTree使用变量indentno来存储要缩进的当前空间数
 */
static int indentno = 0;

/* macros to increase/decrease indentation */
#define INDENT indentno+=2
#define UNINDENT indentno-=2

/* printSpaces indents by printing spaces */
static void printSpaces(void) {
    int i;
    for (i = 0; i < indentno; i++)
        fprintf(listing, " ");
}

/* procedure printTree prints a syntax tree to the 
 * listing file using indentation to indicate subtrees
 * 过程printTree使用缩进将语法树打印到清单文件中，以指示子树
 */
void printTree(TreeNode *tree) {
    int i;
    /*增加缩进*/
    INDENT;
    while (tree != NULL) {
        printSpaces();
        /*语句节点*/
        if (tree->nodekind == StmtK) {
            switch (tree->kind.stmt) {
                case IfK:
                    fprintf(listing, "If\n");
                    break;
                case RepeatK:
                    fprintf(listing, "Repeat\n");
                    break;
                case AssignK:
                    fprintf(listing, "Assign to: %s\n", tree->attr.name);
                    break;
                case ReadK:
                    fprintf(listing, "Read: %s\n", tree->attr.name);
                    break;
                case WriteK:
                    fprintf(listing, "Write\n");
                    break;
                case WhileK:
                    /*新添加WhileK来输出do-while语句*/
                    fprintf(listing, "While\n");
                    break;
                default:
                    fprintf(listing, "Unknown StmtNode kind\n");
                    break;
            }
        } else if (tree->nodekind == ExpK) {
            /*表达式节点*/
            switch (tree->kind.exp) {
                case OpK:
                    fprintf(listing, "Op: ");
                    printToken(tree->attr.op, "\0");
                    break;
                case TypeK:
                    /*输出数据类型*/
                    fprintf(listing, "Type: %s\n", tree->attr.name);
                    break;
                case ConstNumK:
                    /*输出常数*/
                    fprintf(listing, "Const Integer: %d\n", tree->attr.val);
                    break;
                case ConstStrK:
                    /*输出字符串*/
                    if (strcmp(tree->attr.string, "Program") == 0)
                        /*输出Program*/
                        fprintf(listing, "%s\n", tree->attr.string);
                    else
                        fprintf(listing, "Const String: %s\n", tree->attr.string);
                    break;
                case IdK:
                    fprintf(listing, "Id: %s\n", tree->attr.name);
                    break;
                default:
                    fprintf(listing, "Unknown ExpNode kind\n");
                    break;
            }
        } else fprintf(listing, "Unknown node kind\n");
        /*打印子树*/
        for (i = 0; i < MAXCHILDREN; i++)
            printTree(tree->child[i]);
        /*让当前节点变成兄弟节点*/
        tree = tree->sibling;
    }
    /*减少缩进*/
    UNINDENT;
}

int isLegalChar(char c) {
    return (isalnum(c) ||
            isspace(c) ||
            c == '>' ||
            c == '<' ||
            c == '=' ||
            c == ',' ||
            c == ';' ||
            c == '\'' ||
            c == '{' ||
            c == '}' ||
            c == '+' ||
            c == '-' ||
            c == '*' ||
            c == '/' ||
            c == '(' ||
            c == ')'
    );
}