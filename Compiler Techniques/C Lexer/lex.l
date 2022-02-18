%{
    #include <stdio.h>
    #include <stdlib.h>

    int charCount = 0, lineCount = 1;
    // 不同类型单词计数
    int keywordCount = 0, idCount = 0, numberCount = 0; 
    int stringCount = 0, annotationCount = 0;
    int pretreatmentCount = 0, operatorCount = 0;
    int errorCount = 0;
%}

/* symbols */
SEMICOL ;
COMMA ,
DOT \.
COLON :
QUESTION_MARK \?
PTSM \->
SPACE [ \t]
ENTER \n|\r|\n\r|\r\n
PRETREATMENT #.*

/* identifier */
ID [a-zA-Z_][a-zA-Z_0-9]*

/* integer */
INT_DEX [1-9][0-9]*|[0]
INT_HEX [0][Xx](([A-Fa-f0-9])+)
INT_OCT [0][0-7]*

/* float */
FLOATN [0-9]+[.][0-9]+([eE][+-]?[0-9]*|[0])?f?

/* string */
ESC \\a|\\b|\\f|\\n|\\r|\\t|\\v|\\\\|\\'|\\\"|\\0|\\ddd|\\xhh
STRING  \"([^\"]*(\\\n)*)*\"
CHARCONST '{ESC}'|'.'

/* arithmetic operator */
PLUS \+
MINUS \-
MUL \*
DIV \/
MOD %
INCREMENT \+\+
DECREMENT \-\-

/* relational operator */
LT <
LE <=
GT >
GE >=
EQ ==
NE !=

/* logic operator */
AND &&
OR \|\|
NOT !

/* bitwise operator */
BAND &
BOR \|
XOR \^
BNOT ~
LM <<
RM >>

/* assignment operator */
ASSIGNOP =
PLUSA \+= 
MINUSA \-=
MULA \*=
DIVA \/=
MODA %=
LMA <<=
RMA >>=
BANDA &=
BORA \|=
XORA \^=

/* brackets */
LP \(
RP \)
LB \[ 
RB \]
LC \{
RC \}

/* type */
SIGNED signed
UNSIGNED unsigned
LONG long
LONGLONG long\ long
SHORT short
INT int
FLOAT float
DOUBLE double
CHAR char
STRUCT struct
VOID void
COMPTYPE {SIGNED}|{UNSIGNED}\ [{LONG}{SHORT}{INT}{SHORT}{LONGLONG}{CHAR}]

/* storage class */
AUTO auto
REGISTER register
STATIC static
EXTERN extern

/* keyword */
ENUM enum
TYPEDEF typedef
RETURN return
UNION union
DEFAULT default
SIZEOF sizeof
VOLATILE volatile

/* judge */
IF if
ELSE else
SWITCH switch
CASE case

/* loop */
FOR for
DO do
WHILE while
BREAK break
CONTINUE continue
GOTO goto

/* comment */
MULTILINECOMMENT "/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"
SINGLELINECOMMENT \/\/[^\n]*

/* symbols don't match error */
SYMBOLS_DONT_MATCH \"[^\"\n]*|"/*"([^\*]|(\*)*[^\*/])*(\*)*
/* Everything didn't fell into any RE above is an ERROR. */
ERROR [^(\n\r\ \t,\";\*&\(\)\{\}\[\]\+\-\*\/%<>!&\|=\^~%\.:\?\\]*

/*end of definition*/

%%
{PRETREATMENT} {
    fprintf(yyout, "<PRETREATMENT, %s>\n", yytext);
    pretreatmentCount++;
    charCount += yyleng;
}

{SEMICOL} {
    fprintf(yyout, "<SEMICOL, %s>\n", yytext);
    charCount += yyleng;
}

{PTSM} {
    fprintf(yyout, "<PTSM, %s>\n", yytext);
    charCount += yyleng;
}

{COMMA} {
    fprintf(yyout, "<COMMA, %s>\n", yytext);
    charCount += yyleng;
}

{COLON} {
    fprintf(yyout, "<COLON, %s>\n", yytext);
    charCount += yyleng;
}

{QUESTION_MARK} {
    fprintf(yyout, "<QUESTION_MARK, %s>\n", yytext);
    charCount += yyleng;
}

{ASSIGNOP} |
{PLUSA} |
{MINUSA} |
{MULA} |
{DIVA} |
{MODA} |
{LMA} |
{RMA} |
{BANDA} |
{BORA} |
{XORA} {
    fprintf(yyout, "<ASSIGNOP, %s>\n", yytext);
    charCount += yyleng;
    operatorCount++;
}

{BAND} |
{BOR} |
{XOR} |
{BNOT} |
{LM} |
{RM} {
    fprintf(yyout, "<BITOP, %s>\n", yytext);
    charCount += yyleng;
    operatorCount++;
}


{INT_DEX} |
{INT_HEX} |
{INT_OCT} {
    fprintf(yyout, "<INT, %s>\n", yytext);
    charCount += yyleng;
    numberCount++;
}

{FLOATN} {
    fprintf(yyout, "<FLOATN, %s>\n", yytext);
    charCount += yyleng;
    numberCount++;
}

{STRING} {
    fprintf(yyout, "<STRING, %s>\n", yytext);
    charCount += yyleng;
    stringCount++;
    int newlineCount = 0, i = 0;
    while (yytext[i] != '\0') {
        if (yytext[i] == '\n') {
            newlineCount++;
        }
        i++;
    }
    lineCount += newlineCount;
}
{CHARCONST} {
    fprintf(yyout, "<CHARCONST, %s>\n", yytext);
    charCount += yyleng;
}

{PLUS} |
{MINUS} |
{DIV} |
{MUL} |
{MOD} |
{INCREMENT} |
{DECREMENT} {
    fprintf(yyout, "<ARITHOP, %s>\n", yytext);
    charCount += yyleng;
    operatorCount++;
}

{LT} |
{LE} |
{GT} |
{GE} |
{EQ} |
{NE} {
    fprintf(yyout, "<RELOP, %s>\n", yytext);
    charCount += yyleng;
}

{AND} |
{OR} |
{NOT} {
    fprintf(yyout, "<LOGICOP, %s>\n", yytext);
    charCount += yyleng;
}

{DOT} {
    fprintf(yyout, "<DOT, %s>\n", yytext);
    charCount += yyleng;
}

{AUTO} |
{REGISTER} |
{STATIC} |
{EXTERN} |
{ENUM} |
{TYPEDEF} |
{RETURN} |
{UNION} |
{DEFAULT} |
{SIZEOF} |
{VOLATILE} |
{IF} |
{ELSE} |
{SWITCH} |
{CASE} |
{FOR} |
{DO} |
{WHILE} |
{BREAK} |
{CONTINUE} |
{GOTO} {
    fprintf(yyout, "<KEYWORD, %s>\n", yytext);
    charCount += yyleng;
    keywordCount++;
}

{LP} |
{RP} |
{LB} |
{RB} |
{LC} |
{RC} {
    fprintf(yyout, "<BRACKETS, %s>\n", yytext);
    charCount += yyleng;
}

{SIGNED} |
{UNSIGNED} |
{LONG} |
{LONGLONG} |
{SHORT} |
{INT} |
{FLOAT} |
{DOUBLE} |
{CHAR} |
{STRUCT} |
{VOID} |
{COMPTYPE} {
    fprintf(yyout, "<TYPE, %s>\n", yytext);
    charCount += yyleng;
    keywordCount++;
}

{SPACE} {
    charCount += yyleng;
}

{ID} {
    fprintf(yyout, "<ID, %s>\n", yytext);
    charCount += yyleng;
    idCount++;
}

{ENTER} {
    lineCount++;
}

{SINGLELINECOMMENT} |
{MULTILINECOMMENT} {
    fprintf(yyout, "<COMMENT, %s>\n", yytext);
    charCount += yyleng;
    annotationCount++;
    int newlineCount = 0, i = 0;
    while (yytext[i] != '\0') {
        if (yytext[i] == '\n') {
            newlineCount++;
        }
        i++;
    }
    lineCount += newlineCount;
}

{ERROR} {
    fprintf(yyout, "line %d: <ERROR, UNRECOGNIZED_SYMBOL, %s>\n", lineCount, yytext);
    charCount += yyleng;
    errorCount++;
}

{SYMBOLS_DONT_MATCH} {
    fprintf(yyout, "line %d: <ERROR, SYMBOLS_DONT_MATCH, %s>\n", lineCount, yytext);
    charCount += yyleng;
    errorCount++;
    int newlineCount = 0, i = 0;
    while (yytext[i] != '\0') {
        if (yytext[i] == '\n') {
            newlineCount++;
        }
        i++;
    }
    lineCount += newlineCount;
}

%%
int yywrap() {
  return 1;
}

int main(int argc, char** argv) {
    if (argc >= 2) {
        FILE *fp = fopen(argv[1], "r");
        if (fp) yyin = fp;
        
        if (argc >= 3) {
            FILE *fp = fopen(argv[2], "w+");
            if (fp) yyout = fp;
        }
    }
    
    yylex();
    
    fprintf(yyout, "------------------------------------------------\n统计信息\n");
    fprintf(yyout, "------------------------------------------------\n");
    fprintf(yyout, "字符个数: %d  行数: %d  错误数: %d  注释数: %d\n",
        charCount, lineCount, errorCount, annotationCount);
    fprintf(yyout, "关键字数: %d  标识符数: %d  数字数: %d  运算符数: %d\n",
        keywordCount, idCount, numberCount, operatorCount);
    fprintf(yyout, "预处理行数: %d  字符串数: %d\n", pretreatmentCount, stringCount);
    fprintf(yyout, "------------------------------------------------\n");

    if (argc >= 2) {
        fclose(yyin);
        if (argc >= 3)
            fclose(yyout);
    }
   return 0;
}
