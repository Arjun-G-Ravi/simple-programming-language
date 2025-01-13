#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types for the lexer
typedef enum {
    TOKEN_NUMBER,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_ASSIGN,
    TOKEN_IDENTIFIER,
    TOKEN_PRINT,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_EOF
} TokenType;

// Represents a token produced by the lexer
typedef struct {
    TokenType type;  // Type of the token
    char data[64];   // For identifiers or keywords
    double value;    // For numeric literals
} Token;

// Represents a variable in the symbol table
typedef struct {
    char name[64];   // Variable name
    double value;    // Variable value
} Variable;

#define MAX_VARIABLES 100   // Maximum number of variables
Variable variables[MAX_VARIABLES]; // Symbol table for variables
int variableCount = 0;             // Number of stored variables

char *source;    // Input source code
int pos = 0;     // Current position in the source code

// Function prototypes
Token getNextToken();
void match(TokenType type);
double expression();
double term();
double factor();
void statement();
double getVariableValue(const char *name);
void setVariableValue(const char *name, double value);

// Lexer: Converts raw source code into tokens
Token getNextToken() {
    // Skip whitespace and line breaks
    while (isspace(source[pos]) || source[pos] == '\r' || source[pos] == '\n') {
        pos++;
    }

    // Handle numeric literals
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

    // Handle identifiers and keywords
    if (isalpha(source[pos])) {
        Token token = {TOKEN_IDENTIFIER};
        int start = pos;
        while (isalnum(source[pos])) pos++;
        strncpy(token.data, &source[start], pos - start);
        token.data[pos - start] = '\0';
        if (strcmp(token.data, "print") == 0) {
            token.type = TOKEN_PRINT; // Recognize 'print' keyword
        }
        return token;
    }

    // Handle single-character tokens
    Token token;
    switch (source[pos]) {
        case '+': token = (Token){TOKEN_PLUS}; break;
        case '-': token = (Token){TOKEN_MINUS}; break;
        case '*': token = (Token){TOKEN_STAR}; break;
        case '/': token = (Token){TOKEN_SLASH}; break;
        case '=': token = (Token){TOKEN_ASSIGN}; break;
        case ';': token = (Token){TOKEN_SEMICOLON}; break;
        case '(': token = (Token){TOKEN_LPAREN}; break;
        case ')': token = (Token){TOKEN_RPAREN}; break;
        case '\0': token = (Token){TOKEN_EOF}; break;
        default:
            fprintf(stderr, "Unexpected character: %c\n", source[pos]);
            exit(1);
    }
    pos++;
    return token;
}
Token currentToken;

// Ensures the current token matches the expected type and advances to the next token
void match(TokenType type) {
    if (currentToken.type == type) {
        currentToken = getNextToken();
    } else {
        // The last empty token was causing issues. This fixes it
        if (currentToken.type != type) {
            if (currentToken.type != TOKEN_EOF) {
            fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
            }
            else { exit(1); }
        }
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
        exit(1);
    }
}

// Parses and evaluates factors (numbers or variables)
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

// Parses and evaluates terms (handles '*' and '/' operators)
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

// Parses and evaluates expressions (handles '+' and '-' operators)
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

// Parses and executes statements (assignments or print statements)
void statement() {
    if (currentToken.type == TOKEN_PRINT) {
        // Handle 'print(expression);'
        match(TOKEN_PRINT);
        match(TOKEN_LPAREN);
        double value = expression();
        match(TOKEN_RPAREN);
        printf("%lf\n", value);
        match(TOKEN_SEMICOLON);
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        // Handle 'identifier = expression;'
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
        exit(1);
    }
}

// Reads the content of a file into a string
char *readFile(const char *filePath) {
    FILE *file = fopen(filePath, "r");
    if (!file) {
        fprintf(stderr, "Error opening file: %s\n", filePath);
        exit(1);
    }
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file);

    char *buffer = malloc(fileSize + 1);
    if (!buffer) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';
    fclose(file);

    return buffer;
}


int main() {
    const char *filePath = "/home/arjun/Desktop/GitHub/simple-programming-language/code.txt";
    source = readFile(filePath);

    currentToken = getNextToken();
    while (currentToken.type != TOKEN_EOF) {
        statement();
    }

    free(source);
    return 0;
}
