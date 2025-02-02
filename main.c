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
    TOKEN_STRING,
    TOKEN_EOF,
    TOKEN_WHILE,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_EQUALS,    // ==
    TOKEN_LT,       // <
    TOKEN_GT,       // >
    TOKEN_NOT,      // !
    TOKEN_NOT_EQ    // !=
} TokenType;

// Represents a token produced by the lexer
typedef struct {
    TokenType type;
    char data[64];
    double value;
} Token;

// Represents a variable in the symbol table
typedef enum {
    VAR_NUMBER,
    VAR_STRING
} VariableType;

typedef struct {
    char name[64];
    VariableType type;
    union {
        double number;
        char string[256];
    } value;
} Variable;

#define MAX_VARIABLES 100 // Maximum number of variables
Variable variables[MAX_VARIABLES]; // Symbol table for variables
int variableCount = 0; // Number of stored variables

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
const char* getStringValue(const char *name);
void setStringValue(const char *name, const char *value);
int getVariableIndex(const char *name);
int condition();
void while_statement();
void block_statement();

// Add this new struct to store while loop information
typedef struct {
    int condition_pos;
    int block_pos;
    int block_end;
} WhileLoopInfo;

// Lexer: Converts raw source code into tokens
Token getNextToken() {
    // Initialize token with all fields set to 0
    Token token = {
        .type = TOKEN_EOF,
        .data = {0},
        .value = 0.0
    };

    // Skip whitespace and line breaks
    while (isspace(source[pos]) || source[pos] == '\r' || source[pos] == '\n') {
        pos++;
    }

    // Handle numeric literals
    if (isdigit(source[pos])) {
        token.type = TOKEN_NUMBER;
        int start = pos;
        while (isdigit(source[pos]) || source[pos] == '.') pos++;
        char numberStr[64];
        strncpy(numberStr, &source[start], pos - start);
        numberStr[pos - start] = '\0';
        token.value = atof(numberStr);
        return token;
    }

    // Handle string literals
    if (source[pos] == '"') {
        token.type = TOKEN_STRING;
        pos++;
        int start = pos;
        while (source[pos] != '"' && source[pos] != '\0') pos++;
        if (source[pos] == '\0') {
            fprintf(stderr, "Unterminated string literal\n");
            exit(1);
        }
        strncpy(token.data, &source[start], pos - start);
        token.data[pos - start] = '\0';
        pos++;
        return token;
    }

    // Handle identifiers and keywords
    if (isalpha(source[pos])) {
        token.type = TOKEN_IDENTIFIER;
        int start = pos;
        while (isalnum(source[pos])) pos++;
        strncpy(token.data, &source[start], pos - start);
        token.data[pos - start] = '\0';
        if (strcmp(token.data, "print") == 0) {
            token.type = TOKEN_PRINT; // Recognize 'print' keyword
        } else if (strcmp(token.data, "while") == 0) {
            token.type = TOKEN_WHILE;
        }
        return token;
    }

    // Next few blocks handle multi-character tokens 
    if (source[pos] == '=') {
        if (source[pos + 1] == '=') {
            token.type = TOKEN_EQUALS;
            strcpy(token.data, "==");
            pos += 2;
            return token;
        }
        token.type = TOKEN_ASSIGN;
        strcpy(token.data, "=");
        pos++;
        return token;
    }

    if (source[pos] == '!') {
        if (source[pos + 1] == '=') {
            token.type = TOKEN_NOT_EQ;
            strcpy(token.data, "!=");
            pos += 2;
            return token;
        }
        token.type = TOKEN_NOT;
        strcpy(token.data, "!");
        pos++;
        return token;
    }

    switch (source[pos]) {
        case '+': token = (Token){TOKEN_PLUS, "+"}; break;
        case '-': token = (Token){TOKEN_MINUS, "-"}; break;
        case '*': token = (Token){TOKEN_STAR, "*"}; break;
        case '/': token = (Token){TOKEN_SLASH, "/"}; break;
        case '=': token = (Token){TOKEN_ASSIGN, "="}; break;
        case ';': token = (Token){TOKEN_SEMICOLON, ";"}; break;
        case '(': token = (Token){TOKEN_LPAREN, "("}; break;
        case ')': token = (Token){TOKEN_RPAREN, ")"}; break;
        case '{': token = (Token){TOKEN_LBRACE, "{"}; break;
        case '}': token = (Token){TOKEN_RBRACE, "}"}; break;
        case '<': token = (Token){TOKEN_LT, "<"}; break;
        case '>': token = (Token){TOKEN_GT, ">"}; break;
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
        // The last empty token was causing issues. The following code fixes it
        if (currentToken.type != type) {
            if (currentToken.type != TOKEN_EOF) {
                fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
            } else {
                exit(1);
            }
        }
        fprintf(stderr, "Unexpected token: %s\n", currentToken.data);
        exit(1);
    }
}

// Parses and evaluates factors (numbers, variables, or strings)
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
    } else if (currentToken.type == TOKEN_STRING) {
        char value[256];
        strcpy(value, currentToken.data);
        match(TOKEN_STRING);
        // Handle string literals in expressions if needed
        // For now, just return 0.0 as a placeholder
        return 0.0;
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

// Parses and evaluates conditions (handles '==', '!=', '<', '>' operators)
int condition() {
    double left = expression();
    TokenType op = currentToken.type;
    
    if (op != TOKEN_EQUALS && op != TOKEN_NOT_EQ && 
        op != TOKEN_LT && op != TOKEN_GT) {
        fprintf(stderr, "Expected comparison operator, got %s\n", currentToken.data);
        exit(1);
    }
    
    match(op);
    double right = expression();
    
    switch (op) {
        case TOKEN_EQUALS: return left == right;
        case TOKEN_NOT_EQ: return left != right;
        case TOKEN_LT: return left < right;
        case TOKEN_GT: return left > right;
        default: return 0;
    }
}

// Add these helper functions
void saveState(int *saved_pos, Token *saved_token) {
    *saved_pos = pos;
    *saved_token = currentToken;
}

void restoreState(int saved_pos, Token saved_token) {
    pos = saved_pos;
    currentToken = saved_token;
}

// while loop implementation
void while_statement() {
    Token saved_token;
    int saved_pos;
    
    match(TOKEN_WHILE);
    match(TOKEN_LPAREN);
    
    // Save position before first condition check
    saveState(&saved_pos, &saved_token);
    
    while (1) { // Using while loop to make while loop in new language
        // Evaluate condition
        int result = condition();
        match(TOKEN_RPAREN);
        
        if (!result) {
            // Skip block if condition is false
            int brace_count = 1;
            match(TOKEN_LBRACE);
            
            while (brace_count > 0) {
                if (currentToken.type == TOKEN_LBRACE) brace_count++;
                if (currentToken.type == TOKEN_RBRACE) brace_count--;
                match(currentToken.type);
            }
            break;
        }
        
        // Execute block
        block_statement();
        
        // Go back to condition
        restoreState(saved_pos, saved_token);
    }
}

// Update block_statement to be simpler
void block_statement() {
    match(TOKEN_LBRACE);
    while (currentToken.type != TOKEN_RBRACE) {
        if (currentToken.type == TOKEN_EOF) {
            fprintf(stderr, "Unexpected end of file inside block\n");
            exit(1);
        }
        statement();
    }
    match(TOKEN_RBRACE);
}

// Parses and executes statements (assignments, print statements, or while loops)
void statement() {
    if (currentToken.type == TOKEN_WHILE) {
        while_statement();
    } else if (currentToken.type == TOKEN_PRINT) {
        // Handle 'print(expression);'
        match(TOKEN_PRINT);
        match(TOKEN_LPAREN);
        if (currentToken.type == TOKEN_STRING) {
            printf("%s\n", currentToken.data);
            match(TOKEN_STRING);
        } else if (currentToken.type == TOKEN_IDENTIFIER) {
            char name[64];
            strcpy(name, currentToken.data);
            match(TOKEN_IDENTIFIER);
            if (variables[getVariableIndex(name)].type == VAR_STRING) {
                printf("%s\n", getStringValue(name));
            } else {
                printf("%lf\n", getVariableValue(name));
            }
        } else {
            double value = expression();
            printf("%lf\n", value);
        }
        match(TOKEN_RPAREN);
        match(TOKEN_SEMICOLON);
    } else if (currentToken.type == TOKEN_IDENTIFIER) {
        // Handle 'identifier = expression;'
        char name[64];
        strcpy(name, currentToken.data);
        match(TOKEN_IDENTIFIER);
        match(TOKEN_ASSIGN);
        if (currentToken.type == TOKEN_STRING) {
            char value[256];
            strcpy(value, currentToken.data);
            match(TOKEN_STRING);
            setStringValue(name, value);
        } else {
            double value = expression();
            setVariableValue(name, value);
        }
        match(TOKEN_SEMICOLON);
    } else {
        fprintf(stderr, "Unknown statement\n");
        exit(1);
    }
}

// Helper function to get the index of a variable in the symbol table
int getVariableIndex(const char *name) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

// Retrieves the value of a variable from the symbol table
double getVariableValue(const char *name) {
    int index = getVariableIndex(name);
    if (index != -1) {
        if (variables[index].type == VAR_NUMBER) {
            return variables[index].value.number;
        } else {
            fprintf(stderr, "Expected number, got string\n");
            exit(1);
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

// Retrieves the value of a string variable from the symbol table
const char* getStringValue(const char *name) {
    int index = getVariableIndex(name);
    if (index != -1) {
        if (variables[index].type == VAR_STRING) {
            return variables[index].value.string;
        } else {
            fprintf(stderr, "Expected string, got number\n");
            exit(1);
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
}

// Stores or updates a numeric variable in the symbol table
void setVariableValue(const char *name, double value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].type = VAR_NUMBER;
            variables[i].value.number = value;
            return;
        }
    }
    
    if (variableCount >= MAX_VARIABLES) {
        fprintf(stderr, "Too many variables\n");
        exit(1);
    }
    
    strcpy(variables[variableCount].name, name);
    variables[variableCount].type = VAR_NUMBER;
    variables[variableCount].value.number = value;
    variableCount++;
}

// Stores or updates a string variable in the symbol table
void setStringValue(const char *name, const char *value) {
    for (int i = 0; i < variableCount; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].type = VAR_STRING;
            strcpy(variables[i].value.string, value);
            return;
        }
    }
    
    if (variableCount >= MAX_VARIABLES) {
        fprintf(stderr, "Too many variables\n");
        exit(1);
    }
    
    strcpy(variables[variableCount].name, name);
    variables[variableCount].type = VAR_STRING;
    strcpy(variables[variableCount].value.string, value);
    variableCount++;
}

// Reads the content of a file into a string; Used for reading code from external files
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
