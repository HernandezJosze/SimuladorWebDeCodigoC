#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include "lib/debug.h"
#include "lib/lexer.h"
#include "lib/parser.h"
#include "lib/semantico_ejecutor.h"
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
const char* interpreta(const char* codigo, const char* entrada) {
   std::ostringstream oss;
   try {
      std::vector<token_anotada> v = lexer(codigo);
#ifndef __EMSCRIPTEN__
      std::cerr << "TOKENS:\n";
      for (const auto& actual : v) {
         std::cerr << actual.tipo << " " << actual << "\n";
      }
#endif
      std::vector<std::unique_ptr<sentencia>> w = parser(v.data( ));
#ifndef __EMSCRIPTEN__
      std::cerr << "SENTENCIAS:\n";
      for (const auto& actual : w) {
         std::cerr << *actual << "\n";
      }
#endif
      int probar = 1;
#ifndef __EMSCRIPTEN__
      std::cerr << "QUIERES PROBAR EL SEMANTICO? ";
      std::cin >> probar; getchar( );
#endif
      if (probar == 1) {
         std::istringstream iss(entrada);
         evalua(w, { iss, oss });
      }
   } catch (const error& e) {
      auto ini = codigo, fin = codigo + strlen(codigo);
      oss << "\nError en linea " << linea_de(ini, e.tk) << ", columna " << columna_de(ini, e.tk) << ":\n"
                << "\t" << vista_de(e.tk, 10, fin) << "\n"
                << "\t^\n"
                << e.message << "\n";
   } catch (const std::exception& e) {
      oss << e.what( ) << "\n";
   }

   static std::string res;
   res = std::move(oss).str( );
   return res.c_str( );
}

int main(int argc, const char* argv[]) {
#ifdef __EMSCRIPTEN__
   return 0;
#else
   if(argc < 2){
      std::cout << "Modo de uso: " << argv[0] << " ruta_codigo ruta_entrada";
      return 0;
   }

   std::ifstream codigo(argv[1]), entrada(argv[2]);
   if (!codigo.is_open( ) || !entrada.is_open( )) {
      std::cout << "No se pudieron abrir los archivos.\n";
      return 0;
   }

   std::ostringstream codigo_str, entrada_str;
   codigo_str << codigo.rdbuf( ), entrada_str << entrada.rdbuf( );
   std::cout << interpreta(std::move(codigo_str).str( ).c_str( ), std::move(entrada_str).str( ).c_str( ));
#endif
}
