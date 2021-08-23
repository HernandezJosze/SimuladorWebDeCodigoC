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

static int indentacion = 0;
std::ostream& operator<<(std::ostream& os, const sentencia& s);
std::ostream& operator<<(std::ostream& os, const sentencia_expresiones& s) {
   os << std::string(indentacion, ' ');
   for (int i = 0; i < s.ex.size( ); ++i) {
      os << *s.ex[i];
      if (i < s.ex.size( ) - 1) {
         os << ", ";
      }
   }
   return os;
}
std::ostream& operator<<(std::ostream& os, const sentencia_declaraciones& s) {
   os << std::string(indentacion, ' ') << *s.tipo << " ";
   for(int i = 0; i < s.subdeclaraciones.size( ); ++i){
      os << *s.subdeclaraciones[i].nombre;
      for (const auto& actual : s.subdeclaraciones[i].tamanios) {
         os << "[" << *actual << "]";
      }
      if(s.subdeclaraciones[i].inicializador != nullptr){
         os << " = " << *s.subdeclaraciones[i].inicializador;
      }
      os << (i + 1 == s.subdeclaraciones.size( ) ? "" : ", ");
   }
   return os;
}
std::ostream& operator<<(std::ostream& os, const sentencia_if& s) {
   os << std::string(indentacion, ' ') << "if(" << *s.condicion << "){\n";
   indentacion += 3;
   for (const auto& p : s.parte_si){
      os << *p << "\n";
   }
   indentacion -= 3;
   os << std::string(indentacion, ' ') << "}" << (s.parte_no.empty( ) ? "" : "else{\n");
   indentacion += 3;
   for (const auto& p : s.parte_no){
      os << *p << "\n";
   }
   indentacion -= 3;
   os << std::string(indentacion, ' ') << (s.parte_no.empty( ) ? "" : "}");
   return os;
}
std::ostream& operator<<(std::ostream& os, const sentencia_for& s) {
   os << std::string(indentacion, ' ') << "for(" << *s.inicializacion << ";" << *s.condicion << ";" << *s.actualizacion << "){\n";
   indentacion += 3;
   for(auto& se : s.sentencias){
      os << *se << "\n";
   }
   indentacion -= 3;
   os << std::string(indentacion, ' ') << "}";
   return os;
}
std::ostream& operator<<(std::ostream& os, const sentencia_do& s) {
   os << std::string(indentacion, ' ') << "do{\n";
   indentacion += 3;
   for(auto& se : s.sentencias){
      os << *se << "\n";
   }
   indentacion -= 3;
   os << std::string(indentacion, ' ') << "}while(" << *s.condicion << ");\n";
   return os;
}
std::ostream& operator<<(std::ostream& os, const sentencia_while& s) {
   os << std::string(indentacion, ' ') << "while(" << *s.condicion << "){\n";
   indentacion += 3;
   for(auto& se : s.sentencias){
      os << *se << "\n";
   }
   indentacion -= 3;
   os << std::string(indentacion, ' ') << "}";
   return os;
}
std::ostream& operator<<(std::ostream& os, const sentencia_break& s) {
   return os << std::string(indentacion, ' ') << "break";
}
std::ostream& operator<<(std::ostream& os, const sentencia_continue& s) {
   return os << std::string(indentacion, ' ') << "continue";
}

std::ostream& operator<<(std::ostream& os, const sentencia& s) {
   if (auto checar = dynamic_cast<const sentencia_expresiones*>(&s); checar != nullptr) {
      os << *checar;
   }else if(auto checar = dynamic_cast<const sentencia_declaraciones*>(&s); checar != nullptr){
      os << *checar;
   }else if (auto checar = dynamic_cast<const sentencia_if*>(&s); checar != nullptr){
      os << *checar;
   }else if(auto checar = dynamic_cast<const sentencia_for*>(&s); checar != nullptr){
      os << *checar;
   }else if(auto checar = dynamic_cast<const sentencia_break*>(&s); checar != nullptr){
      os << *checar;
   }else if(auto checar = dynamic_cast<const sentencia_continue*>(&s); checar != nullptr){
      os << *checar;
   }else if(auto checar = dynamic_cast<const sentencia_while*>(&s); checar != nullptr){
      os << *checar;
   }else if(auto checar = dynamic_cast<const sentencia_do*>(&s); checar != nullptr){
      os << *checar;
   }
   return os;
}

#endif
