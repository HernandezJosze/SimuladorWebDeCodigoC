//
// Created by :)-|-< on 8/25/2021.
//

#ifndef SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_H
#define SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_H

#include "lexer.h"
#include "parser.h"
#include "semantico_ejecutor_aux.h"
#include <iostream>
#include <string>
#include <set>

struct variable_ejecucion {
   virtual ~variable_ejecucion( ) = 0;
};
variable_ejecucion::~variable_ejecucion( ) { };

template<typename T>
struct variable_escalar : variable_ejecucion {
   T valor;

   using type = T;
   variable_escalar( ) = default;
   variable_escalar(T v)
   : valor(v) {
   }
};
struct variable_arreglo : variable_ejecucion {
   std::vector<std::unique_ptr<variable_ejecucion>> valores;

   variable_arreglo( ) = default;
   variable_arreglo(std::vector<std::unique_ptr<variable_ejecucion>>&& v)
   : valores(std::move(v)) {
   }
};

struct tabla_simbolos {
   std::map<std::string_view, std::unique_ptr<variable_ejecucion>> ambito;
   tabla_simbolos* padre;

   tabla_simbolos(tabla_simbolos* p = nullptr)
   : padre(p) {
   }
   bool agrega(const std::string_view& s, std::unique_ptr<variable_ejecucion>&& p) {
      return ambito.emplace(s, std::move(p)).second;
   }
   variable_ejecucion* busca(const std::string_view& s) const {
      if (auto iter = ambito.find(s); iter != ambito.end( )) {
         return iter->second.get( );
      } else if (padre != nullptr) {
         return padre->busca(s);
      } else {
         return nullptr;
      }
   }
};

struct tabla_temporales {
   std::vector<std::unique_ptr<variable_ejecucion>> valores;
   std::set<variable_ejecucion*> busqueda;

   template<typename T, typename... P>
   variable_ejecucion* crea(P&&... p) {
      valores.emplace_back(std::make_unique<T>(std::forward<P>(p)...));
      return *busqueda.insert(valores.back( ).get( )).first;
   }
   bool es_temporal(variable_ejecucion* p) {
      return busqueda.contains(p);
   }
};

// expresiones

variable_ejecucion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt);
variable_ejecucion* evalua(const expresion_terminal& e, tabla_simbolos& ts, tabla_temporales& tt) {
   if (e.tk->tipo == LITERAL_ENTERA) {
      return tt.crea<variable_escalar<int>>(std::stoi(std::string(e.tk->location)));        // hay mejores formas que stoi, pero nos bastará
   } else if (e.tk->tipo == LITERAL_FLOTANTE) {
      return tt.crea<variable_escalar<float>>(std::stof(std::string(e.tk->location)));
   } else if (e.tk->tipo == LITERAL_CADENA) {
      return tt.crea<variable_escalar<std::string_view>>(std::string(e.tk->location));      // técnicamente una cadena no es un escalar, pero nosotros sólo permitiremos cadenas para cosas como scanf o printf
   } else if (e.tk->tipo == IDENTIFICADOR) {
      if (auto p = ts.busca(e.tk->location); p != nullptr) {
         return p;
      } else {
         throw std::pair(*e.tk, "La variable no esta declarada");
      }
   }
}
variable_ejecucion* evalua(const expresion_op_prefijo& e, tabla_simbolos& ts, tabla_temporales& tt) {
   auto sobre = evalua(*e.sobre, ts, tt);
   if (e.operador->tipo == INCREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw std::pair(*e.operador, "No se puede incrementar un temporal");
      }
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         ++checado->valor;
      })) {
         return sobre;    // nos regresamos nosotros mismos porque ++++i se vale
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DECREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw std::pair(*e.operador, "No se puede decrementar un temporal");
      }
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         --checado->valor;
      })) {
         return sobre;    // nos regresamos nosotros mismos porque ----i se vale
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == MAS) {
      if (valida<variable_escalar<int>*, variable_escalar<float>*>(sobre)) {
         return sobre;    // el operador realmente no hace nada, aunque tenemos que verificar que no quieran hacer +"hola"
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == MENOS) {
      variable_ejecucion* res;
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<variable_escalar<typename std::decay_t<decltype(*checado)>::type>>(-checado->valor);    // crear un temporal con el mismo tipo que el operando pero con el signo opuesto (-(5) crea un int, -(3.14) crea un float)
      })) {
         return res;
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == NOT) {
      variable_ejecucion* res;
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<variable_escalar<int>>(!checado->valor);    // crear un temporal entero con el resultado del !
      })) {
         return res;
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DIRECCION) {
      variable_ejecucion* res;
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<variable_escalar<typename std::decay_t<decltype(*checado)>::type*>>(&checado->valor);
      })) {
         return res;
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   }
}
variable_ejecucion* evalua(const expresion_op_posfijo& e, tabla_simbolos& ts, tabla_temporales& tt) {
   auto sobre = evalua(*e.sobre, ts, tt);
   if (tt.es_temporal(sobre)) {
         throw std::pair(*e.operador, "No se puede realizar un decremento/incremento a un temporal");
      }

   if (e.operador->tipo == INCREMENTO) {
      variable_ejecucion* res;
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<variable_escalar<typename std::decay_t<decltype(*checado)>::type>>(checado->valor++);
      })) {
         return sobre;
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }

   } else if (e.operador->tipo == DECREMENTO) {
      variable_ejecucion* res;
      if (valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<variable_escalar<typename std::decay_t<decltype(*checado)>::type>>(checado->valor--);
      })) {
         return sobre;
      } else {
         throw std::pair(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   }
}
variable_ejecucion* evalua(const expresion_op_binario& e, tabla_simbolos& ts, tabla_temporales& tt) {    // algo más difícil que las demás
   //...
}
variable_ejecucion* evalua(const expresion_llamada& e, tabla_simbolos& ts, tabla_temporales& tt) {       // algo más difícil que las demás
   //...
}
variable_ejecucion* evalua(const expresion_corchetes& e, tabla_simbolos& ts, tabla_temporales& tt) {     // algo más difícil que las demás
   //...
}
variable_ejecucion* evalua(const expresion_arreglo& e, tabla_simbolos& ts, tabla_temporales& tt) {       // algo más difícil que las demás

}

variable_ejecucion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt) {
   if (auto checar = dynamic_cast<const expresion_terminal*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   } else if (auto checar = dynamic_cast<const expresion_op_prefijo*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   } else if (auto checar = dynamic_cast<const expresion_op_posfijo*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   } else if (auto checar = dynamic_cast<const expresion_op_binario*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   } else if (auto checar = dynamic_cast<const expresion_llamada*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   } else if (auto checar = dynamic_cast<const expresion_corchetes*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   } else if (auto checar = dynamic_cast<const expresion_arreglo*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt);
   }
}

// sentencias

variable_ejecucion* evalua(const std::vector<std::unique_ptr<expresion>>& ex, tabla_simbolos& ts, tabla_temporales& tt) {
   variable_ejecucion* res = nullptr;
   for (const auto& actual : ex) {
      res = evalua(*actual, ts, tt);
// prueba
valida_ejecuta<variable_escalar<int>*, variable_escalar<float>*>(res, [&](auto checado) {
std::cerr << checado->valor << "\n";
});
// fin prueba
   }
   return res;
}

void evalua(const sentencia& s, tabla_simbolos& ts);
void evalua(const sentencia_expresiones& s, tabla_simbolos& ts) {
   tabla_temporales tt;
   evalua(s.ex, ts, tt);
}
void evalua(const sentencia_declaraciones& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_if& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_for& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_do& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_while& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_break& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_continue& s, tabla_simbolos& ts) {
   //...
}

void evalua(const sentencia& s, tabla_simbolos& ts) {
   if (auto checar = dynamic_cast<const sentencia_expresiones*>(&s); checar != nullptr) {
      return evalua(*checar, ts);
   }else if(auto checar = dynamic_cast<const sentencia_declaraciones*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }else if (auto checar = dynamic_cast<const sentencia_if*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }else if(auto checar = dynamic_cast<const sentencia_for*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }else if(auto checar = dynamic_cast<const sentencia_break*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }else if(auto checar = dynamic_cast<const sentencia_continue*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }else if(auto checar = dynamic_cast<const sentencia_while*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }else if(auto checar = dynamic_cast<const sentencia_do*>(&s); checar != nullptr){
      return evalua(*checar, ts);
   }
}

#endif
