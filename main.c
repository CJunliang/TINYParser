/****************************************************/
/* File: main.c                                     */
/* Main program for TINY compiler                   */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "globals.h"

/* set NO_PARSE to TRUE to get a scanner-only compiler */
/*将NO_PARSE设置为TRUE可获得仅扫描程序的编译器*/
#define NO_PARSE FALSE

#include "util.h"
#include "analyze.h"
#include "translate.h"

#if NO_PARSE
#include "scan.h"
#else

#include "parse.h"

#endif

/* allocate global variables */
int lineno = 0;
FILE *source;
FILE *listing;
FILE *code;

/* allocate and set tracing flags */
int EchoSource = TRUE;
int TraceScan = TRUE;
int TraceParse = TRUE;
int TraceAnalyze = TRUE;
int TraceCode = TRUE;

int Error = FALSE;

int main(int argc, char *argv[]) {
    TreeNode *syntaxTree;
    char pgm[120]; /* source code file name */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(1);
    }
    strcpy(pgm, argv[1]);
    if (strchr(pgm, '.') == NULL)
        strcat(pgm, ".tny");
    source = fopen(pgm, "r");
    if (source == NULL) {
        fprintf(stderr, "File %s not found\n", pgm);
        exit(1);
    }
    listing = stdout; /* send listing to screen */
    fprintf(listing, "\nTINY COMPILATION: %s\n", pgm);
#if NO_PARSE
    while (getToken() != ENDFILE);
#else
    syntaxTree = parse();
    if (TraceParse) {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(syntaxTree);
    }
#endif
    if (!Error) {
        if (TraceAnalyze)
            fprintf(listing, "\nBuilding Symbol Table...\n");
        buildSymTab(syntaxTree);
        /*if (TraceAnalyze)
            fprintf(listing, "\nChecking Types...\n");
        typeCheck(syntaxTree);
        if (TraceAnalyze)
            fprintf(listing, "\nType Checking Finished\n");*/
    }
    if (!Error) {
        char *codeFile;
        int fnlen = strcspn(pgm, ".");
        codeFile = (char *) calloc(fnlen + 4, sizeof(char));
        strncpy(codeFile, pgm, fnlen);
        strcat(codeFile, ".tm");
        code = fopen(codeFile, "w");
        if (code == NULL) {
            printf("Unable to open %s\n", codeFile);
            exit(1);
        }
        codeGen(syntaxTree, codeFile);
        fclose(code);
    }
    fclose(source);
    return 0;
}

