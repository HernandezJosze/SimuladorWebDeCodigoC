//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_EXPRESION_H
#define SIMULADORWEBDECODIGOC_PARSER_EXPRESION_H

#include "parser_aux.h"
#include <memory>
#include <vector>

struct expresion {
   const token_anotada* tk;
   expresion(const token_anotada* p)
   : tk(p){
   }
   virtual ~expresion() = 0;
};
expresion::~expresion(){}

struct expresion_terminal : expresion {

   expresion_terminal(const token_anotada* tp)
   : expresion(tp) {
   }
};

struct expresion_op_prefijo: expresion {
   std::unique_ptr<expresion> sobre;

   expresion_op_prefijo(const token_anotada* op, std::unique_ptr<expresion>&& s)
   : expresion(op), sobre(std::move(s)) {
   }
};

struct expresion_op_posfijo: expresion {
   std::unique_ptr<expresion> sobre;

   expresion_op_posfijo(std::unique_ptr<expresion>&& s, const token_anotada* op)
   : sobre(std::move(s)), expresion(op) {
   }
};

struct expresion_op_binario : expresion {
   std::unique_ptr<expresion> izq;
   std::unique_ptr<expresion> der;

   expresion_op_binario(std::unique_ptr<expresion>&& i, const token_anotada* op, std::unique_ptr<expresion>&& d)
   : izq(std::move(i)), expresion(op), der(std::move(d)) {
   }
};

struct expresion_llamada : expresion{
   std::unique_ptr<expresion> func;
   std::vector<std::unique_ptr<expresion>> parametros;

   expresion_llamada(const token_anotada* op, std::unique_ptr<expresion>&& f, std::vector<std::unique_ptr<expresion>>&& p)
   : expresion(op), func(std::move(f)), parametros(std::move(p)) {
   }
};

struct expresion_corchetes : expresion{
   std::unique_ptr<expresion> ex, dentro;

   expresion_corchetes(const token_anotada* op, std::unique_ptr<expresion>&& e, std::unique_ptr<expresion>&& d)
   : expresion(op), ex(std::move(e)), dentro(std::move(d)) {
   }
};

struct expresion_arreglo : expresion{
   std::vector<std::unique_ptr<expresion>> elementos;

   expresion_arreglo(const token_anotada* op, std::vector<std::unique_ptr<expresion>>&& e)
   : expresion(op), elementos(std::move(e)) {
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
      espera(iter, LLAVE_I, "Se esperaba {");
      std::vector<std::unique_ptr<expresion>> elem = parsea_lista_expresiones(iter, true);
      espera(iter, LLAVE_D, "Se esperaba }");
      return std::make_unique<expresion_arreglo>(iter, std::move(elem));
   } else {
      return std::make_unique<expresion_terminal>(espera(iter, es_terminal, "Se esperaba identificador o literal"));
   }
}

std::unique_ptr<expresion> parsea_expresion_unaria(const token_anotada*& iter){
   if(es_operador_prefijo(iter->tipo)){
      auto operador = iter++;
      return std::make_unique<expresion_op_prefijo>(operador, parsea_expresion_unaria(iter));
   }
   auto op = iter;
   auto ex = parsea_expresion_primaria(iter); //si no es un prefijo ya comienza una expresión
   while(es_operador_posfijo(iter->tipo)){
      if(iter->tipo == PARENTESIS_I){
         espera(iter, PARENTESIS_I, "Se esperaba (");
         std::vector<std::unique_ptr<expresion>> parametros = parsea_lista_expresiones(iter, true);
         espera(iter, PARENTESIS_D, "Se esperaba )");
         ex = std::make_unique<expresion_llamada>(op, std::move(ex), std::move(parametros));
      }else if(iter->tipo == CORCHETE_I){
         espera(iter, CORCHETE_I, "se esperaba [");
         auto dentro = parsea_expresion(iter);
         espera(iter, CORCHETE_D, "Se esperaba ]");
         ex = std::make_unique<expresion_corchetes>(op, std::move(ex), std::move(dentro));
      } else {
         ex = std::make_unique<expresion_op_posfijo>(std::move(ex), iter++);
      }
   }
   return ex;
}

std::unique_ptr<expresion> parsea_expresion_n_aria(const token_anotada*& iter, int prec){
   std::unique_ptr<expresion> ex = parsea_expresion_unaria(iter);
   while(es_operador_binario(iter->tipo) && precedencia(iter->tipo) >= prec){
      auto op = iter++;
      ex = std::make_unique<expresion_op_binario>(std::move(ex), op, parsea_expresion_n_aria(iter, precedencia(op->tipo) + asociatividad(op->tipo)));
   }
   return ex;
}

std::unique_ptr<expresion> parsea_expresion(const token_anotada*& iter){
   return parsea_expresion_n_aria(iter, 0);
}

#endif
