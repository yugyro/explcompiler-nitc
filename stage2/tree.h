#ifndef TREE_H
#define TREE_H

#include <stdio.h>

typedef struct tnode { 
	int val;	// value of a number for NUM nodes.
	int type;	//type of variable
	char* varname;	//name of a variable for ID nodes  
	int nodetype;  // information about non-leaf nodes - read/write/connector/+/* etc.  {0=num,1=read,2=write,3=connector,4=+,5=-,6=*,7=/}
	struct tnode *left,*right;	//left and right branches   
}tnode;

#define TYPE_INT 1


#define NUM_T        1
#define ID_T         2
#define PLUS_T       3
#define MINUS_T      4
#define MUL_T        5
#define DIV_T        6
#define ASGN_T       7
#define READ_T       8
#define WRITE_T      9
#define CONNECTOR_T  10

#define YYSTYPE tnode*

/*Create a node tnode*/
struct tnode* createTree(int val, int type, char* c, struct tnode *l, struct tnode *r);

#endif 