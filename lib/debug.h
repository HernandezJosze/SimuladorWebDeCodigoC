//
// Created by :)-|-< on 8/14/2021.
//

#ifndef SIMULADORWEBDECODIGOC_DEBUG_H
#define SIMULADORWEBDECODIGOC_DEBUG_H

#include "lexer.h"
#include "parser.h"
#include <iostream>

std::ostream& operator<<(std::ostream& os, const token_anotada& t) {
   return os << (t.tipo == LITERAL_CADENA ? "\"" : "") << t.location << (t.tipo == LITERAL_CADENA ? "\"" : "");
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
   }else if(auto checar = dynamic_cast<const sentencia_declaracion*>(&s); checar != nullptr){
      os << *checar->tipo << " ";
      for(int i = 0; i < checar->nombre.size( ); ++i){
         os << *checar->nombre[i];
         if(checar->tamanio[i]){
            os << "[" << *checar->tamanio[i] << "]";
         }
         if(checar->inicializador[i]){
            os << " = " << *checar->inicializador[i];
         }
         os << (i + 1 == checar->nombre.size( ) ? ";" : ", ");
      }
   }else if (auto checar = dynamic_cast<const sentencia_if*>(&s); checar != nullptr){
      os << "if(" << *checar->condicion << "){\n";
      for (const auto& p : checar->parte_si){
         os << "   " << *p << '\n';
      }
      os << "}" << (checar->esElseif ? "else " : (checar->parte_no.empty( ) ? "" : "else{\n"));
      for (const auto& p : checar->parte_no){
         os << (checar->esElseif && p == checar->parte_no.front( ) ? "" : "   ") << *p << '\n';
      }
      os << (checar->esElseif ? "" : (checar->parte_no.empty( ) ? "" : "}\n"));

   }else if(auto checar = dynamic_cast<const sentencia_for*>(&s); checar != nullptr){
      os << "for(";
      if(checar->inicializacion){
         os << *checar->inicializacion;
      }else{
         os << ";";
      }
      os << *checar->condicion << ";";
      for(auto& p : checar->incrementos){
         os << *p << (p == checar->incrementos.back( ) ? "" : ", ");
      }
      os << "){\n";
      for(auto& se : checar->aEjecutar){
         os << "   " << *se << "\n";
      }
      os << "\n}\n";
   }

   return os;
}

#endif
