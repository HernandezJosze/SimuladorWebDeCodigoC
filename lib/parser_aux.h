//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_AUX_H
#define SIMULADORWEBDECODIGOC_PARSER_AUX_H

#include "lexer.h"

const token_anotada* espera(const token_anotada*& iter, auto pred){
   if(!pred(iter->tipo)){
      throw std::runtime_error("ERROR PARSER");
   }
   return iter++;
}

const token_anotada* espera(const token_anotada*& iter, token esperada){
   return espera(iter, [&](token tipo) {
      return tipo == esperada;
   });
}

int precedencia(token t) {
   // corregir con todos los operadores que sí tenemos (seguir la tabla de precedencia de C; mayor número es mayor precedencia)
   /*if(t == OR){
      return 1;
   }else if(t == AND){
      return 2;
   }else if(t == IGUAL || t == DIFERENTE){
      return 3;
   }else if(t == MENOR || t == MENOR_IGUAL || t == MAYOR || t == MAYOR_IGUAL){
      return 4;
   }else if(t == SUMA || t == RESTA){
      return 5;
   }else if(t == MULTIPLICACION || t == DIVISION || t == PISO || t == RESIDUO){
      return 6;
   }else if(t == POTENCIA){
      return 7;
   }*/
   return -1;
}

int asociatividad(token t) {
   return (t != ASIGNACION);
}

bool es_operador_prefijo(token t){
   // corregir con todos los operadores que sí tenemos
   // return t == MAS || t == MENOS;
}

bool es_operador_binario(token t){
   // corregir con todos los operadores que sí tenemos
   // return t == IGUAL || t == SUMA || t == RESTA || t == DIVISION || t == MULTIPLICACION || t == RESIDUO || t == PISO || t == POTENCIA || t == AND || t == OR || t == DIFERENTE;
}

bool es_tipo(token t){
   // corregir con todos los operadores que sí tenemos (ej INT, FLOAT)
}

bool es_literal(token t){
   // corregir con todos los operadores que sí tenemos
   //return t == LITERAL_ENTERA || t == LITERAL_FLOTANTE || t == LITERAL_CADENA;
}

bool es_terminal(token t){
   return t == IDENTIFICADOR || es_literal(t);
}

#endif
