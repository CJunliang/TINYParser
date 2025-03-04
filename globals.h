/****************************************************/
/* File: globals.h                                  */
/* Global types and vars for TINY compiler          */
/* must come before other include files             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

/* MAXRESERVED = the number of reserved words */
#define MAXRESERVED 18

typedef enum
/* book-keeping tokens */
{
    ENDFILE, ERROR,
    /* reserved words */
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
    T_TRUE, T_FALSE, OR, AND, NOT, INT, BOOL, STRING, DO, WHILE,
    /* multicharacter tokens */
    ID, NUM, STR,
    /* special symbols */
    ASSIGN, EQ, LT, GT, LTE, GTE, PLUS, MINUS, TIMES, OVER, LPAREN, RPAREN, SEMI, COMMA, SQM
} TokenType;

extern FILE *source; /* source code text file */
extern FILE *listing; /* listing output text file */
extern FILE *code; /* code text file for TM simulator */

extern int lineno; /* source line number for listing */

/**************************************************/
/***********   Syntax tree for parsing ************/
/**************************************************/

typedef enum {
    StmtK, ExpK
} NodeKind;
/*需要添加WhileK来识别while循环*/
typedef enum {
    IfK, RepeatK, AssignK, ReadK, WriteK, WhileK, TypeK
} StmtKind;
/*删除ConstK，用ConstNumK来识别常数，用ConstStrK来识别字符串
 * 用TypeK来识别数据类型，BoolK来识别bool变量*/
typedef enum {
    OpK, ConstNumK, IdK, ConstStrK, BoolK
} ExpKind;

/* ExpType is used for type checking
 * ExpType用于类型检查*/
typedef enum {
    Void, Integer, Boolean, String
} ExpType;

#define MAXCHILDREN 3

typedef struct treeNode {
    struct treeNode *child[MAXCHILDREN];
    struct treeNode *sibling;
    int lineno;
    NodeKind nodekind;
    union {
        StmtKind stmt;
        ExpKind exp;
    } kind;
    union {
        /*新添加name和string用来记录数据类型，ID名称和字符串*/
        TokenType op;/*操作类型*/
        int val;/*常数*/
        char *name;/*ID名称或数据类型*/
        char *string;/*str*/
    } attr;
    ExpType type; /* for type checking of exps 用于exp的类型检查*/
} TreeNode;

/**************************************************/
/***********   Flags for tracing       ************/
/**************************************************/

/* EchoSource = TRUE causes the source program to
 * be echoed to the listing file with line numbers
 * during parsing
 */
/***  Error **/
#define MAX_ERROR 6
extern int errorCode;
extern char *errorMsg[MAX_ERROR];

extern int EchoSource;

/* TraceScan = TRUE causes token information to be
 * printed to the listing file as each token is
 * recognized by the scanner
 */
extern int TraceScan;

/* TraceParse = TRUE causes the syntax tree to be
 * printed to the listing file in linearized form
 * (using indents for children)
 */
extern int TraceParse;

/* TraceAnalyze = TRUE causes symbol table inserts
 * and lookups to be reported to the listing file
 */
extern int TraceAnalyze;

/* TraceCode = TRUE causes comments to be written
 * to the TM code file as code is generated
 */
extern int TraceCode;

/* Error = TRUE prevents further passes if an error occurs */
extern int Error;
#endif
