//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_EXPRESION_H
#define SIMULADORWEBDECODIGOC_PARSER_EXPRESION_H

#include "parser_aux.h"
#include <memory>
#include <vector>

struct expresion {
   virtual ~expresion() = 0;
};
expresion::~expresion(){}

struct expresion_terminal : expresion {
   const token_anotada* tk;

   expresion_terminal(const token_anotada* tp)
   : tk(tp) {
   }
};

struct expresion_op_prefijo: expresion {
   const token_anotada* operador;
   std::unique_ptr<expresion> sobre;

   expresion_op_prefijo(const token_anotada* op, std::unique_ptr<expresion>&& s)
   : operador(op), sobre(std::move(s)) {
   }
};

struct expresion_op_posfijo: expresion {
   std::unique_ptr<expresion> sobre;
   const token_anotada* operador;

   expresion_op_posfijo(std::unique_ptr<expresion>&& s, const token_anotada* op)
   : sobre(std::move(s)), operador(op) {
   }
};

struct expresion_op_binario : expresion {
   std::unique_ptr<expresion> izq;
   const token_anotada* operador;
   std::unique_ptr<expresion> der;

   expresion_op_binario(std::unique_ptr<expresion>&& i, const token_anotada* op, std::unique_ptr<expresion>&& d)
   : izq(std::move(i)), operador(op), der(std::move(d)) {
   }
};

struct expresion_llamada : expresion{
   std::unique_ptr<expresion> func;
   std::vector<std::unique_ptr<expresion>> parametros;

   expresion_llamada(std::unique_ptr<expresion>&& f, std::vector<std::unique_ptr<expresion>>&& p)
   : func(std::move(f)), parametros(std::move(p)) {
   }
};

struct expresion_corchetes : expresion{
   std::unique_ptr<expresion> ex, dentro;

   expresion_corchetes(std::unique_ptr<expresion>&& e, std::unique_ptr<expresion>&& d)
   : ex(std::move(e)), dentro(std::move(d)) {
   }
};

struct expresion_arreglo : expresion{
   std::vector<std::unique_ptr<expresion>> elementos;

   expresion_arreglo(std::vector<std::unique_ptr<expresion>>&& e)
   : elementos(std::move(e)) {
   }
};


std::unique_ptr<expresion> parsea_expresion(const token_anotada*& iter);

std::unique_ptr<expresion> parsea_expresion_primaria(const token_anotada*& iter){
   if(iter->tipo == PARENTESIS_I){
      std::unique_ptr<expresion> ex = parsea_expresion(++iter);
      espera(iter, PARENTESIS_D, "Se esperaba )");
      return ex;
   }else if(iter->tipo == LLAVE_I){
      /* reconocer inicializadores de arreglo       { a, b, c, d }       donde los elementos podr�an estar tambi�n dentro de llaves */
      std::vector<std::unique_ptr<expresion>> elem;
      espera(iter, LLAVE_I, "Se esperaba {");
      while(iter->tipo != LLAVE_D){
          elem.push_back(parsea_expresion(iter));
          if(iter->tipo != LLAVE_D){
              espera(iter, COMA, "Se esperaba ,");
          }
      }
      espera(iter, LLAVE_D, "Se esperaba }");
      return std::make_unique<expresion_arreglo>(std::move(elem));
   } else {
      return std::make_unique<expresion_terminal>(espera(iter, es_terminal, "Se esperaba identificador o literal"));
   }
}

std::unique_ptr<expresion> parsea_expresion_unaria(const token_anotada*& iter){
   if(es_operador_prefijo(iter->tipo)){
      auto operador = iter++;
      return std::make_unique<expresion_op_prefijo>(operador, parsea_expresion_unaria(iter));
   }
   auto ex = parsea_expresion_primaria(iter); //si no es un prefijo ya comienza una expresión
   while(es_operador_posfijo(iter->tipo)){
      if(iter->tipo == PARENTESIS_I){
         ++iter;
         std::vector<std::unique_ptr<expresion>> parametros;
         while(iter->tipo != PARENTESIS_D){
            parametros.push_back(parsea_expresion(iter));
            if(iter->tipo != PARENTESIS_D){
               espera(iter, COMA, "Se esperaba ,");
            }
         }
         espera(iter, PARENTESIS_D, "Se esperaba )");
         ex = std::make_unique<expresion_llamada>(std::move(ex), std::move(parametros));
      }else if(iter->tipo == CORCHETE_I){
         ++iter;
         auto dentro = parsea_expresion(iter);
         espera(iter, CORCHETE_D, "Se esperaba ]");
         ex = std::make_unique<expresion_corchetes>(std::move(ex), std::move(dentro));
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
