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
   static std::string res = "";

   try {
      std::cerr << "TOKENS:\n";
      std::vector<token_anotada> v = lexer(codigo);
      for (const auto& actual : v) {
         std::cerr << actual.tipo << " " << actual << "\n";
      }
      std::cerr << "SENTENCIAS:\n";
      std::vector<std::unique_ptr<sentencia>> w = parser(v.data( ));
      for (const auto& actual : w) {
         std::cerr << *actual << "\n";
      }
      std::cerr << "QUIERES PROBAR EL SEMANTICO? ";
      int probar;
      std::cin >> probar; getchar( );
      if (probar == 1) {
         std::istringstream iss(entrada);
         std::ostringstream oss;
         evalua(w, { iss, oss });
         res = std::move(oss).str( );
         std::cerr << "FIN PRUEBA\n";
      }
   } catch (const error& e) {
      auto ini = codigo, fin = codigo + strlen(codigo);
      std::cout << "\nError en linea " << linea_de(ini, e.tk) << ", columna " << columna_de(ini, e.tk) << ":\n"
                << "\t" << vista_de(e.tk, 10, fin) << "\n"
                << "\t^\n"
                << e.message << "\n";
   } catch (const std::exception& e) {
      std::cout << e.what( ) << "\n";
   }
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
