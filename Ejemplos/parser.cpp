#include <iostream>
#include <memory>
#include <vector>

/*
   Suponer un lenguaje (sin sentido) como el siguiente:
      EJECUTA a + b;
      EJECUTA x - y;
      IF (a - b) {
         EJECUTA a[5]
      }
      EJECUTA z[2 - 2]

   En este lenguaje tonto, las expresiones son del tipo:
      entero
      identificador + identificador
      identificador - identificador
      identificador[expresion]

   En este lenguaje tonto, las sentencias son del tipo:
      EJECUTA expresion;
      IF (expresion) {
         cero o más sentencias
      }
*/

// LEXER ------------------------------------------------- en este ejemplo no tendremos token_anotada y daremos directamente la lista de tokens en el código

enum token {
   EJECUTA,
   IF,
   PARENTESIS_IZQ,
   PARENTESIS_DER,
   CORCHETE_IZQ,
   CORCHETE_DER,
   LLAVE_IZQ,
   LLAVE_DER,
   PUNTO_COMA,
   MAS,
   MENOS,
   IDENTIFICADOR,
   LITERAL_ENTERA,
   FIN_ARCHIVO
};

// PARSER (expresiones) -------------------------------------------------

struct expresion {
   virtual ~expresion() = 0;
};
expresion::~expresion(){}

struct expresion_terminal : expresion {      // identificador o literal entera
   const token* t;

   expresion_terminal(const token* tp)
   : t(tp) {
   }
};

struct expresion_op_binario : expresion {    // identificador + identificador      identificador - identificador
   const token* izq;
   const token* op;
   const token* der;

   expresion_op_binario(const token* i, const token* o, const token* d)
   : izq(i), op(o), der(d) {
   }
};

struct expresion_corchetes : expresion{     // identificador[expresion]
   const token* izq;
   std::unique_ptr<expresion> dentro;

   expresion_corchetes(const token* i, std::unique_ptr<expresion>&& e)
   : izq(i), dentro(std::move(e)) {
   }
};

// funciones auxiliares

bool es_operador_binario(token t) {
   return t == MAS || t == MENOS;
}

const token* espera(const token*& iter, auto pred){            // si lo que vemos cumple el predicado, avanzamos el iterador pero regresamos el que había cumplido; en caso contrario es un error fatal
   if(!pred(*iter)){
      throw std::runtime_error("ERROR PARSER");
   }
   return iter++;
}

const token* espera(const token*& iter, token esperada){       // si lo que vemos es el token que esperamos, avanzamos el iterador pero regresamos el que había cumplido; en caso contrario es un error fatal
   return espera(iter, [&](token tipo) {
      return tipo == esperada;
   });
}

// funciones de parsing

std::unique_ptr<expresion> parsea_expresion(const token*& iter){      // por referencia porque iremos avanzando
   /*
      Reconocer:

      entero
      identificador + identificador
      identificador - identificador
      identificador[expresion]
   */

   if(*iter == LITERAL_ENTERA) {
      return std::make_unique<expresion_terminal>(iter++);
   }

   auto izq = espera(iter, IDENTIFICADOR);         // si no era entero, entonces forzosamente debe ser identificador
   if (*iter == CORCHETE_IZQ) {                    // ¿tenemos corchete?
      ++iter;                                      // lo ignoramos
      auto expr = parsea_expresion(iter);          // jalamos la expresión interna de los corchetes
      espera(iter, CORCHETE_DER);                  // forzosamente debe venir el que cierra
      return std::make_unique<expresion_corchetes>(izq, std::move(expr));
   } else {
      auto op = espera(iter, es_operador_binario);    // forzosamente debe venir un operador binario
      auto der = espera(iter, IDENTIFICADOR);         // forzosamente debe venir otro identificador
      return std::make_unique<expresion_op_binario>(izq, op, der);
   }
}

// PARSER (sentencias) -------------------------------------------------

struct sentencia {
   virtual ~sentencia() = 0;
};
sentencia::~sentencia(){}

struct sentencia_ejecuta : sentencia {       // EJECUTA expresión
   std::unique_ptr<expresion> ex;

   sentencia_ejecuta(std::unique_ptr<expresion>&& e)
   : ex(std::move(e)) {
   }
};

struct sentencia_if : sentencia {    // IF (expresión) { cero o más sentencias }
   std::unique_ptr<expresion> ex;
   std::vector<std::unique_ptr<sentencia>> sentencias;

   sentencia_if(std::unique_ptr<expresion>&& e, std::vector<std::unique_ptr<sentencia>>&& s)
   : ex(std::move(e)), sentencias(std::move(s)) {
   }
};

// funciones de parsing

std::unique_ptr<sentencia> parsea_sentencia(const token*& iter) {
   /*
      Reconocer:

      EJECUTA expresion;
      IF (expresion) {
         cero o más sentencias
      }
   */

   if (*iter == EJECUTA) {
      ++iter;
      auto ex = parsea_expresion(iter);
      espera(iter, PUNTO_COMA);
      return std::make_unique<sentencia_ejecuta>(std::move(ex));
   }

   espera(iter, IF);
   espera(iter, PARENTESIS_IZQ);
   auto ex = parsea_expresion(iter);
   espera(iter, PARENTESIS_DER);
   espera(iter, LLAVE_IZQ);
   std::vector<std::unique_ptr<sentencia>> sentencias;
   while (*iter != LLAVE_DER) {
      sentencias.push_back(parsea_sentencia(iter));      // recursivo
   }
   espera(iter, LLAVE_DER);      // ya sabemos que viene (sino el ciclo while no hubiera terminado), pero lo hacemos explícito
   return std::make_unique<sentencia_if>(std::move(ex), std::move(sentencias));
}

// PARSER (general) -------------------------------------------------

std::vector<std::unique_ptr<sentencia>> parsea(const token* iter) {
   std::vector<std::unique_ptr<sentencia>> res;
   while (*iter != FIN_ARCHIVO) {
      res.push_back(parsea_sentencia(iter));
   }
   return res;
}

// IMPRESION -------------------------------------------------

void imprime(const expresion&);

void imprime(const expresion_terminal& ex) {
   std::cout << *ex.t;
}

void imprime(const expresion_op_binario& ex) {
   std::cout << *ex.izq << " " << *ex.op << " " << *ex.der;
}

void imprime(const expresion_corchetes& ex) {
   std::cout << *ex.izq << "[";
   imprime(*ex.dentro);
   std::cout << "]";
}

void imprime(const expresion& ex) {
   if (typeid(ex) == typeid(expresion_terminal)) {
      imprime(dynamic_cast<const expresion_terminal&>(ex));
   } else if (typeid(ex) == typeid(expresion_op_binario)) {
      imprime(dynamic_cast<const expresion_op_binario&>(ex));
   } else if (typeid(ex) == typeid(expresion_corchetes)) {
      imprime(dynamic_cast<const expresion_corchetes&>(ex));
   }
}

void imprime(const sentencia&);

void imprime(const sentencia_ejecuta& s) {
   std::cout << "EJECUTA ";
   imprime(*s.ex);
   std::cout << ";\n";
}

void imprime(const sentencia_if& s) {
   std::cout << "IF (";
   imprime(*s.ex);
   std::cout << ") {\n";
   for (const auto& p : s.sentencias) {
      imprime(*p);
   }
   std::cout << "}\n";
}

void imprime(const sentencia& s) {
   if (typeid(s) == typeid(sentencia_ejecuta)) {
      imprime(dynamic_cast<const sentencia_ejecuta&>(s));
   } else if (typeid(s) == typeid(sentencia_if)) {
      imprime(dynamic_cast<const sentencia_if&>(s));
   }
}

// MAIN -------------------------------------------------

int main( ) {
   token tokens[] = {
      EJECUTA, IDENTIFICADOR, MAS, IDENTIFICADOR, PUNTO_COMA,
      EJECUTA, IDENTIFICADOR, CORCHETE_IZQ, IDENTIFICADOR, MENOS, IDENTIFICADOR, CORCHETE_DER, PUNTO_COMA,
      IF, PARENTESIS_IZQ, LITERAL_ENTERA, PARENTESIS_DER, LLAVE_IZQ,
         EJECUTA, LITERAL_ENTERA, PUNTO_COMA,
         EJECUTA, LITERAL_ENTERA, PUNTO_COMA,
      LLAVE_DER,
      EJECUTA, IDENTIFICADOR, CORCHETE_IZQ, LITERAL_ENTERA, CORCHETE_DER, PUNTO_COMA,
      FIN_ARCHIVO
   };

   for (const auto& p : parsea(&tokens[0])) {
      imprime(*p);
   }
}
