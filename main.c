#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_ASSIGN,
    TOKEN_IDENTIFIER,
    TOKEN_PRINT,
    TOKEN_SEMICOLON,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[64];
    double value; // For numbers
} Token;

char *source;
char *next;
int pos = 0;

// Function prototypes
Token getNextToken();
void match(TokenType type);
double expression();
void statement();

// Lexer
Token getNextToken() {
    while (isspace(source[pos])) pos++;
    if (isdigit(source[pos])) {
        Token token = {TOKEN_NUMBER};
        token.value = strtod(&source[pos], &source);
        return token;
    }
    if (isalpha(source[pos])) {
        Token token = {TOKEN_IDENTIFIER};
        int start = pos;
        while (isalnum(source[pos])) pos++;
        strncpy(token.lexeme, &source[start], pos - start);
        token.lexeme[pos - start] = '\0';
        if (strcmp(token.lexeme, "print") == 0) {
            token.type = TOKEN_PRINT;
        }
        return token;
    }
    Token token;
    switch (source[pos]) {
        case '+': token = (Token){TOKEN_PLUS}; break;
        case '-': token = (Token){TOKEN_MINUS}; break;
        case '*': token = (Token){TOKEN_STAR}; break;
        case '/': token = (Token){TOKEN_SLASH}; break;
        case '=': token = (Token){TOKEN_ASSIGN}; break;
        case ';': token = (Token){TOKEN_SEMICOLON}; break;
        case '\0': token = (Token){TOKEN_EOF}; break;
        default: fprintf(stderr, "Unexpected character: %c\n", source[pos]); exit(1);
    }
    pos++;
    return token;
}

// Parser and Interpreter
Token currentToken;

void match(TokenType type) {
    if (currentToken.type == type) {
        currentToken = getNextToken();
    } else {
        fprintf(stderr, "Unexpected token: %s\n", currentToken.lexeme);
        exit(1);
    }
}

double expression() {
    Token token = currentToken;
    match(TOKEN_NUMBER);
    return token.value;
}

void statement() {
    if (currentToken.type == TOKEN_PRINT) {
        match(TOKEN_PRINT);
        double value = expression();
        printf("%lf\n", value);
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

int main() {
    source = """print 42;""";
    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
    return 0;
}
