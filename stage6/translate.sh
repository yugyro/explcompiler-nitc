lex -o label_lex.c labeltranslator.l
gcc label_lex.c labeltranslator.c -o labeltranslator

bash ./xsm -l library.lib -e < path to target_file.xsm > --debug
