//
// Created by liang on 2020/6/26.
//

#ifndef TINY_TRANSLATE_H
#define TINY_TRANSLATE_H

/*存储四元组的数据结构*/
typedef struct QuadrupleRec {
    char *operator;
    char *arg1;
    char *arg2;
    char *result;
} Quadruple;

/*回填时使用的链接*/
typedef struct quaLinkList {
    int index;
    struct quaLinkList *next;
} QuaLinkList;

/*添加一个四元组*/
void addQuadruple(char *operator, char *arg1, char *arg2, char *result);

/*新增加一个链表来记录要回填的信息*/
QuaLinkList *makeList(int i);

/*合并两个链表*/
QuaLinkList *merge(QuaLinkList *list1, QuaLinkList *list2);

/*进行回填*/
void backPatch(QuaLinkList *list, int target);

/*遍历语法树来将四元式生成到代码文件*/
void codeGen(TreeNode *syntaxTree, char *codeFile);

/*根据节点类型的不同来使用不同的函数来遍历语法树*/
char *cGen(TreeNode *tree);

/*遍历语句*/
void genStmt(TreeNode *tree);

/*遍历表达式*/
char *genExp(TreeNode *tree);

#endif //TINY_TRANSLATE_H
