//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_AUX_H
#define SIMULADORWEBDECODIGOC_PARSER_AUX_H

#include "lexer.h"
#include <utility>

const token_anotada* espera(const token_anotada*& iter, auto pred, const char* mensaje){
   if(!pred(iter->tipo)){
      throw std::pair(*iter, mensaje);
   }
   return iter++;
}

const token_anotada* espera(const token_anotada*& iter, token esperada, const char* mensaje){
   return espera(iter, [&](token tipo) {
      return tipo == esperada;
   }, mensaje);
}

int precedencia(token t) {
    if(t == ASIGNACION){
        return 1;
    }else if(t == OR || t == AND){
        return 2;
    }else if(t == IGUAL || t == DIFERENTE){
        return 3;
   }else if(t == MENOR || t == MENOR_IGUAL || t == MAYOR || t == MAYOR_IGUAL){
      return 4;
   }else if(t == MAS || t == MENOS){
      return 5;
   }else if(t == MULTIPLICACION || t == DIVISION || t == MODULO){
       return 6;
   }
   return -1;
}

int asociatividad(token t) {
   return t != ASIGNACION;
}

bool es_operador_prefijo(token t){
   return t == INCREMENTO || t == DECREMENTO || t == NOT || t == MAS || t == MENOS;
}

bool es_operador_posfijo(token t) {
   return t == INCREMENTO || t == DECREMENTO || t == PARENTESIS_I || t == CORCHETE_I;
}

bool es_operador_binario(token t){
    return t == IGUAL || t == MAS || t == MENOS || t == DIVISION || t == MULTIPLICACION || t == DIFERENTE ||
           t == MAYOR || t == MENOR || t == MAYOR_IGUAL || t == MENOR_IGUAL || t == OR || t == AND;
}

bool es_tipo(token t){
    return t == INT || t == FLOAT || t == CHAR;
}

bool es_literal(token t){
   return t == LITERAL_ENTERA || t == LITERAL_FLOTANTE || t == LITERAL_CADENA;
}

bool es_terminal(token t){
   return t == IDENTIFICADOR || es_literal(t);
}

#endif
