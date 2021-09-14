//
// Created by :)-|-< on 8/25/2021.
//

#ifndef SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_H
#define SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_H

#include "lexer.h"
#include "parser.h"
#include "semantico_ejecutor_aux.h"
#include <array>
#include <cstdio>
#include <iostream>
#include <string>
#include <string_view>
#include <set>
#include <type_traits>

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
   std::vector<T> valor;

   valor_arreglo( ) = default;
   valor_arreglo(std::vector<T>&& v)
   : valor(std::move(v)) {
   }
};
template<int(*T)(const char*, ...)>
struct valor_funcion : valor_expresion {
   static constexpr auto funcion = T;
};

struct tabla_simbolos {
   std::map<std::string_view, std::unique_ptr<valor_expresion>> ambito;
   std::map<valor_expresion*, std::string> inverso;
   tabla_simbolos* padre;

   tabla_simbolos(tabla_simbolos* p = nullptr)
   : padre(p) {
   }
   valor_expresion* agrega(const std::string_view& s, std::unique_ptr<valor_expresion>&& p) {
      if (auto [iter, ins] = ambito.emplace(s, std::move(p)); ins) {
         if (auto checar = dynamic_cast<valor_arreglo<std::unique_ptr<valor_expresion>>*>(iter->second.get( )); checar != nullptr) {
            for (int i = 0; i < checar->valor.size( ); ++i) {
               inverso.emplace(checar->valor[i].get( ), std::string(s) + "[" + std::to_string(i) + "]");
            }
         }
         return inverso.emplace(iter->second.get( ), s).first->first;
      } else {
         return nullptr;
      }
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
   std::string busca(valor_expresion* p) const {
      if (auto iter = inverso.find(p); iter != inverso.end( )) {
         return iter->second;
      } else if (padre != nullptr) {
         return padre->busca(p);
      } else {
         return "";
      }
   }
};
struct tabla_temporales {
   std::vector<std::unique_ptr<valor_expresion>> valores;
   std::set<valor_expresion*> busqueda;

   template<typename T, typename... P>
   T* crea(P&&... p) {
      valores.emplace_back(std::make_unique<T>(std::forward<P>(p)...));
      return dynamic_cast<T*>(*busqueda.insert(valores.back( ).get( )).first);
   }
   bool es_temporal(valor_expresion* p) {
      return busqueda.contains(p);
   }
};

// procesamiento de la entrada e impresión de instrucciones

void esquiva_lectura(std::istream& is, char c) {
   if (isspace(c)) {
      while (isspace(is.peek( ))) {
         is.ignore( );
      }
   } else if (is.peek( ) == c) {
      is.ignore( );
   }
}
void emite_imprime(std::ostream& os, const std::string& s) {
   os << "IMPRIME\01" << s << "\04\n";
}
void emite_crea(std::ostream& os, valor_expresion* v, const tabla_simbolos& ts) {
   valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*, valor_arreglo<std::unique_ptr<valor_expresion>>*>(v, [&](auto checado) {
      if constexpr(std::is_arithmetic_v<decltype(checado->valor)>) {
         os << "CREA\01VAR\01" << ts.busca(checado) << "\04\n";
      } else {
         os << "CREA\01ARR\01" << ts.busca(checado) << "\01" << checado->valor.size( ) << "\04\n";
      }
   });
}
void emite_crea_escribe(std::ostream& os, valor_expresion* v, const tabla_simbolos& ts) {
   valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*, valor_arreglo<std::unique_ptr<valor_expresion>>*>(v, [&](auto checado) {
      if constexpr(std::is_scalar_v<decltype(checado->valor)>) {
         os << "CREA\01VAR\01" << ts.busca(checado) << "\01" << checado->valor << "\04\n";
      } else {
         os << "CREA\01ARR\01" << ts.busca(checado) << "\01" << checado->valor.size( ) << "\01";
         for (int i = 0; i < checado->valor.size( ); ++i) {
            valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(checado->valor[i].get( ), [&](auto elem) {
               os << elem->valor << (i + 1 < checado->valor.size( ) ? "," : "\04\n");
            });
         }
      }
   });
}
void emite_escribe(std::ostream& os, valor_expresion* v, const tabla_simbolos& ts) {
   valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*, valor_arreglo<std::unique_ptr<valor_expresion>>*>(v, [&](auto checado) {
      if constexpr(std::is_scalar_v<decltype(checado->valor)>) {
         os << "ESCRIBE\01" << ts.busca(checado) << "\01" << checado->valor << "\04\n";
      } else {
         for (const auto& p : checado->valor) {
            emite_escribe(os, p.get( ), ts);
         }
      }
   });
}
void emite_destruye(std::ostream& os, const tabla_simbolos& ts) {
   if (!ts.ambito.empty( )) {
      os << "DESTRUYE\01" << ts.ambito.size( ) << "\04\n";
   }
}

// expresiones

template<typename T>
valor_escalar<T>* castea(valor_expresion* v, tabla_temporales& tt, const token_anotada& pos) {
   valor_escalar<T>* res = nullptr;
   valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(v, [&]<typename U>(valor_escalar<U>* checado) {
      if constexpr(std::is_same_v<T, U>) {
         res = checado;
      } else {
         res = tt.crea<valor_escalar<T>>(checado->valor);
      }
   });
   if(res == nullptr){
      throw error(pos, "Solo se permiten manipular valores int y float en un cast");
   }
   return res;
}

valor_expresion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios);
valor_expresion* evalua(const expresion_terminal& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
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
   }else {
      return nullptr;
   }
}
valor_expresion* evalua(const expresion_op_prefijo& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   auto sobre = evalua(*e.sobre, ts, tt, ios);
   if (e.operador->tipo == INCREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede incrementar un temporal");
      }
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         checado->valor += 1;
         emite_escribe(ios.second, checado, ts);
      })) {
         return sobre;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == DECREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede decrementar un temporal");
      }
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&](auto checado) {
         checado->valor -= 1;
         emite_escribe(ios.second, checado, ts);
      })) {
         return sobre;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == MAS) {
      if (valida<valor_escalar<int>*, valor_escalar<float>*>(sobre)) {
         return sobre;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else if (e.operador->tipo == MENOS) {
      valor_expresion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&]<typename T>(valor_escalar<T>* checado) {
         res = tt.crea<valor_escalar<T>>(-checado->valor);
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
         res = tt.crea<valor_escalar<valor_escalar<T>*>>(checado);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else {
      return nullptr;
   }
}
valor_expresion* evalua(const expresion_op_posfijo& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   auto sobre = evalua(*e.sobre, ts, tt, ios);
   if (e.operador->tipo == INCREMENTO) {
      if (tt.es_temporal(sobre)) {
         throw error(*e.operador, "No se puede incrementar un temporal");
      }
      valor_expresion* res;
      if (valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(sobre, [&]<typename T>(valor_escalar<T>* checado) {
         res = tt.crea<valor_escalar<T>>(checado->valor++);
         emite_escribe(ios.second, checado, ts);
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
         emite_escribe(ios.second, checado, ts);
      })) {
         return res;
      } else {
         throw error(*e.operador, "Solo se puede aplicar el operador a enteros o flotantes");
      }
   } else {
      return nullptr;
   }
}

valor_expresion* evalua(const expresion_op_binario& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   if (e.operador->tipo == AND) {
      auto checado_izq = castea<float>(evalua(*e.izq, ts, tt, ios), tt, *e.pos);
      return tt.crea<valor_escalar<int>>(checado_izq->valor ? bool(castea<float>(evalua(*e.der, ts, tt, ios), tt, *e.pos)->valor) : false);
   } else if (e.operador->tipo == OR) {
      auto checado_izq = castea<float>(evalua(*e.izq, ts, tt, ios), tt, *e.pos);
      return tt.crea<valor_escalar<int>>(checado_izq->valor ? true : bool(castea<float>(evalua(*e.der, ts, tt, ios), tt, *e.pos)->valor));
   }

   valor_expresion* res = nullptr;
   valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(*e.izq, ts, tt, ios), [&]<typename TI>(valor_escalar<TI>* checado_izq) {
      valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(*e.der, ts, tt, ios),[&]<typename TD>(valor_escalar<TD>* checado_der) {
         if (es_operador_asignacion(e.operador->tipo)) {
            if (tt.es_temporal(checado_izq)) {
               throw error(*e.operador, "No se puede realizar una asignacion un temporal");
            }
            if (e.operador->tipo == ASIGNACION) {
               checado_izq->valor = checado_der->valor;
            }else if (e.operador->tipo == MAS_IGUAL) {
               checado_izq->valor += checado_der->valor;
            }else if (e.operador->tipo == MENOS_IGUAL) {
               checado_izq->valor -= checado_der->valor;
            }else if (e.operador->tipo == MULTIPLICA_IGUAL) {
               checado_izq->valor *= checado_der->valor;
            }else if (e.operador->tipo == DIVIDE_IGUAL) {
               checado_izq->valor /= checado_der->valor;
            }else if (e.operador->tipo == MODULO_IGUAL) {
               if constexpr(!std::is_same_v<TI, int> || !std::is_same_v<TI, TD>){
                  throw error(*e.pos, "Solo se puede aplicar este operador a enteros");
               } else {
                  checado_izq->valor %= checado_der->valor;
               }
            }
            res = checado_izq;
            emite_escribe(ios.second, checado_izq, ts);
         } else if (e.operador->tipo == IGUAL) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor == checado_der->valor);
         }else if (e.operador->tipo == DIFERENTE) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor != checado_der->valor);
         }else if (e.operador->tipo == MENOR) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor < checado_der->valor);
         }else if (e.operador->tipo == MENOR_IGUAL) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor <= checado_der->valor);
         }else if (e.operador->tipo == MAYOR) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor > checado_der->valor);
         }else if (e.operador->tipo == MAYOR_IGUAL) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor >= checado_der->valor);
         }else if (e.operador->tipo == MAS) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor + checado_der->valor);
         }else if (e.operador->tipo == MENOS) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor - checado_der->valor);
         }else if (e.operador->tipo == MULTIPLICACION) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor * checado_der->valor);
         }else if (e.operador->tipo == DIVISION) {
            res = tt.crea<valor_escalar<int>>(checado_izq->valor / checado_der->valor);
         }else if (e.operador->tipo == MODULO) {
            if constexpr(!std::is_same_v<TI, int> || !std::is_same_v<TI, TD>){
               throw error(*e.pos, "Solo se puede aplicar este operador a enteros");
            } else {
               res = tt.crea<valor_escalar<int>>(checado_izq->valor % checado_der->valor);
            }
         }
      });
   });

   if (res == nullptr){
      throw error(*e.pos, "Solo se puede aplicar operadores a int o float");
   }
   return res;
}

valor_expresion* evalua(const expresion_llamada& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   int(*funcion)(const char*, ...);
   if(!valida_ejecuta<valor_funcion<printf>*, valor_funcion<scanf>*>(evalua(*e.func, ts, tt, ios), [&](auto checado) {
      funcion = checado->funcion;
   })) {
      throw error(*e.pos, "Solo se puede llamar a una funcion que ademas sea printf o scanf");
   }

   std::vector<valor_expresion*> params;
   for(const auto& p : e.parametros){
      params.push_back(evalua(*p, ts, tt, ios));
   }
   if(params.empty( )){
      throw error(*e.func->pos, "No hay ningun parametro en la funcion");
   }

   auto cad = dynamic_cast<valor_escalar<std::string_view>*>(params[0]);
   if(cad == nullptr){
      throw error(*e.func->pos, "El primer parametro debe de ser una cadena");
   }
   static const std::map<char, char> escapeSequence = {
      {'n', '\n'}, {'t', '\t'}, {'a', '\a'}, {'f', '\f'}, {'r', '\r'}, {'?', '\?'}, {'b', '\b'}, {'\"', '\"'}, {'\'', '\''}
   };

   int res = 0; std::ostringstream bufer;
   for(int i = 1, actual = 1; i < cad->valor.size( ) - 1; ++i){
      if (cad->valor[i] == '%') {
         if (cad->valor[++i] != '%') {
            if (actual >= params.size( )) {
               throw error(*e.func->pos, "No hay suficientes parametros");
            }
            if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*, valor_escalar<valor_escalar<int>*>*, valor_escalar<valor_escalar<float>*>*>(params[actual++], [&]<typename T>(valor_escalar<T>* checado) {
               if (cad->valor[i] == 'd' && (funcion == printf && std::is_same_v<T, int> || funcion == scanf && std::is_same_v<T, valor_escalar<int>*>) ||
                   cad->valor[i] == 'f' && (funcion == printf && std::is_same_v<T, float> || funcion == scanf && std::is_same_v<T, valor_escalar<float>*>)) {
                  if constexpr(std::is_arithmetic_v<T>) {
                     bufer << checado->valor, res += 1;
                  } else if (ios.first >> checado->valor->valor) {
                     emite_escribe(ios.second, checado->valor, ts), res += 1;
                  } else {
                     throw error(*e.func->pos, "Error durante la lectura");
                  }
               } else {
                  throw error(*e.func->pos, "El especificador no coincide con el tipo");
               }
            })) {
               throw error(*e.func->pos, "Parametro no soportado");
            }
         } else {
            (funcion == scanf ? esquiva_lectura(ios.first, '%') : (void)(bufer << "%"));
         }
      } else if (cad->valor[i] == '\\') {
         if (auto iter = escapeSequence.find(cad->valor[++i]); iter != escapeSequence.end( )) {
            (funcion == scanf ? esquiva_lectura(ios.first, iter->second) : (void)(bufer << iter->second));
         } else {
            throw error(*e.func->pos, "Secuencia de escape desconocida");
         }
      } else {
         (funcion == scanf ? esquiva_lectura(ios.first, cad->valor[i]) : (void)(bufer << cad->valor[i]));
      }
   }

   std::string imprimir = std::move(bufer).str( );
   if (!imprimir.empty( )) {
      emite_imprime(ios.second, imprimir);
   }
   return tt.crea<valor_escalar<int>>(res);
}
valor_expresion* evalua(const expresion_corchetes& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   if (auto arr = dynamic_cast<valor_arreglo<std::unique_ptr<valor_expresion>>*>(evalua(*e.ex, ts, tt, ios)); arr != nullptr) {
      if (auto indice = dynamic_cast<valor_escalar<int>*>(evalua(*e.dentro, ts, tt, ios)); indice != nullptr) {
         if (indice->valor >= 0 && indice->valor < arr->valor.size( )){
            return arr->valor[indice->valor].get( );
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
valor_expresion* evalua(const expresion_arreglo& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   std::vector<valor_expresion*> v;
   for (const auto& actual : e.elementos) {
      v.push_back(evalua(*actual, ts, tt, ios));
   }
   return tt.crea<valor_arreglo<valor_expresion*>>(std::move(v));
}

valor_expresion* evalua(const expresion& e, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   if (auto checar = dynamic_cast<const expresion_terminal*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else if (auto checar = dynamic_cast<const expresion_op_prefijo*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else if (auto checar = dynamic_cast<const expresion_op_posfijo*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else if (auto checar = dynamic_cast<const expresion_op_binario*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else if (auto checar = dynamic_cast<const expresion_llamada*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else if (auto checar = dynamic_cast<const expresion_corchetes*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else if (auto checar = dynamic_cast<const expresion_arreglo*>(&e); checar != nullptr) {
      return evalua(*checar, ts, tt, ios);
   } else {
      return nullptr;
   }
}

// sentencias

valor_expresion* evalua(const std::vector<std::unique_ptr<expresion>>& ex, tabla_simbolos& ts, tabla_temporales& tt, std::pair<std::istream&, std::ostream&> ios) {
   valor_expresion* res = nullptr;
   for (const auto& actual : ex) {
      res = evalua(*actual, ts, tt, ios);
   }
   return res;
}

void evalua(const sentencia& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios);
void evalua(const sentencia_expresiones& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   tabla_temporales tt;
   evalua(s.ex, ts, tt, ios);
}
void evalua(const sentencia_declaraciones& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   tabla_temporales tt;
   for(const auto& actual : s.subdeclaraciones) {
      std::unique_ptr<valor_expresion> valor;
      if(!actual.arreglo.second) {
         if (actual.inicializador == nullptr) {
            valor = (s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>( ) : std::make_unique<valor_escalar<float>>( ));
         } else if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(*actual.inicializador, ts, tt, ios), [&](auto checado) {
            valor = (s.tipo->tipo == INT ? (std::unique_ptr<valor_expresion>)std::make_unique<valor_escalar<int>>(checado->valor) : std::make_unique<valor_escalar<float>>(checado->valor));
         })){
            throw error(*actual.inicializador->pos, "El inicializador no es compatible con la declaracion");
         }
      }else{
         if (actual.inicializador == nullptr && actual.arreglo.first == nullptr) {
            throw error(*actual.nombre, "No se puede deducir el tamanio del arreglo.");
         }

         std::vector<std::unique_ptr<valor_expresion>> elementos;
         if (actual.inicializador != nullptr) {
            if(auto checar = dynamic_cast<valor_arreglo<valor_expresion*>*>(evalua(*actual.inicializador, ts, tt, ios)); checar != nullptr){
               for(auto& elemento : checar->valor){
                  if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(elemento, [&](auto checado) {
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
            if (auto tam = dynamic_cast<valor_escalar<int>*>(evalua(*actual.arreglo.first, ts, tt, ios)); tam != nullptr){
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

      if(auto v = ts.agrega(actual.nombre->location, std::move(valor)); v == nullptr) {
         throw error(*actual.nombre, "La variable ya ha sido declarada");
      } else {
         if (actual.inicializador == nullptr) {
            emite_crea(ios.second, v, ts);
         } else {
            emite_crea_escribe(ios.second, v, ts);
         }
      }
   }
}
void evalua(const sentencia_if& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   tabla_temporales tt;
   tabla_simbolos ts_ambito(&ts);
   scope_exit fin_ambito([&]{ emite_destruye(ios.second, ts_ambito); });

   if (!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt, ios), [&](auto checado) {
      if (checado->valor) {
         for(const auto& si : s.parte_si){
            evalua(*si, ts_ambito, ios);
         }
      } else {
         for(const auto& no : s.parte_no){
            evalua(*no, ts_ambito, ios);
         }
      }
   })){
      throw error(*s.pos, "Tipo inválido en condición");
   }
}
void evalua(const sentencia_for& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   tabla_simbolos ts_init(&ts);
   scope_exit fin_init([&]{ emite_destruye(ios.second, ts_init); });
   evalua(*s.inicializacion, ts_init, ios);

   for(;;){
      tabla_temporales tt; bool b = true;
      tabla_simbolos ts_ambito(&ts_init);
      scope_exit fin_ambito([&]{ emite_destruye(ios.second, ts_ambito); });

      if(!s.condicion.empty( ) && !valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts_ambito, tt, ios), [&](auto checado){
         b = checado->valor;
      })){
         throw error(*s.pos, "Tipo invalido de condicion");
      }
      if(!b){
         break;
      }

      try{
         for(const auto& sentencia : s.sentencias){
            evalua(*sentencia, ts_ambito, ios);
         }
      }catch(const sentencia_break &e){
         break;
      }catch(const sentencia_continue & c){
         ;
      }
      evalua(s.actualizacion, ts_ambito, tt, ios);
   }
}
void evalua(const sentencia_do& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   for(;;){
      tabla_temporales tt; bool b;
      tabla_simbolos ts_ambito(&ts);
      scope_exit fin_ambito([&]{ emite_destruye(ios.second, ts_ambito); });

      try{
         for(const auto& sentencia : s.sentencias){
            evalua(*sentencia, ts_ambito, ios);
         }
      }catch(const sentencia_break &e){
         break;
      }catch(const sentencia_continue & c){
         ;
      }

      if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt, ios), [&](auto checado){
         b = checado->valor;
      })){
         throw error(*s.pos, "Tipo invalido de condicion");
      }
      if(!b){
         break;
      }
   }
}
void evalua(const sentencia_while& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   for(;;){
      tabla_temporales tt; bool b;
      tabla_simbolos ts_ambito(&ts);
      scope_exit fin_ambito([&]{ emite_destruye(ios.second, ts_ambito); });

      if(!valida_ejecuta<valor_escalar<int>*, valor_escalar<float>*>(evalua(s.condicion, ts, tt, ios), [&](auto checado){
         b = checado->valor;
      })){
         throw error(*s.pos, "Tipo invalido de condicion");
      }
      if(!b){
         break;
      }

      try{
         for(const auto& sentencia : s.sentencias){
            evalua(*sentencia, ts_ambito, ios);
         }
      }catch(const sentencia_break &e){
         break;
      }catch(const sentencia_continue &c){
         ;
      }
   }
}
void evalua(const sentencia_break& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   throw s;
}
void evalua(const sentencia_continue& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   throw s;
}

void evalua(const sentencia& s, tabla_simbolos& ts, std::pair<std::istream&, std::ostream&> ios) {
   if (auto checar = dynamic_cast<const sentencia_expresiones*>(&s); checar != nullptr) {
      return evalua(*checar, ts, ios);
   }else if(auto checar = dynamic_cast<const sentencia_declaraciones*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }else if (auto checar = dynamic_cast<const sentencia_if*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }else if(auto checar = dynamic_cast<const sentencia_for*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }else if(auto checar = dynamic_cast<const sentencia_break*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }else if(auto checar = dynamic_cast<const sentencia_continue*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }else if(auto checar = dynamic_cast<const sentencia_while*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }else if(auto checar = dynamic_cast<const sentencia_do*>(&s); checar != nullptr){
      return evalua(*checar, ts, ios);
   }
}

// rutina inicial

void evalua(const std::vector<std::unique_ptr<sentencia>>& sentencias, std::pair<std::istream&, std::ostream&> ios){
   tabla_simbolos ts;
   ts.agrega("scanf", std::make_unique<valor_funcion<scanf>>( ));
   ts.agrega("printf", std::make_unique<valor_funcion<printf>>( ));
   try{
      tabla_simbolos ts_ambito(&ts);
      scope_exit fin_ambito([&]{ emite_destruye(ios.second, ts_ambito); });
      for (const auto& sentencia : sentencias) {
         evalua(*sentencia, ts, ios);
      }
   }catch(const sentencia_break &e){
      throw error(*e.pos, "No se permite hacer un break fuera de un ciclo");
   }catch(const sentencia_continue & c){
      throw error(*c.pos, "No se permite hacer un continue fuera de un ciclo");
   }
}

#endif
