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
#include <string_view>
#include <set>

struct valor_ejecucion {
   virtual ~valor_ejecucion( ) = 0;
};
valor_ejecucion::~valor_ejecucion( ) { };

template<typename T>
struct valor_escalar : valor_ejecucion {
   T valor;

   using type = T;
   valor_escalar( ) = default;
   valor_escalar(T v)
   : valor(v) {
   }
};
struct valor_arreglo : valor_ejecucion {
   std::vector<std::unique_ptr<valor_ejecucion>> valores;

   valor_arreglo( ) = default;
   valor_arreglo(std::vector<std::unique_ptr<valor_ejecucion>>&& v)
   : valores(std::move(v)) {
   }
};

struct tabla_simbolos {
   std::map<std::string_view, std::unique_ptr<valor_ejecucion>> ambito;
   tabla_simbolos* padre;

   tabla_simbolos(tabla_simbolos* p = nullptr)
   : padre(p) {
   }
   bool agrega(const std::string_view& s, std::unique_ptr<valor_ejecucion>&& p) {
      return ambito.emplace(s, std::move(p)).second;
   }
   valor_ejecucion* busca(const std::string_view& s) const {
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
   std::vector<std::unique_ptr<valor_ejecucion>> valores;
   std::set<valor_ejecucion*> busqueda;

   template<typename T, typename... P>
   valor_ejecucion* crea(P&&... p) {
      valores.emplace_back(std::make_unique<T>(std::forward<P>(p)...));
      return *busqueda.insert(valores.back( ).get( )).first;
   }
   bool es_temporal(valor_ejecucion* p) {
      return busqueda.contains(p);
   }
};

// expresiones

valor_ejecucion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt);
valor_ejecucion* evalua(const expresion_terminal& e, tabla_simbolos& ts, tabla_temporales& tt){
   if (e.tk->tipo == LITERAL_ENTERA){
      return tt.crea<valor_escalar<int>>(std::stoi(std::string(e.tk->location)));        // hay mejores formas que stoi, pero nos bastará
   }else if (e.tk->tipo == LITERAL_FLOTANTE) {
      return tt.crea<valor_escalar<float>>(std::stof(std::string(e.tk->location)));
   }else if (e.tk->tipo == LITERAL_CADENA) {
      return tt.crea<valor_escalar<std::string_view>>(std::string(e.tk->location));      // técnicamente una cadena no es un escalar, pero nosotros sólo permitiremos cadenas para cosas como scanf o printf
   }else if (e.tk->tipo == IDENTIFICADOR) {
      if (auto p = ts.busca(e.tk->location); p != nullptr) {
         return p;
      }else{
         throw error(*e.tk, "La variable no esta declarada");
      }
   }
}
valor_ejecucion* evalua(const expresion_op_prefijo& e, tabla_simbolos& ts, tabla_temporales& tt) {
   auto sobre = evalua(*e.sobre, ts, tt);
   if (e.operador->tipo == INCREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede incrementar un temporal");
      }
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         ++checado->valor;
      })) {
         return sobre;    // nos regresamos nosotros mismos porque ++++i se vale
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DECREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede decrementar un temporal");
      }
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         --checado->valor;
      })) {
         return sobre;    // nos regresamos nosotros mismos porque ----i se vale
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == MAS) {
      if (valida<valor_escalar<int>*, valor_escalar<float>*>(sobre)) {
         return sobre;    // el operador realmente no hace nada, aunque tenemos que verificar que no quieran hacer +"hola"
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == MENOS) {
      valor_ejecucion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type>>(-checado->valor);    // crear un temporal con el mismo tipo que el operando pero con el signo opuesto (-(5) crea un int, -(3.14) crea un float)
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == NOT) {
      valor_ejecucion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<int>>(!checado->valor);    // crear un temporal entero con el resultado del !
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DIRECCION) {
      valor_ejecucion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type*>>(&checado->valor);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   }
}
valor_ejecucion* evalua(const expresion_op_posfijo& e, tabla_simbolos& ts, tabla_temporales& tt) {
   auto sobre = evalua(*e.sobre, ts, tt);
   if (e.operador->tipo == INCREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede incrementar un temporal");
      }
      valor_ejecucion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type>>(checado->valor++);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DECREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede decrementar un temporal");
      }
      valor_ejecucion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type>>(checado->valor--);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   }
}
valor_ejecucion* evalua(const expresion_op_binario& e, tabla_simbolos& ts, tabla_temporales& tt) {    // algo más difícil que las demás
   //...
}
valor_ejecucion* evalua(const expresion_llamada& e, tabla_simbolos& ts, tabla_temporales& tt) {       // algo más difícil que las demás
   //...
}
valor_ejecucion* evalua(const expresion_corchetes& e, tabla_simbolos& ts, tabla_temporales& tt) {     // algo más difícil que las demás
   //...
}
valor_ejecucion* evalua(const expresion_arreglo& e, tabla_simbolos& ts, tabla_temporales& tt) {       // algo más difícil que las demás

}

valor_ejecucion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt) {
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

valor_ejecucion* evalua(const std::vector<std::unique_ptr<expresion>>& ex, tabla_simbolos& ts, tabla_temporales& tt) {
   valor_ejecucion* res = nullptr;
   for (const auto& actual : ex) {
      res = evalua(*actual, ts, tt);
// inicio prueba
valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(res, [&](auto checado) {
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
   tabla_temporales tt;
   for(const auto& actual : s.subdeclaraciones) {
      if(actual.tamanios.empty( )){
         std::unique_ptr<valor_ejecucion> valor;         // cada variable debe tener su propio valor_ejecucion que no es temporal, porque la variable debe sobrevivir (y en consecuencia, su valor también)
         if (actual.inicializador == nullptr) {
            valor = std::unique_ptr<valor_ejecucion>(s.tipo->tipo == INT ? std::unique_ptr<valor_ejecucion>(std::make_unique<valor_escalar<int>>( )) : std::make_unique<valor_escalar<float>>( ));    // valor indefinido
         } else if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(*actual.inicializador, ts, tt), [&](auto checado) {
            valor = std::unique_ptr<valor_ejecucion>(s.tipo->tipo == INT ? std::unique_ptr<valor_ejecucion>(std::make_unique<valor_escalar<int>>(checado->valor)) : std::make_unique<valor_escalar<float>>(checado->valor));  //valor que es una copia del del inicializador
         })){
            throw error(*s.tipo, "El inicializador no es compatible con la declaracion");
         }
         if (!ts.agrega(actual.nombre->location, std::move(valor))) {
            throw error(*s.tipo, "La variable ya ha sido declarada");
         }
      }else{

      }
   }
}
void evalua(const sentencia_if& s, tabla_simbolos& ts) {
   tabla_temporales tt;
   if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt), [&](auto checado) {
      if (checado->valor) {
         for(const auto& si : s.parte_si){
            evalua(*si, ts);
         }
      } else {
         for(const auto& no : s.parte_no){
            evalua(*no, ts);
         }
      }
   })){
      throw error(*s.pos, "Tipo inválido en condición");
   }
}
void evalua(const sentencia_for& s, tabla_simbolos& ts) {
   //...
}
void evalua(const sentencia_do& s, tabla_simbolos& ts) {
   for(;;){
      for(const auto& sentencia : s.sentencias){
         evalua(*sentencia, ts);
      }
      tabla_temporales tt; bool b;              // la tabla de temporales debería morir en cada iteración
      if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt), [&](auto checado){
         b = checado->valor;
      })){
         throw error(*s.pos, "Tipo invalido de condicion");
      }
      if(!b){
         break;
      }
   }
}
void evalua(const sentencia_while& s, tabla_simbolos& ts) {
   for(;;){
      tabla_temporales tt; bool b;              // la tabla de temporales debería morir en cada iteración
      if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt), [&](auto checado){
         b = checado->valor;
      })){
         throw error(*s.pos, "Tipo invalido de condicion");
      }
      if(!b){
         break;
      }
      for(const auto& sentencia : s.sentencias){
         evalua(*sentencia, ts);
      }
   }
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
