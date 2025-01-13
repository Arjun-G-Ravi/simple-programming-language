#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types that lexer will produce
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
    char data[64]; // Non numbers
    double value; // For numbers
} Token;

char *source; // A pointer to the input source code as a string.
int pos = 0; // Tracks the current position in the source code.

// Function prototypes
Token getNextToken();
void match(TokenType type);
double expression();
void statement();

// Lexer or Tokenizer
// Converts the raw source code into tokens by scanning it character by character.
Token getNextToken() {
    while (isspace(source[pos])) pos++; // skip spaces

    if (isdigit(source[pos])) {
        Token token = {TOKEN_NUMBER};
        token.value = strtod(&source[pos], &source);
        return token;
    }
    if (isalpha(source[pos])) {
        Token token = {TOKEN_IDENTIFIER};
        int start = pos;
        while (isalnum(source[pos])) pos++;
        strncpy(token.data, &source[start], pos - start);
        token.data[pos - start] = '\0';
        if (strcmp(token.data, "print") == 0) {
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
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
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
    source = """print 4;""";
    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
    return 0;
}
