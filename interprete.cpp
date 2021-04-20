#include <string>
#include <iostream>
#include <fstream>
#include "lib/lexer.h"
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern "C" {
#ifdef __EMSCRIPTEN__
EMSCRIPTEN_KEEPALIVE
#endif
const char* interpreta(const char* codigo) {
   static std::string res = "";
   //something
   std::vector<token_anotada> v = lexer(codigo);
   for(int i = 0; i < v.size( ); ++i){
      std::cout << v[i].location << '\n';
   }
   return res.c_str( );
}
}

int main(int argc, const char* argv[]) {
#ifdef __EMSCRIPTEN__
   return 0;
#else
   /*
   if(argc < 2){
      std::cout << "Modo de uso: " << argv[0] << " ruta_codigo";
      return 0;
   }*/
   std::string s;
   std::ifstream file;
   file.open("cpp.txt", std::ios::in);
   std::string r;
   if(file.is_open( )) {
      std::cout << "leyendo..." << std::endl;
      while (!file.eof()) {
         std::getline(file, r);
         s+=r;
      }
   }
   std::cout << interpreta(s.c_str());
   file.close( );
#endif
}
