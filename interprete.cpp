#include <string>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include "C:\Users\Hernandez\CLionProjects\SimuladorWebDeCodigoC\lib\lexer.h"
#ifdef __EMSCRIPTEN__
   #include <emscripten.h>
#endif

extern "C" {
   #ifdef __EMSCRIPTEN__
      EMSCRIPTEN_KEEPALIVE
   #endif

   const char* interpreta(char* codigo) {
      static std::string res = ""
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
		if(argc < 2){
		    std::cout << "Modo de uso: " << argv[0] << << " ruta_codigo";
		    return 0;
		}
		std::string s;
        std::ifstream file;
        file.open("cpp.txt", ios::in);
        if(!file.fail( )) {
            std::string r;
            std::getline(file, r);
            s+=' ' + r;
        }
        std::cout << interpreta(s.c_str());
	#endif
}
