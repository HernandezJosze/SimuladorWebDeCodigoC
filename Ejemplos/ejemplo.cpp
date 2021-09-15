#include <string>
#include <stdio.h>
#include <string.h>
#ifdef __EMSCRIPTEN__
   #include <emscripten.h>
#endif

extern "C" {
   #ifdef __EMSCRIPTEN__
      EMSCRIPTEN_KEEPALIVE
   #endif
   int ejecuta1(const char* codigo) {
      return strlen(codigo);
   }

   #ifdef __EMSCRIPTEN__
      EMSCRIPTEN_KEEPALIVE
   #endif
   const char* ejecuta2(const char* codigo) {
      static std::string res;
      res = std::to_string(strlen(codigo)) + "!";
      return res.c_str( );
   }
}

int main(int argc, const char* argv[]) {
	#ifdef __EMSCRIPTEN__
		return 0;
	#else
		printf("Fuera de WebAssembly");
	#endif
}
