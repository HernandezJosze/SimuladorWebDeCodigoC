//
// Created by Hernandez on 4/18/2021.
//

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

   expresion_terminal(const token_anotada* tp, const token_anotada* tipo = nullptr)
   : tk(tp), expresion(tipo){
   }
};

struct expresion_op_prefijo: expresion {
   const token_anotada* operador;
   std::unique_ptr<expresion> sobre;

   expresion_op_prefijo(const token_anotada* op, std::unique_ptr<expresion>&& s, const token_anotada* tipo)
   : operador(op), sobre(std::move(s)), expresion(tipo) {
   }
};

struct expresion_op_posfijo: expresion {
   std::unique_ptr<expresion> sobre;
   const token_anotada* operador;

   expresion_op_posfijo(std::unique_ptr<expresion>&& s, const token_anotada* op, const token_anotada* tipo)
   : sobre(std::move(s)), operador(op), expresion(tipo) {
   }
};

struct expresion_op_binario : expresion {
   std::unique_ptr<expresion> izq;
   const token_anotada* operador;
   std::unique_ptr<expresion> der;

   expresion_op_binario(std::unique_ptr<expresion>&& i, const token_anotada* op, std::unique_ptr<expresion>&& d, const token_anotada* tipo)
   : izq(std::move(i)), operador(op), der(std::move(d)), expresion(tipo) {
   }
};

struct expresion_llamada : expresion {
   std::unique_ptr<expresion> func;
   std::vector<std::unique_ptr<expresion>> parametros;

   expresion_llamada(std::unique_ptr<expresion>&& f, std::vector<std::unique_ptr<expresion>>&& p, const token_anotada* tipo)
   : func(std::move(f)), parametros(std::move(p)), expresion(tipo) {
   }
};

struct expresion_corchetes : expresion{
   std::unique_ptr<expresion> ex, dentro;

   expresion_corchetes(std::unique_ptr<expresion>&& e, std::unique_ptr<expresion>&& d, const token_anotada* tipo)
   : ex(std::move(e)), dentro(std::move(d)), expresion(tipo) {
   }
};

struct expresion_arreglo : expresion{
   std::vector<std::unique_ptr<expresion>> elementos;

   expresion_arreglo(std::vector<std::unique_ptr<expresion>>&& e, const token_anotada* tp)
   : elementos(std::move(e)), expresion(tp) {
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
      auto tipo = iter;
      espera(iter, LLAVE_I, "Se esperaba {");
      std::vector<std::unique_ptr<expresion>> elem = parsea_lista_expresiones(iter, true);
      espera(iter, LLAVE_D, "Se esperaba }");
      return std::make_unique<expresion_arreglo>(std::move(elem), tipo);
   } else {
      auto tipo = iter;
      return std::make_unique<expresion_terminal>(espera(iter, es_terminal, "Se esperaba identificador o literal"), tipo);
   }
}

std::unique_ptr<expresion> parsea_expresion_unaria(const token_anotada*& iter){
   if(es_operador_prefijo(iter->tipo)){
      auto operador = iter++;
      return std::make_unique<expresion_op_prefijo>(operador, parsea_expresion_unaria(iter), operador);
   }
   auto tipo = iter;
   auto ex = parsea_expresion_primaria(iter); //si no es un prefijo ya comienza una expresión
   while(es_operador_posfijo(iter->tipo)){
      if(iter->tipo == PARENTESIS_I){
         espera(iter, PARENTESIS_I, "Se esperaba (");
         std::vector<std::unique_ptr<expresion>> parametros = parsea_lista_expresiones(iter, true);
         espera(iter, PARENTESIS_D, "Se esperaba )");
         ex = std::make_unique<expresion_llamada>(std::move(ex), std::move(parametros), tipo);
      }else if(iter->tipo == CORCHETE_I){
         ++iter;
         auto dentro = parsea_expresion(iter);
         espera(iter, CORCHETE_D, "Se esperaba ]");
         ex = std::make_unique<expresion_corchetes>(std::move(ex), std::move(dentro), tipo);
      } else {
         ex = std::make_unique<expresion_op_posfijo>(std::move(ex), iter++, tipo);
      }
   }
   return ex;
}

std::unique_ptr<expresion> parsea_expresion_n_aria(const token_anotada*& iter, int prec){
   std::unique_ptr<expresion> ex = parsea_expresion_unaria(iter);
   while(es_operador_binario(iter->tipo) && precedencia(iter->tipo) >= prec){
      auto op = iter++;
      ex = std::make_unique<expresion_op_binario>(std::move(ex), op, parsea_expresion_n_aria(iter, precedencia(op->tipo) + asociatividad(op->tipo)), op);
   }
   return ex;
}

std::unique_ptr<expresion> parsea_expresion(const token_anotada*& iter){
   return parsea_expresion_n_aria(iter, 0);
}

#endif
