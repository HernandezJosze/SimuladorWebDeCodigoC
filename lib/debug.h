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
   if (auto checar = dynamic_cast<const expresion_terminal*>(&e); checar != nullptr) {
      os << *checar->t;
   } else if (auto checar = dynamic_cast<const expresion_op_prefijo*>(&e); checar != nullptr) {
      os << "(" << *checar->operador << *checar->sobre << ")";
   } else if (auto checar = dynamic_cast<const expresion_op_posfijo*>(&e); checar != nullptr) {
      os << "(" << *checar->sobre << *checar->operador << ")";
   } else if (auto checar = dynamic_cast<const expresion_op_binario*>(&e); checar != nullptr) {
      os << "(" << *checar->izq << ' ' << *checar->operador << ' ' << *checar->der << ")";
   } else if (auto checar = dynamic_cast<const expresion_llamada*>(&e); checar != nullptr) {
      os << *checar->func << "(";
      for (const auto& p : checar->parametros) {
         os << *p << (checar->parametros.back( ) == p ? "" : ", ");
      }
      os << ")";
   } else if (auto checar = dynamic_cast<const expresion_corchetes*>(&e); checar != nullptr) {
      os << *checar->ex << "[" << *checar->dentro << "]";
   } else if (auto checar = dynamic_cast<const expresion_arreglo*>(&e); checar != nullptr) {
      os << "{";
      for (const auto& e : checar->elementos) {
         os << *e << (checar->elementos.back( ) == e ? "" : ", ");
      }
      os << "}";
   }

   return os;
}

std::ostream& operator<<(std::ostream& os, const sentencia& s) {
   if (auto checar = dynamic_cast<const sentencia_expresion*>(&s); checar != nullptr) {
      os << *checar->ex << ";";
   }

   return os;
}

#endif
