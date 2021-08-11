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
   if(t == ASIGNACION || t == DIFERENTE){
      return 1;
   }else if(t == MENOR || t == MENOR_IGUAL || t == MAYOR || t == MAYOR_IGUAL){
      return 2;
   }else if(t == MAS || t == MENOS){
      return 3;
   }else if(t == MULTIPLICACION || t == DIVISION || t == MODULO){
      return 4;
   }else if(t == PARENTESIS_I){
      return 5;
   }else if(t == CORCHETE_I || t == COMA){
       return 6;
   }
   return -1;
}

int asociatividad(token t) {
   return t != ASIGNACION;
}

bool es_operador_prefijo(token t){
   return t ==  PARENTESIS_I || t == CORCHETE_I;
}

bool es_operador_binario(token t){
    return t == IGUAL || t == MAS || t == MENOS || t == DIVISION || t == MULTIPLICACION || t == DIFERENTE || t == MAYOR || t == MENOR || t == MAYOR_IGUAL || t == MENOR_IGUAL;
}

bool es_tipo(token t){
    return t == INT || t == FLOAT;
}

bool es_literal(token t){
   return t == LITERAL_ENTERA || t == LITERAL_FLOTANTE || t == LITERAL_CADENA;
}

bool es_terminal(token t){
   return t == IDENTIFICADOR || es_literal(t);
}

#endif
