//
// Created by :)-|-< on 8/14/2021.
//

#ifndef SIMULADORWEBDECODIGOC_DEBUG_H
#define SIMULADORWEBDECODIGOC_DEBUG_H

#include "lexer.h"
#include "parser.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const token_anotada& t) {
   return os << t.location;
}

std::ostream& operator<<(std::ostream& os, const expresion& e) {
   if (auto x = dynamic_cast<const expresion_terminal*>(&e); x != nullptr) {
      os << *x->t;
   } else if (auto x = dynamic_cast<const expresion_op_prefijo*>(&e); x != nullptr) {
      os << "(" << *x->operador << *x->sobre << ")";
   } else if (auto x = dynamic_cast<const expresion_op_posfijo*>(&e); x != nullptr) {
      os << "(" << *x->sobre << *x->operador << ")";
   } else if (auto x = dynamic_cast<const expresion_op_binario*>(&e); x != nullptr) {
      os << "(" << *x->izq << *x->operador << *x->der << ")";
   } else if (auto x = dynamic_cast<const expresion_llamada*>(&e); x != nullptr) {
      os << *x->func << "(";
      for (const auto& p : x->parametros) {
         os << *p << ",";
      }
      os << ")";
   } else if (auto x = dynamic_cast<const expresion_corchetes*>(&e); x != nullptr) {
      os << *x->ex << "[" << *x->dentro << "]";
   } else if (auto x = dynamic_cast<const expresion_arreglo*>(&e); x != nullptr) {
      os << "{";
      for (const auto& e : x->elementos) {
         os << *e << ",";
      }
      os << "}";
   }

   return os;
}

#endif
