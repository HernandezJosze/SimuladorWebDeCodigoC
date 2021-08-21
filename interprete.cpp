#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "lib/debug.h"
#include "lib/lexer.h"
#include "lib/parser.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

int linea_de(const char* ini, const token_anotada& t) {
   return 1 + std::count(ini, t.location.begin( ), '\n');
}

int columna_de(const char* ini, const token_anotada& t) {
   return 1 + (t.location.begin( ) - std::find(std::reverse_iterator(t.location.begin( )), std::reverse_iterator(ini), '\n').base( ));
}

std::string_view vista_de(const token_anotada& t, int len, const char* fin) {
   return { t.location.begin( ), size_t(std::find(t.location.begin( ), (fin - t.location.begin( ) <= len ? fin : t.location.begin( ) + len), '\n') - t.location.begin( )) };
}

extern "C"
#ifdef __EMSCRIPTEN__
   EMSCRIPTEN_KEEPALIVE
#endif
const char* interpreta(const char* codigo) {
   static std::string res = "";

   try {
      std::cout << "TOKENS:\n";
      std::vector<token_anotada> v = lexer(codigo);
      for (const auto& actual : v) {
         std::cout << actual.tipo << " " << actual << "\n";
      }
      std::cout << "SENTENCIAS:\n";
      std::vector<std::unique_ptr<sentencia>> w = parser(v.data( ));
      for (const auto& actual : w) {
         std::cout << *actual << "\n";
      }
   } catch (const std::pair<token_anotada, const char*>& e) {
      auto ini = codigo, fin = codigo + strlen(codigo);
      std::cout << "Error en linea " << linea_de(ini, e.first) << ", columna " << columna_de(ini, e.first) << ":\n"
                << "\t" << vista_de(e.first, 10, fin) << "\n"
                << "\t^\n"
                << e.second << "\n";
   }
   return res.c_str( );
}

int main(int argc, const char* argv[]) {
#ifdef __EMSCRIPTEN__
   return 0;
#else
   if(argc < 2){
      std::cout << "Modo de uso: " << argv[0] << " ruta_codigo";
      return 0;
   }

   std::ifstream file(argv[1], std::ios::in);
   if (!file.is_open( )) {
      std::cout << "No se pudo abrir " << argv[1] << "\n";
      return 0;
   }
   std::ostringstream buffer;
   buffer << file.rdbuf( );
   std::cout << interpreta(std::move(buffer).str( ).c_str( ));
#endif
}
