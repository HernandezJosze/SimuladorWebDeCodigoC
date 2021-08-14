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
   const token_anotada* t;

   expresion_terminal(const token_anotada* tp)
   : t(tp) {
   }
};

struct expresion_op_prefijo: expresion {
   const token_anotada* operador;
   std::unique_ptr<expresion> sobre;

   expresion_op_prefijo(const token_anotada* op, std::unique_ptr<expresion>&& s)
   : operador(op), sobre(std::move(s)) {
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
   if(es_terminal(iter->tipo)){
      return std::make_unique<expresion_terminal>(iter++);
   }else if(iter->tipo == PARENTESIS_I){
      std::unique_ptr<expresion> ex = parsea_expresion(++iter);
      espera(iter, PARENTESIS_D);
      return ex;
   }else if(iter->tipo == LLAVE_I){
      /* reconocer inicializadores de arreglo       { a, b, c, d }       donde los elementos podr�an estar tambi�n dentro de llaves */
      std::vector<std::unique_ptr<expresion>> elem;
      espera(iter, LLAVE_I);
      while(iter->tipo != LLAVE_D){
          elem.push_back(parsea_expresion(iter));
          if(iter->tipo != LLAVE_D){
              espera(iter, COMA);
          }
      }
      espera(iter, LLAVE_D);
      return std::make_unique<expresion_arreglo>(std::move(elem));
   }
}

std::unique_ptr<expresion> parsea_expresion_unaria(const token_anotada*& iter){
   if(es_operador_prefijo(iter->tipo)){
      auto operador = iter++;
      return std::make_unique<expresion_op_prefijo>(operador, parsea_expresion_unaria(iter));
   }

   auto ex = parsea_expresion_primaria(iter); //si no es un prefijo ya comienza una expresión
   while(iter->tipo == PARENTESIS_I || iter->tipo == CORCHETE_I){
      if(iter->tipo == PARENTESIS_I){
         ++iter;
         std::vector<std::unique_ptr<expresion>> parametros;
         while(iter->tipo != PARENTESIS_D){
            parametros.push_back(parsea_expresion(iter));
            if(iter->tipo != PARENTESIS_D){ // si es parentesis no es una funcion, por lo tanto es parametro y esperamos una coma
               espera(iter, COMA);
            }
         }
         espera(iter, PARENTESIS_D);
         ex = std::make_unique<expresion_llamada>(std::move(ex), std::move(parametros));
      }else if(iter->tipo == CORCHETE_I){
         ++iter;
         auto dentro = parsea_expresion(iter);
         espera(iter, CORCHETE_D);
         ex = std::make_unique<expresion_corchetes>(std::move(ex), std::move(dentro));
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
