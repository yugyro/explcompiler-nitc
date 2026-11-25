#include "codegen.h"
#include "tree.h"
#include <stdio.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

int reg[20];

void initialize(){
    for(int i = 0; i < 20; i++)
        reg[i] = 0;
}

int freereg(int num){
    if(num < 0 || num >= 20) return 0;
    reg[num] = 0;
    return 1;
}

int getreg(){
    for(int i = 0; i < 20; i++)
        if(reg[i] == 0) return i;
    return -1;
}

/* Emits the library CALL 0 sequence to write the value currently in register vreg.
   Convention: PUSH funcname, PUSH -2, PUSH value, PUSH blank, PUSH retval-slot, CALL 0 */
void genWrite(int vreg, FILE *f){
    int s = getreg();               /* scratch register for funcname/-2/blank/retval */
    fprintf(f, "MOV R%d, \"Write\"\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "MOV R%d, -2\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "PUSH R%d\n", vreg);   /* the value to print */
    fprintf(f, "PUSH R%d\n", s);      /* blank arg3, reusing scratch */
    fprintf(f, "PUSH R%d\n", s);      /* space for return value */
    fprintf(f, "CALL 0\n");
    fprintf(f, "POP R%d\n", s);       /* return value, discarded */
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    freereg(s);
    freereg(vreg);
}

/* Emits the library CALL 0 sequence to read a value into memory address addr.
   arg2 must be the ADDRESS, not a loaded value. */
void genRead(int addr, FILE *f){
    int s = getreg();
    int a = getreg();
    fprintf(f, "MOV R%d, \"Read\"\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "MOV R%d, -2\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "MOV R%d, %d\n", a, addr);
    fprintf(f, "PUSH R%d\n", a);      /* address to store the input into */
    fprintf(f, "PUSH R%d\n", s);      /* blank arg3 */
    fprintf(f, "PUSH R%d\n", s);      /* space for return value */
    fprintf(f, "CALL 0\n");
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    fprintf(f, "POP R%d\n", s);
    freereg(s);
    freereg(a);
}

int codegen(struct tnode *t, FILE *target_file){
    if(t == NULL) return -1;

    int lreg = -1, rreg = -1;

    switch(t->nodetype){

        case NUM_T: {
            int p = getreg();
            fprintf(target_file, "MOV R%d,%d\n", p, t->val);
            reg[p] = 1;
            return p;
        }

        case ID_T: {
            /* rvalue use: load the variable's value from memory */
            int p = getreg();
            int addr = VARADDR(t->varname[0]);
            fprintf(target_file, "MOV R%d,[%d]\n", p, addr);
            reg[p] = 1;
            return p;
        }

        case PLUS_T:
            lreg = codegen(t->left, target_file);
            rreg = codegen(t->right, target_file);
            fprintf(target_file, "ADD R%d,R%d\n", min(lreg,rreg), max(lreg,rreg));
            freereg(max(lreg,rreg));
            return min(lreg,rreg);

        case MUL_T:
            lreg = codegen(t->left, target_file);
            rreg = codegen(t->right, target_file);
            fprintf(target_file, "MUL R%d,R%d\n", min(lreg,rreg), max(lreg,rreg));
            freereg(max(lreg,rreg));
            return min(lreg,rreg);

        case MINUS_T:
            lreg = codegen(t->left, target_file);
            rreg = codegen(t->right, target_file);
            fprintf(target_file, "SUB R%d,R%d\n", lreg, rreg);  /* order matters: non-commutative */
            freereg(rreg);
            return lreg;

        case DIV_T:
            lreg = codegen(t->left, target_file);
            rreg = codegen(t->right, target_file);
            fprintf(target_file, "DIV R%d,R%d\n", lreg, rreg);  /* order matters: non-commutative */
            freereg(rreg);
            return lreg;

        case ASGN_T: {
            /* t->left is the ID node (target); t->right is the RHS expression.
               We do NOT call codegen(t->left,...) - we only need its address, not its value. */
            int vreg = codegen(t->right, target_file);
            int addr = VARADDR(t->left->varname[0]);
            fprintf(target_file, "MOV [%d],R%d\n", addr, vreg);
            freereg(vreg);
            return -1;
        }

        case READ_T: {
            /* t->left is the ID node whose address we read into */
            int addr = VARADDR(t->left->varname[0]);
            genRead(addr, target_file);
            return -1;
        }

        case WRITE_T: {
            /* t->left is the expression to evaluate and print */
            int vreg = codegen(t->left, target_file);
            genWrite(vreg, target_file);
            return -1;
        }

        case CONNECTOR_T:
            /* joins two statement subtrees - just walk both, no register produced */
            codegen(t->left, target_file);
            codegen(t->right, target_file);
            return -1;

        default:
            fprintf(stderr, "codegen: unknown nodetype %d\n", t->nodetype);
            return -1;
    }
}

int vars[26];

int evaluate(tnode *t){
    if(t == NULL) return 0;

    switch(t->nodetype){
        case NUM_T:
            return t->val;

        case ID_T:
            return vars[t->varname[0] - 'a'];

        case PLUS_T:
            return evaluate(t->left) + evaluate(t->right);

        case MINUS_T:
            return evaluate(t->left) - evaluate(t->right);

        case MUL_T:
            return evaluate(t->left) * evaluate(t->right);

        case DIV_T:
            return evaluate(t->left) / evaluate(t->right);

        case ASGN_T:
            vars[t->left->varname[0] - 'a'] = evaluate(t->right);
            return 0;

        case READ_T:
            scanf("%d", &vars[t->left->varname[0] - 'a']);
            return 0;

        case WRITE_T:
            printf("%d\n", evaluate(t->left));
            return 0;

        case CONNECTOR_T:
            evaluate(t->left);
            evaluate(t->right);
            return 0;

        default:
            fprintf(stderr, "evaluate: unknown nodetype %d\n", t->nodetype);
            return 0;
    }
}