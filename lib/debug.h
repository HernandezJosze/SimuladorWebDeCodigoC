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

std::ostream& operator<<(std::ostream& os, const expresion&);

std::ostream& operator<<(std::ostream& os, const expresion_terminal& e) {
   return os << *e.tk;
}

std::ostream& operator<<(std::ostream& os, const expresion_op_prefijo& e) {
   return os << "(" << *e.operador << *e.sobre << ")";
}

std::ostream& operator<<(std::ostream& os, const expresion_op_posfijo& e) {
   return os << "(" << *e.sobre << *e.operador << ")";
}

std::ostream& operator<<(std::ostream& os, const expresion_op_binario& e) {
   return os << "(" << *e.izq << ' ' << *e.operador << ' ' << *e.der << ")";
}

std::ostream& operator<<(std::ostream& os, const expresion_llamada& e) {
   os << *e.func << "(";
   for (const auto& actual : e.parametros) {
      os << *actual << (e.parametros.back( ) == actual ? "" : ", ");
   }
   return os << ")";
}

std::ostream& operator<<(std::ostream& os, const expresion_corchetes& e) {
   return os << *e.ex << "[" << *e.dentro << "]";
}

std::ostream& operator<<(std::ostream& os, const expresion_arreglo& e) {
   os << "{";
   for (const auto& actual : e.elementos) {
      os << *actual << (e.elementos.back( ) == actual ? "" : ", ");
   }
   return os << "}";
}

std::ostream& operator<<(std::ostream& os, const expresion& e) {
   if (auto checar = dynamic_cast<const expresion_terminal*>(&e); checar != nullptr) {
      os << *checar;
   } else if (auto checar = dynamic_cast<const expresion_op_prefijo*>(&e); checar != nullptr) {
      os << *checar;
   } else if (auto checar = dynamic_cast<const expresion_op_posfijo*>(&e); checar != nullptr) {
      os << *checar;
   } else if (auto checar = dynamic_cast<const expresion_op_binario*>(&e); checar != nullptr) {
      os << *checar;
   } else if (auto checar = dynamic_cast<const expresion_llamada*>(&e); checar != nullptr) {
      os << *checar;
   } else if (auto checar = dynamic_cast<const expresion_corchetes*>(&e); checar != nullptr) {
      os << *checar;
   } else if (auto checar = dynamic_cast<const expresion_arreglo*>(&e); checar != nullptr) {
      os << *checar;
   }
   return os;
}

std::ostream& operator<<(std::ostream& os, const sentencia& s) {
   static int indentacion = 0;
   if (auto checar = dynamic_cast<const sentencia_expresiones*>(&s); checar != nullptr) {
      os << std::string(indentacion, ' ');
      for (int i = 0; i < checar->ex.size( ); ++i) {
         os << *checar->ex[i];
         if (i < checar->ex.size( ) - 1) {
            os << ", ";
         }
      }
   }else if(auto checar = dynamic_cast<const sentencia_declaraciones*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << *checar->tipo << " ";
      for(int i = 0; i < checar->subdeclaraciones.size( ); ++i){
         os << *checar->subdeclaraciones[i].nombre;
         for (const auto& actual : checar->subdeclaraciones[i].tamanios) {
            os << "[" << *actual << "]";
         }
         if(checar->subdeclaraciones[i].inicializador != nullptr){
            os << " = " << *checar->subdeclaraciones[i].inicializador;
         }
         os << (i + 1 == checar->subdeclaraciones.size( ) ? "" : ", ");
      }
   }else if (auto checar = dynamic_cast<const sentencia_if*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << "if(" << *checar->condicion << "){\n";
      indentacion += 3;
      for (const auto& p : checar->parte_si){
         os << *p << "\n";
      }
      indentacion -= 3;
      os << std::string(indentacion, ' ') << "}" << (checar->parte_no.empty( ) ? "" : "else{\n");
      indentacion += 3;
      for (const auto& p : checar->parte_no){
         os << *p << "\n";
      }
      indentacion -= 3;
      os << std::string(indentacion, ' ') << (checar->parte_no.empty( ) ? "" : "}");


   }else if(auto checar = dynamic_cast<const sentencia_for*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << "for(" << *checar->inicializacion << ";" << *checar->condicion << ";" << *checar->actualizacion << "){\n";
      indentacion += 3;
      for(auto& se : checar->sentencias){
         os << *se << "\n";
      }
      indentacion -= 3;
      os << std::string(indentacion, ' ') << "}";
   }else if(auto checar = dynamic_cast<const sentencia_break*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << "break";
   }else if(auto checar = dynamic_cast<const sentencia_continue*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << "continue";
   }else if(auto checar = dynamic_cast<const sentencia_while*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << "while(" << *checar->condicion << "){\n";
      indentacion += 3;
      for(auto& se : checar->sentencias){
         os << *se << "\n";
      }
      indentacion -= 3;
      os << std::string(indentacion, ' ') << "}";
   }else if(auto checar = dynamic_cast<const sentencia_do*>(&s); checar != nullptr){
      os << std::string(indentacion, ' ') << "do{\n";
      indentacion += 3;
      for(auto& se : checar->sentencias){
         os << *se << "\n";
      }
      indentacion -= 3;
      os << std::string(indentacion, ' ') << "}while(" << *checar->condicion << ");\n";
   }

   return os;
}

#endif
