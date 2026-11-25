//list out all C-like comments (both single line and multi line comments) from a text file.

%{
    #include <stdio.h>
%}

upp [A-Z]
low [a-z]
newline [\n]
digits [0-9]
characters
