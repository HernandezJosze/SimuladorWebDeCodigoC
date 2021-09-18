#ifndef SIMULADORWEBDECODIGOC_PARSER_EXPRESION_H
#define SIMULADORWEBDECODIGOC_PARSER_EXPRESION_H

#include "parser_aux.h"
#include <memory>
#include <vector>

struct expresion {
   const token_anotada* pos;

   expresion(const token_anotada* tipo)
   : pos(tipo){
   }
   virtual ~expresion() = 0;
};
expresion::~expresion(){}

struct expresion_terminal : expresion {
   const token_anotada* tk;

   expresion_terminal(const token_anotada* p, const token_anotada* t)
   : expresion(p), tk(t){
   }
};

struct expresion_op_prefijo: expresion {
   const token_anotada* operador;
   std::unique_ptr<expresion> sobre;

   expresion_op_prefijo(const token_anotada* p, const token_anotada* op, std::unique_ptr<expresion>&& s)
   : expresion(p), operador(op), sobre(std::move(s)) {
   }
};

struct expresion_op_posfijo: expresion {
   std::unique_ptr<expresion> sobre;
   const token_anotada* operador;

   expresion_op_posfijo(const token_anotada* p, std::unique_ptr<expresion>&& s, const token_anotada* op)
   : expresion(p), sobre(std::move(s)), operador(op) {
   }
};

struct expresion_op_binario : expresion {
   std::unique_ptr<expresion> izq;
   const token_anotada* operador;
   std::unique_ptr<expresion> der;

   expresion_op_binario(const token_anotada* p, std::unique_ptr<expresion>&& i, const token_anotada* op, std::unique_ptr<expresion>&& d)
   : expresion(p), izq(std::move(i)), operador(op), der(std::move(d)){
   }
};

struct expresion_llamada : expresion {
   std::unique_ptr<expresion> func;
   std::vector<std::unique_ptr<expresion>> parametros;

   expresion_llamada(const token_anotada* pos, std::unique_ptr<expresion>&& f, std::vector<std::unique_ptr<expresion>>&& p)
   : expresion(pos), func(std::move(f)), parametros(std::move(p)){
   }
};

struct expresion_corchetes : expresion{
   std::unique_ptr<expresion> ex, dentro;

   expresion_corchetes(const token_anotada* p, std::unique_ptr<expresion>&& e, std::unique_ptr<expresion>&& d)
   : expresion(p), ex(std::move(e)), dentro(std::move(d)) {
   }
};

struct expresion_arreglo : expresion{
   std::vector<std::unique_ptr<expresion>> elementos;

   expresion_arreglo(const token_anotada* p, std::vector<std::unique_ptr<expresion>>&& e)
   : expresion(p), elementos(std::move(e)) {
   }
};


std::unique_ptr<expresion> parsea_expresion(const token_anotada*& iter);

std::vector<std::unique_ptr<expresion>> parsea_lista_expresiones(const token_anotada*& iter, bool permitir_vacio) {
   std::vector<std::unique_ptr<expresion>> ex;
   if (es_inicio_expresion(iter->tipo)) {
      ex.push_back(parsea_expresion(iter));
      while (iter->tipo == COMA) {
         ex.push_back(parsea_expresion(++iter));
      }
   }
   if (ex.empty( ) && !permitir_vacio) {
      throw error(*iter, "Se esperaba una expresión");
   }
   return ex;
}
std::unique_ptr<expresion> parsea_expresion_primaria(const token_anotada*& iter){
   if(iter->tipo == PARENTESIS_I){
      std::unique_ptr<expresion> ex = parsea_expresion(++iter);
      espera(iter, PARENTESIS_D, "Se esperaba )");
      return ex;
   }else if(iter->tipo == LLAVE_I){
      auto pos = iter;
      espera(iter, LLAVE_I, "Se esperaba {");
      std::vector<std::unique_ptr<expresion>> elem = parsea_lista_expresiones(iter, true);
      espera(iter, LLAVE_D, "Se esperaba }");
      return std::make_unique<expresion_arreglo>(pos, std::move(elem));
   } else {
      auto pos = iter;
      return std::make_unique<expresion_terminal>(pos, espera(iter, es_terminal, "Se esperaba identificador o literal"));
   }
}

std::unique_ptr<expresion> parsea_expresion_unaria(const token_anotada*& iter){
   if(es_operador_prefijo(iter->tipo)){
      auto pos = iter, operador = iter++;
      return std::make_unique<expresion_op_prefijo>(pos, operador, parsea_expresion_unaria(iter));
   }
   auto pos = iter;
   auto ex = parsea_expresion_primaria(iter);
   while(es_operador_posfijo(iter->tipo)){
      if(iter->tipo == PARENTESIS_I){
         espera(iter, PARENTESIS_I, "Se esperaba (");
         std::vector<std::unique_ptr<expresion>> parametros = parsea_lista_expresiones(iter, true);
         espera(iter, PARENTESIS_D, "Se esperaba )");
         ex = std::make_unique<expresion_llamada>(pos, std::move(ex), std::move(parametros));
      }else if(iter->tipo == CORCHETE_I){
         ++iter;
         auto dentro = parsea_expresion(iter);
         espera(iter, CORCHETE_D, "Se esperaba ]");
         ex = std::make_unique<expresion_corchetes>(pos, std::move(ex), std::move(dentro));
      } else {
         ex = std::make_unique<expresion_op_posfijo>(pos, std::move(ex), iter++);
      }
   }
   return ex;
}

std::unique_ptr<expresion> parsea_expresion_n_aria(const token_anotada*& iter, int prec){
   std::unique_ptr<expresion> ex = parsea_expresion_unaria(iter);
   while(es_operador_binario(iter->tipo) && precedencia(iter->tipo) >= prec){
      auto pos = iter, op = iter++;
      ex = std::make_unique<expresion_op_binario>(pos, std::move(ex), op, parsea_expresion_n_aria(iter, precedencia(op->tipo) + asociatividad(op->tipo)));
   }
   return ex;
}

std::unique_ptr<expresion> parsea_expresion(const token_anotada*& iter){
   return parsea_expresion_n_aria(iter, 0);
}

#endif
