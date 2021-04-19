//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_LEXER_H
#define SIMULADORWEBDECODIGOC_LEXER_H
#include <map>

enum token{
    IF,
    ELSE,
    FOR,
    DO,
    WHILE,
    INT,
    FLOAT,
    BREAK,
    CONTINUE,
    MAS,
    MENOS,
    MULTIPLICACION,
    MODULO,
    DIVISION,
    MENOR,
    MAYOR,
    MENOR_IGUAL,
    MAYOR_IGUAL,
    DIFERENTE,
    IGUAL,
    ASIGNACION,
    CORCHETE_I,
    CORCHETE_D,
    LLAVE_I,
    LLAVE_D,
    PARENTESIS_I,
    PARENTESIS_D,
    COMA,
    PUNTO_Y_COMA,
    LITERAL_ENTERA,
    LITERAL_FLOTANTE,
    LITERAL_CADENA,
    IDENTIFICADOR,
    END_FILE
};
struct token_anotada{
    token tipo;
    std::string_view location;
};

std::map<std::string_view, token> tokens {
    {"if", IF},
    {"else", ELSE},
    {"for", FOR},
    {"do", DO},
    {"while", WHILE},
    {"int", INT},
    {"float", FLOAT},
    {"break", BREAK},
    {"continue", CONTINUE},
    {"+", MAX},
    {"-", MENOS},
    {"*", MULTIPLICACION},
    {"%", MODULO},
    {"/", DIVISION},
    {"<", MENOR},
    {">", MAYOR},
    {"<=", MENOR_IGUAL},
    {">=", MAYOR_IGUAL},
    {"!=", DIFERENTE},
    {"==", IGUAL},
    {"=", ASIGNACION},
    {"[", CORCHETE_I},
    {"]", CORCHETE_D},
    {"{", LLAVE_I},
    {"}", LLAVE_D},
    {"(", PARENTESIS_I},
    {")", PARENTESIS_D},
    {",", COMA},
    {";", PUNTO_Y_COMA},
    {"\0", END_FILE}
};

std::vector<token_anotada> lexer(const char* s){
    std::vector<token_anotada> vectorLexer;
    while(s){

    }
    return vectorLexer;
}


#endif //SIMULADORWEBDECODIGOC_LEXER_H