#ifndef SIMULADORWEBDECODIGOC_LEXER_H
#define SIMULADORWEBDECODIGOC_LEXER_H
#include <map>
#include <cctype>
#include <cstdlib>
#include <initializer_list>
#include <vector>
#include <string_view>
#include <cmath>
#include <cctype>
#include <memory>

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
   LITERAL_ENTERA,
   LITERAL_FLOTANTE,
   LITERAL_CADENA,
   IDENTIFICADOR,
   END_FILE,
   WRONG_TOKEN,
   MAS,
   MENOS,
   NOT,
   MULTIPLICACION,
   MODULO,
   DIVISION,
   DIRECCION,
   MENOR,
   MAYOR,
   MENOR_IGUAL,
   MAYOR_IGUAL,
   DIFERENTE,
   IGUAL,
   INCREMENTO,
   DECREMENTO,
   MAS_IGUAL,
   MENOS_IGUAL,
   MULTIPLICA_IGUAL,
   DIVIDE_IGUAL,
   MODULO_IGUAL,
   ASIGNACION,
   CORCHETE_I,
   CORCHETE_D,
   LLAVE_I,
   LLAVE_D,
   PARENTESIS_I,
   PARENTESIS_D,
   COMA,
   PUNTO_Y_COMA,
   AND,
   OR
};
struct token_anotada{
   token tipo;
   std::string_view location;
};
struct trie {
   token tipo = WRONG_TOKEN;
   std::map<char, std::unique_ptr<trie>> hijos;

   trie( ) = default;
   trie(std::initializer_list<std::pair<std::string_view, token>> tokens) {
      for (auto [simbolo, tipo] : tokens) {
         auto actual = this;
         for (int i = 0; i < simbolo.size( ); ++i) {
            auto& siguiente = actual->hijos[simbolo[i]];
            if (siguiente == nullptr) {
               siguiente = std::make_unique<trie>( );
            }
            actual = siguiente.get( );
         }
         actual->tipo = tipo;
      }
   }
   bool find(const char* ptr, int& advance, token& tipo) const {
      auto actual = this;
      advance = 0, tipo = actual->tipo;
      for (int i = 0; ; ++i) {
         auto iter = actual->hijos.find(ptr[i]);
         if (iter == actual->hijos.end( )) {
            return (tipo != WRONG_TOKEN);
         }
         actual = iter->second.get( );
         if (actual->tipo != WRONG_TOKEN) {
            advance = i + 1, tipo = actual->tipo;
         }
      }
   }
};
struct error{
   token_anotada tk;
   std::string message;

   error(const token_anotada& t, std::string&& s)
   : tk(t), message(std::move(s)) {
   }
};

const trie keywords = {
   {"if", IF},
   {"else", ELSE},
   {"for", FOR},
   {"do", DO},
   {"while", WHILE},
   {"int", INT},
   {"float", FLOAT},
   {"break", BREAK},
   {"continue", CONTINUE}
};
const trie symbols = {
   {"+", MAS},
   {"-", MENOS},
   {"!", NOT},
   {"*", MULTIPLICACION},
   {"%", MODULO},
   {"/", DIVISION},
   {"&", DIRECCION},
   {"<", MENOR},
   {">", MAYOR},
   {"<=", MENOR_IGUAL},
   {">=", MAYOR_IGUAL},
   {"!=", DIFERENTE},
   {"==", IGUAL},
   {"++", INCREMENTO},
   {"--", DECREMENTO},
   {"+=", MAS_IGUAL},
   {"-=", MENOS_IGUAL},
   {"*=", MULTIPLICA_IGUAL},
   {"/=", DIVIDE_IGUAL},
   {"%=", MODULO_IGUAL},
   {"=", ASIGNACION},
   {"[", CORCHETE_I},
   {"]", CORCHETE_D},
   {"{", LLAVE_I},
   {"}", LLAVE_D},
   {"(", PARENTESIS_I},
   {")", PARENTESIS_D},
   {",", COMA},
   {";", PUNTO_Y_COMA},
   {"&&", AND},
   {"||", OR}
};

bool isComment(const char* ptr, int& advance){
   auto ptr_end = ptr;
   while(*ptr_end == '/'){
      ++ptr_end;
   }
   if(ptr_end - ptr < 2) {
      return false;
   }
   while(*ptr_end != '\0' && *ptr_end != '\n'){
      ++ptr_end;
   }
   advance = ptr_end - ptr;
   return true;
}
bool isString(const char *ptr, int& advance, token& tipo){
   auto ptr_end = ptr;
   if (*ptr_end++ != '"') {
      return false;
   }
   while(*ptr_end != '"') {
      if(*ptr_end == '\\') {
         ++ptr_end;
      }
      if(*ptr_end == '\0'){
         return false;
      }
      ++ptr_end;
   }
   advance = ++ptr_end - ptr, tipo = LITERAL_CADENA;
   return true;
}
bool isNumeric(const char *ptr, int& advance, token& tipo){
   auto ptr_endi = const_cast<char*>(ptr), ptr_endf = ptr_endi;
   auto resi = strtol(ptr, &ptr_endi, 0);
   auto resf = strtod(ptr, &ptr_endf);
   if (ptr_endi == ptr && ptr_endf == ptr) {
      return false;
   }
   advance = std::max(ptr_endi, ptr_endf) - ptr, tipo = (ptr_endf > ptr_endi ? LITERAL_FLOTANTE : LITERAL_ENTERA);
   return true;
}
bool isSymbol(const char *ptr, int& advance, token& tipo){
   return symbols.find(ptr, advance, tipo);
}
bool isAlphaNumeric(const char *ptr, int& advance, token& tipo){
   auto ptr_end = ptr;
   if (*ptr_end != '_' && !std::isalpha(*ptr_end)) {
      return false;
   }
   do {
      ++ptr_end;
   } while (*ptr_end == '_' || std::isalnum(*ptr_end));

   if (!keywords.find(ptr, advance, tipo) || advance != ptr_end - ptr) {
      advance = ptr_end - ptr, tipo = IDENTIFICADOR;
   }
   return true;
}
std::vector<token_anotada> lexer(const char* ptr){
   std::vector<token_anotada> vectorLexer;
   while(*ptr != '\0'){
      int advance; token tipo;
      if(std::isspace(*ptr)){
         ptr += 1;
      }else if(isComment(ptr, advance)) {
         ptr += advance;
      }else if(isString(ptr, advance, tipo) ||
               isSymbol(ptr, advance, tipo) ||
               isNumeric(ptr, advance, tipo) ||
               isAlphaNumeric(ptr, advance, tipo)) {
         vectorLexer.push_back(token_anotada{tipo, std::string_view(ptr, advance)});
         ptr += advance;
      }else{
         throw error(token_anotada{WRONG_TOKEN, std::string_view(ptr, 1)}, "Lexer error");
      }
   }
   vectorLexer.push_back({END_FILE, std::string_view(ptr, 1)});
   return vectorLexer;
}

#endif //SIMULADORWEBDECODIGOC_LEXER_H
