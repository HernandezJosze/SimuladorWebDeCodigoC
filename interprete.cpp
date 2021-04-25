#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "lib/lexer.h"
#include "lib/parser.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern "C"
#ifdef __EMSCRIPTEN__
   EMSCRIPTEN_KEEPALIVE
#endif
const char* interpreta(const char* codigo) {
   static std::string res = "";
   std::vector<token_anotada> v = lexer(codigo);
   for(int i = 0; i < v.size( ); ++i){
      std::cout << v[i].tipo << " " << v[i].location << '\n';
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
