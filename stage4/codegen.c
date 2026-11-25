#include "codegen.h"
#include "tree.h"
#include <stdio.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

int reg[20];
void initialize(){ for(int i=0;i<20;i++) reg[i]=0; }
int freereg(int num){ if(num<0||num>=20) return 0; reg[num]=0; return 1; }
int getreg(){ for(int i=0;i<20;i++) if(reg[i]==0) return i; return -1; }

static int labelCounter = 0;
int getLabel(){ return labelCounter++; }

/* break/continue: remember which labels the innermost enclosing loop uses */
#define MAXLOOP 64
static int loopStart[MAXLOOP], loopEnd[MAXLOOP], loopSP = 0;
static void pushLoop(int s,int e){ loopStart[loopSP]=s; loopEnd[loopSP]=e; loopSP++; }
static void popLoop(){ if(loopSP>0) loopSP--; }

/* Returns a register holding the memory ADDRESS of the given lvalue node
   (scalar ID_T or array-element ARRACC_T). Caller is responsible for
   freeing this register once done with it. */
int computeAddr(struct tnode *lv, FILE *f){
    if(lv->nodetype == ID_T){
        int p = getreg();
        fprintf(f, "MOV R%d,%d\n", p, lv->entry->binding);
        reg[p] = 1;
        return p;
    } else { /* ARRACC_T */
        int rowreg = codegen(lv->left, f);
        if(lv->right == NULL){
            /* 1D: addr = base + row */
            fprintf(f, "ADD R%d,%d\n", rowreg, lv->entry->binding);
            return rowreg;
        } else {
            /* 2D: addr = base + row*cols + col */
            int colsreg = getreg();
            fprintf(f, "MOV R%d,%d\n", colsreg, lv->entry->cols);
            fprintf(f, "MUL R%d,R%d\n", rowreg, colsreg);   // rowreg = row*cols
            freereg(colsreg);
            int colreg = codegen(lv->right, f);
            fprintf(f, "ADD R%d,R%d\n", rowreg, colreg);    // rowreg = row*cols + col
            freereg(colreg);
            fprintf(f, "ADD R%d,%d\n", rowreg, lv->entry->binding);  // + base
            return rowreg;
        }
    }
}

void genWrite(int vreg, FILE *f){
    int s = getreg();
    fprintf(f, "MOV R%d, \"Write\"\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "MOV R%d, -2\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "PUSH R%d\n", vreg);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "CALL 0\n");
    fprintf(f, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", s,s,s,s,s);
    freereg(s); freereg(vreg);
}

void genRead(int addrreg, FILE *f){
    int s = getreg();
    fprintf(f, "MOV R%d, \"Read\"\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "MOV R%d, -2\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "PUSH R%d\n", addrreg);   // push the address directly - it's already in a register
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "PUSH R%d\n", s);
    fprintf(f, "CALL 0\n");
    fprintf(f, "POP R%d\nPOP R%d\nPOP R%d\nPOP R%d\nPOP R%d\n", s,s,s,s,s);
    freereg(s);
    freereg(addrreg);
}

int codegen(struct tnode *t, FILE *f){
    if(t == NULL) return -1;
    int lreg=-1, rreg=-1;

    switch(t->nodetype){

        case NUM_T: {
            int p = getreg();
            fprintf(f, "MOV R%d,%d\n", p, t->val);
            reg[p] = 1;
            return p;
        }
        case ID_T:
        case ARRACC_T: {
            int addrreg = computeAddr(t, f);
            int p = getreg();
            fprintf(f, "MOV R%d,[R%d]\n", p, addrreg);
            freereg(addrreg);
            reg[p] = 1;
            return p;
        }
        case PLUS_T:
            lreg=codegen(t->left,f); rreg=codegen(t->right,f);
            fprintf(f, "ADD R%d,R%d\n", min(lreg,rreg), max(lreg,rreg));
            freereg(max(lreg,rreg));
            return min(lreg,rreg);
        case MUL_T:
            lreg=codegen(t->left,f); rreg=codegen(t->right,f);
            fprintf(f, "MUL R%d,R%d\n", min(lreg,rreg), max(lreg,rreg));
            freereg(max(lreg,rreg));
            return min(lreg,rreg);
        case MINUS_T:
            lreg=codegen(t->left,f); rreg=codegen(t->right,f);
            fprintf(f, "SUB R%d,R%d\n", lreg, rreg);
            freereg(rreg);
            return lreg;
        case DIV_T:
            lreg=codegen(t->left,f); rreg=codegen(t->right,f);
            fprintf(f, "DIV R%d,R%d\n", lreg, rreg);
            freereg(rreg);
            return lreg;

        case LT_T: case GT_T: case LE_T: case GE_T: case NE_T: case EQREL_T: {
            const char *mnem = t->nodetype==LT_T?"LT": t->nodetype==GT_T?"GT":
                                t->nodetype==LE_T?"LE": t->nodetype==GE_T?"GE":
                                t->nodetype==NE_T?"NE":"EQ";
            lreg=codegen(t->left,f); rreg=codegen(t->right,f);
            fprintf(f, "%s R%d,R%d\n", mnem, lreg, rreg);
            freereg(rreg);
            return lreg;
        }

        case ASGN_T: {
            int vreg = codegen(t->right, f);
            int addrreg = computeAddr(t->left, f);
            fprintf(f, "MOV [R%d],R%d\n", addrreg, vreg);
            freereg(addrreg);
            freereg(vreg);
            return -1;
        }

        case READ_T: {
            int addrreg = computeAddr(t->left, f);
            genRead(addrreg, f);
            return -1;
        }
        case WRITE_T: {
            int vreg = codegen(t->left, f);
            genWrite(vreg, f);
            return -1;
        }
        case CONNECTOR_T:
            codegen(t->left, f);
            codegen(t->right, f);
            return -1;

        case IF_T: {
            int condreg = codegen(t->left, f);
            int Lelse = getLabel();
            fprintf(f, "JZ R%d,L%d\n", condreg, Lelse);
            freereg(condreg);
            codegen(t->right, f);
            if(t->third != NULL){
                int Lend = getLabel();
                fprintf(f, "JMP L%d\n", Lend);
                fprintf(f, "L%d:\n", Lelse);
                codegen(t->third, f);
                fprintf(f, "L%d:\n", Lend);
            } else {
                fprintf(f, "L%d:\n", Lelse);
            }
            return -1;
        }

        case WHILE_T: {
            int Lstart = getLabel();
            int Lend = getLabel();
            pushLoop(Lstart, Lend);
            fprintf(f, "L%d:\n", Lstart);
            int condreg = codegen(t->left, f);
            fprintf(f, "JZ R%d,L%d\n", condreg, Lend);
            freereg(condreg);
            codegen(t->right, f);
            fprintf(f, "JMP L%d\n", Lstart);
            fprintf(f, "L%d:\n", Lend);
            popLoop();
            return -1;
        }

        case BREAK_T:
            if(loopSP > 0) fprintf(f, "JMP L%d\n", loopEnd[loopSP-1]);
            return -1;
        case CONTINUE_T:
            if(loopSP > 0) fprintf(f, "JMP L%d\n", loopStart[loopSP-1]);
            return -1;

        default:
            fprintf(stderr, "codegen: unknown nodetype %d\n", t->nodetype);
            return -1;
    }
}

/* interpreter — unchanged from before */
int vars[26];
static int breakFlag=0, continueFlag=0;
int evaluate(tnode *t){
    if(t==NULL || breakFlag || continueFlag) return 0;
    switch(t->nodetype){
        case NUM_T: return t->val;
        case ID_T:  return vars[t->varname[0]-'a'];
        case PLUS_T: return evaluate(t->left)+evaluate(t->right);
        case MINUS_T: return evaluate(t->left)-evaluate(t->right);
        case MUL_T: return evaluate(t->left)*evaluate(t->right);
        case DIV_T: return evaluate(t->left)/evaluate(t->right);
        case LT_T: return evaluate(t->left) < evaluate(t->right);
        case GT_T: return evaluate(t->left) > evaluate(t->right);
        case LE_T: return evaluate(t->left) <= evaluate(t->right);
        case GE_T: return evaluate(t->left) >= evaluate(t->right);
        case NE_T: return evaluate(t->left) != evaluate(t->right);
        case EQREL_T: return evaluate(t->left) == evaluate(t->right);
        case ASGN_T: vars[t->left->varname[0]-'a']=evaluate(t->right); return 0;
        case READ_T: scanf("%d",&vars[t->left->varname[0]-'a']); return 0;
        case WRITE_T: printf("%d\n", evaluate(t->left)); return 0;
        case CONNECTOR_T:
            evaluate(t->left);
            if(breakFlag||continueFlag) return 0;
            evaluate(t->right);
            return 0;
        case IF_T:
            if(evaluate(t->left)) evaluate(t->right);
            else if(t->third) evaluate(t->third);
            return 0;
        case WHILE_T:
            while(evaluate(t->left)){
                evaluate(t->right);
                if(breakFlag){breakFlag=0;break;}
                if(continueFlag){continueFlag=0;continue;}
            }
            return 0;
        case BREAK_T: breakFlag=1; return 0;
        case CONTINUE_T: continueFlag=1; return 0;
        default: return 0;
    }
}