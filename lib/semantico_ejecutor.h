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

struct valor_expresion {
   virtual ~valor_expresion( ) = 0;
};
valor_expresion::~valor_expresion( ) { };

template<typename T>
struct valor_escalar : valor_expresion {
   T valor;

   using type = T;
   valor_escalar( ) = default;
   valor_escalar(T v)
   : valor(v) {
   }
};
template<typename T>
struct valor_arreglo : valor_expresion {
   std::vector<T> valores;

   valor_arreglo( ) = default;
   valor_arreglo(std::vector<T>&& v)
   : valores(std::move(v)) {
   }
};

struct tabla_simbolos {
   std::map<std::string_view, std::unique_ptr<valor_expresion>> ambito;
   tabla_simbolos* padre;

   tabla_simbolos(tabla_simbolos* p = nullptr)
   : padre(p) {
   }
   bool agrega(const std::string_view& s, std::unique_ptr<valor_expresion>&& p) {
      return ambito.emplace(s, std::move(p)).second;
   }
   valor_expresion* busca(const std::string_view& s) const {
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
   std::vector<std::unique_ptr<valor_expresion>> valores;
   std::set<valor_expresion*> busqueda;

   template<typename T, typename... P>
   valor_expresion* crea(P&&... p) {
      valores.emplace_back(std::make_unique<T>(std::forward<P>(p)...));
      return *busqueda.insert(valores.back( ).get( )).first;
   }
   bool es_temporal(valor_expresion* p) {
      return busqueda.contains(p);
   }
};

// expresiones

valor_expresion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt);
valor_expresion* evalua(const expresion_terminal& e, tabla_simbolos& ts, tabla_temporales& tt){
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
valor_expresion* evalua(const expresion_op_prefijo& e, tabla_simbolos& ts, tabla_temporales& tt) {
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
      valor_expresion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type>>(-checado->valor);    // crear un temporal con el mismo tipo que el operando pero con el signo opuesto (-(5) crea un int, -(3.14) crea un float)
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == NOT) {
      valor_expresion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<int>>(!checado->valor);    // crear un temporal entero con el resultado del !
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DIRECCION) {
      valor_expresion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type*>>(&checado->valor);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   }
}
valor_expresion* evalua(const expresion_op_posfijo& e, tabla_simbolos& ts, tabla_temporales& tt) {
   auto sobre = evalua(*e.sobre, ts, tt);
   if (e.operador->tipo == INCREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede incrementar un temporal");
      }
      valor_expresion* res;
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
      valor_expresion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         res = tt.crea<valor_escalar<typename std::decay_t<decltype(*checado)>::type>>(checado->valor--);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   }
}
valor_expresion* evalua(const expresion_op_binario& e, tabla_simbolos& ts, tabla_temporales& tt) {    // algo más difícil que las demás
   //...
}
valor_expresion* evalua(const expresion_llamada& e, tabla_simbolos& ts, tabla_temporales& tt) {
   //...
}
valor_expresion* evalua(const expresion_corchetes& e, tabla_simbolos& ts, tabla_temporales& tt) {
   if (auto arr = dynamic_cast<valor_arreglo<std::unique_ptr<valor_expresion>>*>(evalua(*e.ex, ts, tt)); arr != nullptr) {
      if (auto indice = dynamic_cast<valor_escalar<int>*>(evalua(*e.dentro, ts, tt)); indice != nullptr) {
         if (indice->valor >= 0 && indice->valor < arr->valores.size( )){
            return arr->valores[indice->valor].get( );
         } else {
            throw error(*e.pos, "El valor esta fuera de rango");
         }
      } else {
         throw error(*e.pos, "El indice dado se sale del arreglo");
      }
   } else {
      throw error(*e.pos, "Solo se puede aplicar este operador a un arreglo");
   }
}
valor_expresion* evalua(const expresion_arreglo& e, tabla_simbolos& ts, tabla_temporales& tt) {
   std::vector<valor_expresion*> v;
   for (const auto& actual : e.elementos) {
      v.push_back(evalua(*actual, ts, tt));
   }
   return tt.crea<valor_arreglo<valor_expresion*>>(std::move(v));
}

valor_expresion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt) {
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

valor_expresion* evalua(const std::vector<std::unique_ptr<expresion>>& ex, tabla_simbolos& ts, tabla_temporales& tt) {
   valor_expresion* res = nullptr;
   for (const auto& actual : ex) {
      res = evalua(*actual, ts, tt);
// inicio prueba
valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(res, [&](auto checado) {
   std::cerr << checado->valor << "\n";
});
valida_ejecuta<valor_arreglo<std::unique_ptr<valor_expresion>>*>(res, [&](auto checado) {       // realmente debería ser un algoritmo recursivo para manejar matrices; funcionará por ahora
   std::cerr << "{";
   for (const auto& actual : checado->valores) {
      valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(actual.get( ), [&](auto checado) {
         std::cerr << checado->valor << ",";
      });
   }
   std::cerr << "}\n";
});
// fin prueba
   }
   return res;
}

void evalua(const sentencia& s, tabla_simbolos& ts);
void evalua(const std::vector<std::unique_ptr<sentencia>>& sentencias){
   tabla_simbolos ts;
   try{
      for (const auto& sentencia : sentencias) {
         evalua(*sentencia, ts);
      }
   }catch(const sentencia_break &e){
      throw error(*e.pos, "No se permite hacer un break fuera de un ciclo");
   }catch(const sentencia_continue & c){
      throw error(*c.pos, "No se permite hacer un continue fuera de un ciclo");
   }
}
void evalua(const sentencia_expresiones& s, tabla_simbolos& ts) {
   tabla_temporales tt;
   evalua(s.ex, ts, tt);
}
void evalua(const sentencia_declaraciones& s, tabla_simbolos& ts) {
   tabla_temporales tt;
   for(const auto& actual : s.subdeclaraciones) {
      std::unique_ptr<valor_expresion> valor;     // cada variable debe tener su propio valor_expresion que no es temporal, porque la variable debe sobrevivir (y en consecuencia, su valor también)
      if(actual.tamanios.empty( )){
         if (actual.inicializador == nullptr) {
            valor = (s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>( ) : std::make_unique<valor_escalar<float>>( ));    // valor indefinido
         } else if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(*actual.inicializador, ts, tt), [&](auto checado) {
            valor = (s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>(checado->valor) : std::make_unique<valor_escalar<float>>(checado->valor));  //valor que es una copia del del inicializador
         })){
            throw error(*s.tipo, "El inicializador no es compatible con la declaracion");
         }
      }else{
         if (actual.inicializador == nullptr) {
            if (actual.tamanios[0] == nullptr) {
               throw error(*s.pos, "No se puede deducir el tamaño del arreglo.");
            }
            if (auto tam = dynamic_cast<valor_escalar<int>*>(evalua(*actual.tamanios[0], ts, tt)); tam != nullptr){
               std::vector<std::unique_ptr<valor_expresion>> elementos;
               for (int i = 0; i < tam->valor; ++i) {
                  elementos.push_back((s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>( ) : std::make_unique<valor_escalar<float>>( )));
               }
               valor = std::make_unique<valor_arreglo<std::unique_ptr<valor_expresion>>>(std::move(elementos));
            } else {
               throw error(*s.pos, "El tamanio debe de ser un valor entero");
            }
         } else {
            /*if(actual.inicializador != nullptr){ // si no es nulo creamos el arreglo con los elementos del inicializador
               if(!valida_ejecuta<valor_arreglo<int>*, valor_arreglo<float>*>(evalua(*actual.inicializador, ts, tt), [&](auto checado) {
                  if(s.tipo->tipo == INT){
                     arreglo = std::unique_ptr<valor_expresion>(std::make_unique<valor_arreglo<int>>(std::move(checado->valores)));//valor que es una copia del del inicializador
                  }else if(s.tipo->tipo == FLOAT){
                     arreglo = std::unique_ptr<valor_expresion>(std::make_unique<valor_arreglo<float>>(std::move(checado->valores)));//valor que es una copia del del inicializador
                  }
               })){
                  throw error(*s.tipo, "El inicializador no es compatible con la declaracion");
               }
            }*/
   /*
            if(actual.tamanios[0] != nullptr){
               if(auto tam = dynamic_cast<valor_escalar<int>*>(evalua(*actual.tamanios[0], ts, tt)); tam != nullptr){
                  if(tam->valor >= arreglo->valores.size( )){
                     auto val = std::unique_ptr<valor_expresion>(s.tipo->tipo == INT ? std::unique_ptr<valor_expresion>(std::make_unique<valor_escalar<int>>( )) : std::make_unique<valor_escalar<float>>( ));
                     arreglo->valores.resize(tam->valor, std::move(val));    // valor indefinido
                  }else{
                     throw error(*s.tipo, "El tamanio del arreglo es menor que el inicializador");
                  }
               }else{
                  throw error(*s.pos, "El tamanio debe de ser un valor entero");
               }
            }else{

            }
   */
         }
      }

      if(!ts.agrega(actual.nombre->location, std::move(valor))){
         throw error(*s.tipo, "La variable ya ha sido declarada");
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
   tabla_simbolos ts_incializador(&ts);
   evalua(*s.inicializacion, ts_incializador);

   for(;;){
      tabla_temporales tt; bool b;              // la tabla de temporales debería morir en cada iteración
      if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts_incializador, tt), [&](auto checado){
         b = checado->valor;
      })){
         throw error(*s.pos, "Tipo invalido de condicion");
      }
      if(!b){
         break;
      }
      try{
         for(const auto& sentencia : s.sentencias){
            evalua(*sentencia, ts_incializador);
         }
      }catch(const sentencia_break &e){
         break;
      }catch(const sentencia_continue & c){

      }
      evalua(s.actualizacion, ts_incializador, tt);
   }
}
void evalua(const sentencia_do& s, tabla_simbolos& ts) {
   for(;;){
      try{
         for(const auto& sentencia : s.sentencias){
            evalua(*sentencia, ts);
         }
      }catch(const sentencia_break &e){
         break;
      }catch(const sentencia_continue & c){

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
      try{
         for(const auto& sentencia : s.sentencias){
            evalua(*sentencia, ts);
         }
      }catch(const sentencia_break &e){
         break;
      }catch(const sentencia_continue &c){

      }
   }
}
void evalua(const sentencia_break& s, tabla_simbolos& ts) {
   throw s;
}
void evalua(const sentencia_continue& s, tabla_simbolos& ts) {
   throw s;
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
