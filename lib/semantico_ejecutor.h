//
// Created by :)-|-< on 8/25/2021.
//

#ifndef SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_H
#define SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_H

#include "lexer.h"
#include "parser.h"
#include "semantico_ejecutor_aux.h"
#include <cstdio>
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
template<int(*T)(const char*, ...)>
struct valor_funcion : valor_expresion {
   valor_funcion( ) = default;
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
      return tt.crea<valor_escalar<int>>(std::stoi(std::string(e.tk->location)));
   }else if (e.tk->tipo == LITERAL_FLOTANTE) {
      return tt.crea<valor_escalar<float>>(std::stof(std::string(e.tk->location)));
   }else if (e.tk->tipo == LITERAL_CADENA) {
      return tt.crea<valor_escalar<std::string_view>>(e.tk->location);
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
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&]<typename T>(valor_escalar<T>* checado) {
         res = tt.crea<valor_escalar<T>>(-checado->valor);    // crear un temporal con el mismo tipo que el operando pero con el signo opuesto (-(5) crea un int, -(3.14) crea un float)
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
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&]<typename T>(valor_escalar<T>* checado) {
         res = tt.crea<valor_escalar<T*>>(&checado->valor);
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
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&]<typename T>(valor_escalar<T>* checado) {
         res = tt.crea<valor_escalar<T>>(checado->valor++);
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
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&]<typename T>(valor_escalar<T>* checado) {
         res = tt.crea<valor_escalar<T>>(checado->valor--);
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
   auto f = evalua(*e.func, ts, tt);
   if(!valida<valor_funcion<scanf>*, valor_funcion<printf>*>(f)){
      throw error(*e.pos, "Solo se puede llamar a las funcones scanf y printf");
   }

   std::vector<valor_expresion*> params;
   for(const auto& p : e.parametros){
      params.push_back(evalua(*p, ts, tt));
   }

   if(params.empty( )){
      throw error(*e.func->pos, "No hay ningun parametro en la funcion");
   }

   auto cad = dynamic_cast<valor_escalar<std::string_view>*>(params[0]);
   if(cad == nullptr){
      throw error(*e.func->pos, "El primer parametro debe de ser una cadena");
   }

   auto Scanf = dynamic_cast<const valor_funcion<scanf>*>(f);
   auto Printf= dynamic_cast<const valor_funcion<printf>*>(f);
   int actual = 1; int res = 0; std::string s = "";
   std::map<char, char> scapeSecuense = {
      {'n', '\n'},
      {'t', '\t'},
      {'a', '\a'},
      {'f', '\f'},
      {'r', '\r'},
      {'?', '\?'},
      {'b', '\b'},
      {'\"', '\"'},
      {'\'', '\''},
      {'%', '%'}
   };
   for(int i = 1; i < cad->valor.size( ) - 1; ++i){
      if(cad->valor[i] != '%' || cad->valor[i + 1] == '%'){ // si no es != '%' es igual y solo checamos si el siguiente es tambien '%'
         if(cad->valor[i] == '\\' || cad->valor[i] == '%'){
            s.push_back(scapeSecuense[cad->valor[++i]]);
         }else{
            s.push_back(cad->valor[i]);
         }
      }else if(cad->valor[i] == '%' && (cad->valor[i + 1] == 'd' || cad->valor[i + 1] == 'f') && actual < params.size( )){ //ya sabemos que i == '%' AND innecesario
         if(Printf){
            res += std::printf("%s", s.c_str( ));
            if(auto checar = dynamic_cast<valor_escalar<int>*>(params[actual]); checar != nullptr && cad->valor[i + 1] == 'd'){
               res += std::printf("%d", checar->valor);
            }else if(auto checar = dynamic_cast<valor_escalar<float>*>(params[actual]); checar != nullptr && cad->valor[i + 1] == 'f'){
               res += std::printf("%f", checar->valor);
            }else{
               throw error(*e.pos, "No concuerda el tipo a imprimir");
            }
         }else if(Scanf){
            if(auto checar = dynamic_cast<valor_escalar<int*>*>(params[actual]); checar != nullptr && cad->valor[i + 1] == 'd'){
               res += std::scanf(std::string(s + "%d").c_str( ), &*checar->valor);
            }else if(auto checar = dynamic_cast<valor_escalar<float*>*>(params[actual]); checar != nullptr && cad->valor[i + 1] == 'f'){
               res += std::scanf(std::string(s + "%f").c_str( ), &*checar->valor);
            }else{
               throw error(*e.pos, "No concuerda el tipo a leer");
            }
         }
         ++i, ++actual; s.clear( );
      }else{
         if(actual == params.size( )){
            throw error(*e.pos, "No existen parametros suficientes");
         }else{
            throw error(*e.pos, "Solo es valido %d y %f");
         }
      }
   }

   if(Printf){
      res += std::printf("%s", s.c_str( ));
   }
   return tt.crea<valor_escalar<int>>(res);// cambiar por # de cosas
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
   ts.agrega("scanf", std::make_unique<valor_funcion<scanf>>( ));
   ts.agrega("printf", std::make_unique<valor_funcion<printf>>( ));
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
      if(!actual.arreglo.second) {
         if (actual.inicializador == nullptr) {
            valor = (s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>( ) : std::make_unique<valor_escalar<float>>( ));    // valor indefinido
         } else if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(*actual.inicializador, ts, tt), [&](auto checado) {
            valor = (s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>(checado->valor) : std::make_unique<valor_escalar<float>>(checado->valor));  //valor que es una copia del del inicializador
         })){
            throw error(*actual.inicializador->pos, "El inicializador no es compatible con la declaracion");
         }
      }else{
         if (actual.inicializador == nullptr && actual.arreglo.first == nullptr) {
            throw error(*actual.nombre, "No se puede deducir el tamanio del arreglo.");
         }

         std::vector<std::unique_ptr<valor_expresion>> elementos;
         if (actual.inicializador != nullptr) {
            if(auto checar = dynamic_cast<valor_arreglo<valor_expresion*>*>(evalua(*actual.inicializador, ts, tt)); checar != nullptr){
               for(auto& el : checar->valores){
                  if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(el, [&](auto checado) {
                     elementos.push_back((s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>(checado->valor) : std::make_unique<valor_escalar<float>>(checado->valor)));  //valor que es una copia del del inicializador
                  })){
                     throw error(*actual.inicializador->pos, "El tipo del elemento no es válido al inicializar el arreglo");
                  }
               }
            } else {
               throw error(*actual.inicializador->pos, "El inicializador no es compatible con la declaracion");
            }
         }
         if (actual.arreglo.first != nullptr) {
            if (auto tam = dynamic_cast<valor_escalar<int>*>(evalua(*actual.arreglo.first, ts, tt)); tam != nullptr){
               if(elementos.size( ) > tam->valor){
                  throw error(*actual.inicializador->pos, "El el inicializador tiene elementos en exceso");
               }
               while(elementos.size( ) < tam->valor) {
                  elementos.push_back(s.tipo->tipo == INT ? std::unique_ptr<valor_expresion>(std::make_unique<valor_escalar<int>>( )) : std::make_unique<valor_escalar<float>>( ));
               }
            } else {
               throw error(*actual.arreglo.first->pos, "El tamanio debe de ser un valor entero");
            }
         }
         valor = std::make_unique<valor_arreglo<std::unique_ptr<valor_expresion>>>(std::move(elementos));
      }
      if(!ts.agrega(actual.nombre->location, std::move(valor))){
         throw error(*actual.nombre, "La variable ya ha sido declarada");
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
      tabla_temporales tt; bool b = true;              // la tabla de temporales debería morir en cada iteración
      if(!s.condicion.empty( )){
         if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts_incializador, tt), [&](auto checado){
            b = checado->valor;
         })){
            throw error(*s.pos, "Tipo invalido de condicion");
         }
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
      if(!s.condicion.empty( ) && !valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt), [&](auto checado){
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
      if(!s.condicion.empty( ) && !valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt), [&](auto checado){
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
