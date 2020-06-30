//
// Created by liang on 2020/6/26.
//

#ifndef TINY_TRANSLATE_H
#define TINY_TRANSLATE_H

/*存储四元组的数据结构*/
typedef struct QuadrupleRec {
    char *operator;/*操作符*/
    char *arg1;
    char *arg2;
    char *result;   /*结果*/
} Quadruple;
/*连接需要回填的逻辑地址*/
typedef struct quaLinkList {
    int index;/*逻辑地址下标*/
    struct quaLinkList *next;
} QuaLinkList;
/*返回时的数据结构*/
typedef struct retStruct {
    QuaLinkList *trueList;/*真链*/
    QuaLinkList *falseList;/*假链*/
    char *str;/*其他信息如str、num和布尔类型*/
} RetStruct;

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
RetStruct *cGen(TreeNode *tree);

/*遍历语句*/
void genStmt(TreeNode *tree);

/*遍历表达式*/
RetStruct *genExp(TreeNode *tree);

#endif //TINY_TRANSLATE_H
