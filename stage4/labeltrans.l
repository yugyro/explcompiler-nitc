%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLABELS 1000
int labelAddr[MAXLABELS];
int address = 2056;
int pass = 1;
FILE *outfile;
%}

%%
^"L"[0-9]+":"\n {
    /* a label definition line, e.g. "L3:" - not a real instruction */
    int n = atoi(yytext + 1);
    if (pass == 1) {
        labelAddr[n] = address;
        printf("label %d = %d\n",n,address);
    }
    /* pass 2: emit nothing for this line */
}

.*\n {
    if (pass == 1) {
        address += 2;   /* every real instruction is 2 words */
    } else {
        char buf[512];
        strncpy(buf, yytext, sizeof(buf)-1);
        buf[sizeof(buf)-1] = '\0';
        int len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';

        /* does this line end in "L<digits>" (a jump target)? */
        char *p = strrchr(buf, 'L');
        int isLabelRef = 0;
        if (p && isdigit((unsigned char)*(p+1))) {
            isLabelRef = 1;
            for (char *q = p+1; *q; q++)
                if (!isdigit((unsigned char)*q)) { isLabelRef = 0; break; }
        }
        if (isLabelRef) {
            int n = atoi(p+1);
            *p = '\0';
            fprintf(outfile, "%s%d\n", buf, labelAddr[n]);
        } else {
            fprintf(outfile, "%s\n", buf);
        }
    }
}
%%

int yywrap(){ return 1; }

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <intermediate.xsm> <targetcode.xsm>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) { perror("fopen intermediate"); return 1; }
    outfile = fopen(argv[2], "w");
    if (!outfile) { perror("fopen target"); return 1; }

    /* first 8 lines are the raw header - copy them through untouched */
    char headerLines[8][64];
    for (int i = 0; i < 8; i++) {
        if (!fgets(headerLines[i], sizeof(headerLines[i]), yyin)) {
            fprintf(stderr, "Unexpected EOF reading header\n");
            return 1;
        }
    }
    long instrStart = ftell(yyin);

    /* pass 1: find every label's real address */
    pass = 1;
    address = 2056;
    yylex();

    /* pass 2: write header, then translated instructions */
    for (int i = 0; i < 8; i++) fputs(headerLines[i], outfile);
    fseek(yyin, instrStart, SEEK_SET);
    yyrestart(yyin);
    pass = 2;
    yylex();

    fclose(yyin);
    fclose(outfile);
    
    printf("Label translation complete -> %s\n", argv[2]);
    return 0;
}