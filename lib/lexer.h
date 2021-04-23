//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_LEXER_H
#define SIMULADORWEBDECODIGOC_LEXER_H
#include <map>
#include <cctype>
#include <vector>
#include <string_view>
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
   RETURN,
   END_FILE
};
struct token_anotada{
   token tipo;
   std::string_view location;
};

std::map<std::string_view, token> AlphaMp{
        {"if", IF},
        {"else", ELSE},
        {"for", FOR},
        {"do", DO},
        {"while", WHILE},
        {"int", INT},
        {"float", FLOAT},
        {"break", BREAK},
        {"continue", CONTINUE},
        {"return", RETURN}
};
std::map<std::string_view, token> SymbolsMp{
        {"+", MAS},
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
};
bool isYetId(const char& c){
   return std::isalnum(c) || c == '_';
}/*
bool isAStartId(const char& c){
    return std::isalpha(c) || c == '_';
}*/
bool isIntoAlphaMp(std::vector<token_anotada>& v,const char *ptr){
   std::string s;
   const char *ptr_end = ptr;
   while(std::isalpha(*ptr_end)) {
      s.push_back(*ptr_end++);
      auto fnd = AlphaMp.find(s);
      //std::cout << "Alpha: " << s << "\n";
      if (fnd != AlphaMp.end()) {
         //std::cout << "Alpha: " << s << "\n";
         //std::cout << "->" << *(ptr_end) << "<-\n";
         if (!isYetId(*(ptr_end))) {
            //std::cout << "find\n";
            v.push_back({fnd->second, std::string_view(ptr, ptr_end)});
            return true;
         }
         return false;
      }
   }
   return false;
}
bool isIntoSymbolsMp(std::vector<token_anotada>& v,const char *ptr){
   std::string s;
   const char *ptr_end = ptr;
   while(!std::isalnum(*ptr_end)){
      //std::cout << "Symbol: " << *ptr_end << "\n";
      s.push_back(*ptr_end++);
      //std::cout << s << "\n";
      auto fnd = SymbolsMp.find(s);
      if(fnd != SymbolsMp.end( )){
         //std::cout << "->" << int(*ptr_end) << "<-" << std::endl;
         if(SymbolsMp.find(s + *ptr_end) == SymbolsMp.end( )) {
            //std::cout << "find\n";
            v.push_back({fnd->second, std::string_view(ptr, ptr_end)});
            return true;
         }
      }
   }
   return false;
}
void isLiteralCad(std::vector<token_anotada>& v,const char *ptr, char type) {
   const char* ptr_end = ptr;
   while(*ptr_end != type || *(ptr_end - 1) == '\\') {
      //std::cout << "cad: " << *ptr_end << "\n";
      ++ptr_end;
   }
   v.push_back({LITERAL_CADENA, std::string_view(ptr, ptr_end)});
}
void numeric(std::vector<token_anotada>& v,const char *ptr){
   const char *ptr_end = ptr;
   while(std::isdigit(*ptr_end)) {
      //std::cout << "digito: " << *ptr_end << "\n";
      ++ptr_end;
   }
   bool isFloat = *ptr_end == '.';
   if(isFloat)
      ptr_end++;
   while(std::isdigit(*ptr_end)) {
      //std::cout << "digito: " << *ptr_end << "\n";
      ++ptr_end;
   }
   v.push_back({isFloat ? LITERAL_FLOTANTE : LITERAL_ENTERA, std::string_view(ptr, ptr_end)});
}
void id(std::vector<token_anotada>& v,const char *ptr){
   const char *ptr_end = ptr;
   while(isYetId(*ptr_end)){
      //std::cout << "id: " << *ptr_end << "\n";
      ++ptr_end;
   }

   v.push_back({IDENTIFICADOR, std::string_view(ptr, ptr_end)});
}

std::vector<token_anotada> lexer(const char* ptr){
   std::vector<token_anotada> vectorLexer;
   while(*ptr != '\0'){
      //std::cout << "caracter: " << int(*ptr) << "\n";
      if(std::isblank(*ptr) || *ptr == 13){
         ++ptr;
      }else if(*ptr == '\"') {
         isLiteralCad(vectorLexer, ++ptr, '\"');
         std::advance(ptr, vectorLexer.back( ).location.size( ) + 1);

      }else if(*ptr == '\'') {
         isLiteralCad(vectorLexer, ++ptr, '\'');
         std::advance(ptr, vectorLexer.back( ).location.size( ) + 1);

      }else if(isIntoAlphaMp(vectorLexer, ptr) || isIntoSymbolsMp(vectorLexer, ptr)){
         std::advance(ptr, vectorLexer.back( ).location.size( ));
      }else{
         if(std::isdigit(*ptr)){
            numeric(vectorLexer, ptr);
            std::advance(ptr, vectorLexer.back().location.size( ));
         }else{
            id(vectorLexer, ptr);
            std::advance(ptr, vectorLexer.back().location.size( ));
         }
      }
   }
   vectorLexer.push_back({END_FILE, std::string_view(ptr, ptr + 1)});
   return vectorLexer;
}

#endif //SIMULADORWEBDECODIGOC_LEXER_H
