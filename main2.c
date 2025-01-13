#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types for the lexer
typedef enum {
    TOKEN_NUMBER,      // Numeric literal
    TOKEN_PLUS,        // '+'
    TOKEN_MINUS,       // '-'
    TOKEN_STAR,        // '*'
    TOKEN_SLASH,       // '/'
    TOKEN_ASSIGN,      // '='
    TOKEN_IDENTIFIER,  // Variable names
    TOKEN_PRINT,       // 'print'
    TOKEN_LPAREN,      // '('
    TOKEN_RPAREN,      // ')'
    TOKEN_SEMICOLON,   // ';'
    TOKEN_EOF          // End of input
} TokenType;

// Represents a token produced by the lexer
typedef struct {
    TokenType type; #include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    char data[64];
    double value;
} Token;

typedef struct {
    char name[64];
    double value;
} Variable;

#define MAX_VARIABLES 100
Variable variables[MAX_VARIABLES];
int variableCount = 0;

char *source;
int pos = 0;

Token getNextToken();
void match(TokenType type);
double expression();
double term();
double factor();
void statement();
double getVariableValue(const char *name);
void setVariableValue(const char *name, double value);

Token getNextToken() {
    while (isspace(source[pos])) pos++;

    if (isdigit(source[pos])) {
        Token token = {TOKEN_NUMBER};
        int start = pos;
        while (isdigit(source[pos]) || source[pos] == '.') pos++;
        char numberStr[64];
        strncpy(numberStr, &source[start], pos - start);
        numberStr[pos - start] = '\0';
        token.value = atof(numberStr);
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

Token currentToken;

void match(TokenType type) {
    if (currentToken.type == type) {
        currentToken = getNextToken();
    } else {
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
        exit(1);
    }
}

// Parse factors (numbers or variables)
double factor() {
    if (currentToken.type == TOKEN_NUMBER) {
        double value = currentToken.value;
        match(TOKEN_NUMBER);
        return value;
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        return getVariableValue(name);
    } else {
        fprintf(stderr, "Invalid factor\n");
        exit(1);
    }
}

// Parse terms (handles *, / operators)
double term() {
    double result = factor();
    while (currentToken.type == TOKEN_STAR || currentToken.type == TOKEN_SLASH) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_STAR) {
            result *= factor();
        } else if (op.type == TOKEN_SLASH) {
            result /= factor();
        }
    }
    return result;
}

// Parse expressions (handles +, - operators)
double expression() {
    double result = term();
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_PLUS) {
            result += term();
        } else if (op.type == TOKEN_MINUS) {
            result -= term();
        }
    }
    return result;
}

void statement() {
    if (currentToken.type == TOKEN_PRINT) {
        match(TOKEN_PRINT);
        double value = expression();
        printf("%lf\n", value);
        match(TOKEN_SEMICOLON);
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        match(TOKEN_ASSIGN);
        double value = expression();
        setVariableValue(name, value);
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

double getVariableValue(const char *name) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

void setVariableValue(const char *name, double value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (variableCount < MAX_VARIABLES) {
        strcpy(variables[variableCount].name, name);
        variables[variableCount].value = value;
        variableCount++;
    } else {
        fprintf(stderr, "Variable limit exceeded\n");
        exit(1);
    }
}

int main() {
    source = """x = print x;\ny = 10;\nprint y + x;";
    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
    return 0;
}
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

// Retrieves the value of a variable from the symbol table
double getVariableValue(const char *name) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

// Stores or updates a variable in the symbol table
void setVariableValue(const char *name, double value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (variableCount < MAX_VARIABLES) {
        strcpy(variables[variableCount].name, name);
        variables[variableCount].value = value;
        variableCount++;
    } else {
        fprintf(stderr, "Variable limit exceeded\n");
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    char data[64];
    double value;
} Token;

typedef struct {
    char name[64];
    double value;
} Variable;

#define MAX_VARIABLES 100
Variable variables[MAX_VARIABLES];
int variableCount = 0;

char *source;
int pos = 0;

Token getNextToken();
void match(TokenType type);
double expression();
double term();
double factor();
void statement();
double getVariableValue(const char *name);
void setVariableValue(const char *name, double value);

Token getNextToken() {
    while (isspace(source[pos])) pos++;

    if (isdigit(source[pos])) {
        Token token = {TOKEN_NUMBER};
        int start = pos;
        while (isdigit(source[pos]) || source[pos] == '.') pos++;
        char numberStr[64];
        strncpy(numberStr, &source[start], pos - start);
        numberStr[pos - start] = '\0';
        token.value = atof(numberStr);
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

Token currentToken;

void match(TokenType type) {
    if (currentToken.type == type) {
        currentToken = getNextToken();
    } else {
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
        exit(1);
    }
}

// Parse factors (numbers or variables)
double factor() {
    if (currentToken.type == TOKEN_NUMBER) {
        double value = currentToken.value;
        match(TOKEN_NUMBER);
        return value;
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        return getVariableValue(name);
    } else {
        fprintf(stderr, "Invalid factor\n");
        exit(1);
    }
}

// Parse terms (handles *, / operators)
double term() {
    double result = factor();
    while (currentToken.type == TOKEN_STAR || currentToken.type == TOKEN_SLASH) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_STAR) {
            result *= factor();
        } else if (op.type == TOKEN_SLASH) {
            result /= factor();
        }
    }
    return result;
}

// Parse expressions (handles +, - operators)
double expression() {
    double result = term();
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_PLUS) {
            result += term();
        } else if (op.type == TOKEN_MINUS) {
            result -= term();
        }
    }
    return result;
}

void statement() {
    if (currentToken.type == TOKEN_PRINT) {
        match(TOKEN_PRINT);
        double value = expression();
        printf("%lf\n", value);
        match(TOKEN_SEMICOLON);
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        match(TOKEN_ASSIGN);
        double value = expression();
        setVariableValue(name, value);
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

double getVariableValue(const char *name) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

void setVariableValue(const char *name, double value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (variableCount < MAX_VARIABLES) {
        strcpy(variables[variableCount].name, name);
        variables[variableCount].value = value;
        variableCount++;
    } else {
        fprintf(stderr, "Variable limit exceeded\n");
        exit(1);
    }
}

int main() {
    source = """x = print x;\ny = 10;\nprint y + x;";
    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    char data[64];
    double value;
} Token;

typedef struct {
    char name[64];
    double value;
} Variable;

#define MAX_VARIABLES 100
Variable variables[MAX_VARIABLES];
int variableCount = 0;

char *source;
int pos = 0;

Token getNextToken();
void match(TokenType type);
double expression();
double term();
double factor();
void statement();
double getVariableValue(const char *name);
void setVariableValue(const char *name, double value);

Token getNextToken() {
    while (isspace(source[pos])) pos++;

    if (isdigit(source[pos])) {
        Token token = {TOKEN_NUMBER};
        int start = pos;
        while (isdigit(source[pos]) || source[pos] == '.') pos++;
        char numberStr[64];
        strncpy(numberStr, &source[start], pos - start);
        numberStr[pos - start] = '\0';
        token.value = atof(numberStr);
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

Token currentToken;

void match(TokenType type) {
    if (currentToken.type == type) {
        currentToken = getNextToken();
    } else {
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
        exit(1);
    }
}

// Parse factors (numbers or variables)
double factor() {
    if (currentToken.type == TOKEN_NUMBER) {
        double value = currentToken.value;
        match(TOKEN_NUMBER);
        return value;
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        return getVariableValue(name);
    } else {
        fprintf(stderr, "Invalid factor\n");
        exit(1);
    }
}

// Parse terms (handles *, / operators)
double term() {
    double result = factor();
    while (currentToken.type == TOKEN_STAR || currentToken.type == TOKEN_SLASH) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_STAR) {
            result *= factor();
        } else if (op.type == TOKEN_SLASH) {
            result /= factor();
        }
    }
    return result;
}

// Parse expressions (handles +, - operators)
double expression() {
    double result = term();
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_PLUS) {
            result += term();
        } else if (op.type == TOKEN_MINUS) {
            result -= term();
        }
    }
    return result;
}

void statement() {
    if (currentToken.type == TOKEN_PRINT) {
        match(TOKEN_PRINT);
        double value = expression();
        printf("%lf\n", value);
        match(TOKEN_SEMICOLON);
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        match(TOKEN_ASSIGN);
        double value = expression();
        setVariableValue(name, value);
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

double getVariableValue(const char *name) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

void setVariableValue(const char *name, double value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (variableCount < MAX_VARIABLES) {
        strcpy(variables[variableCount].name, name);
        variables[variableCount].value = value;
        variableCount++;
    } else {
        fprintf(stderr, "Variable limit exceeded\n");
        exit(1);
    }
}

int main() {
    source = """x = print x;\ny = 10;\nprint y + x;";
    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    char data[64];
    double value;
} Token;

typedef struct {
    char name[64];
    double value;
} Variable;

#define MAX_VARIABLES 100
Variable variables[MAX_VARIABLES];
int variableCount = 0;

char *source;
int pos = 0;

Token getNextToken();
void match(TokenType type);
double expression();
double term();
double factor();
void statement();
double getVariableValue(const char *name);
void setVariableValue(const char *name, double value);

Token getNextToken() {
    while (isspace(source[pos])) pos++;

    if (isdigit(source[pos])) {
        Token token = {TOKEN_NUMBER};
        int start = pos;
        while (isdigit(source[pos]) || source[pos] == '.') pos++;
        char numberStr[64];
        strncpy(numberStr, &source[start], pos - start);
        numberStr[pos - start] = '\0';
        token.value = atof(numberStr);
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

Token currentToken;

void match(TokenType type) {
    if (currentToken.type == type) {
        currentToken = getNextToken();
    } else {
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
        exit(1);
    }
}

// Parse factors (numbers or variables)
double factor() {
    if (currentToken.type == TOKEN_NUMBER) {
        double value = currentToken.value;
        match(TOKEN_NUMBER);
        return value;
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        return getVariableValue(name);
    } else {
        fprintf(stderr, "Invalid factor\n");
        exit(1);
    }
}

// Parse terms (handles *, / operators)
double term() {
    double result = factor();
    while (currentToken.type == TOKEN_STAR || currentToken.type == TOKEN_SLASH) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_STAR) {
            result *= factor();
        } else if (op.type == TOKEN_SLASH) {
            result /= factor();
        }
    }
    return result;
}

// Parse expressions (handles +, - operators)
double expression() {
    double result = term();
    while (currentToken.type == TOKEN_PLUS || currentToken.type == TOKEN_MINUS) {
        Token op = currentToken;
        match(op.type);
        if (op.type == TOKEN_PLUS) {
            result += term();
        } else if (op.type == TOKEN_MINUS) {
            result -= term();
        }
    }
    return result;
}

void statement() {
    if (currentToken.type == TOKEN_PRINT) {
        match(TOKEN_PRINT);
        double value = expression();
        printf("%lf\n", value);
        match(TOKEN_SEMICOLON);
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        match(TOKEN_ASSIGN);
        double value = expression();
        setVariableValue(name, value);
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

double getVariableValue(const char *name) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return variables[i].value;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

void setVariableValue(const char *name, double value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (variableCount < MAX_VARIABLES) {
        strcpy(variables[variableCount].name, name);
        variables[variableCount].value = value;
        variableCount++;
    } else {
        fprintf(stderr, "Variable limit exceeded\n");
        exit(1);
    }
}

int main() {
    source = """x = 5;
    print x;\ny = 10;\nprint y + x;";
    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }
    return 0;
}
