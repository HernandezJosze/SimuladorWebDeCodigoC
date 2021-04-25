//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_SENTENCIA_H
#define SIMULADORWEBDECODIGOC_PARSER_SENTENCIA_H

#include "parser_aux.h"
#include "parser_expresion.h"

struct sentencia{
   virtual ~sentencia() = 0;
};
sentencia::~sentencia(){}

struct sentencia_declaracion : sentencia{
   const token_anotada* tipo;
   const token_anotada* nombre;
   std::unique_ptr<expresion> inicializador;

   sentencia_declaracion(const token_anotada* t, const token_anotada* n, std::unique_ptr<expresion>&& i)
   : tipo(t), nombre(n), inicializador(std::move(i)) {
   }
};

struct sentencia_if : sentencia{
   std::unique_ptr<expresion> condicion;
   std::vector<std::unique_ptr<sentencia>> parte_si;
   std::vector<std::unique_ptr<sentencia>> parte_no;

   sentencia_if(std::unique_ptr<expresion>&& cond, std::vector<std::unique_ptr<sentencia>>&& si, std::vector<std::unique_ptr<sentencia>>&& no)
   : condicion(std::move(cond)), parte_si(std::move(si)), parte_no(std::move(no)) {
   }
};

struct sentencia_return : sentencia{
   std::unique_ptr<expresion> ex;

   sentencia_return(std::unique_ptr<expresion>&& e)
   : ex(std::move(e)) {
   }
};

std::unique_ptr<sentencia> parsea_sentencia(const token_anotada*&);

std::unique_ptr<sentencia> parsea_declaracion(const token_anotada*& iter){
   /*auto tipo = espera(iter, es_tipo);
   auto nombre = espera(iter, IDENTIFICADOR);
   espera(iter, ASIGNACION);
   auto ex = parsea_expresion(iter);
   espera(iter, PUNTO_Y_COMA);
   return std::make_unique<sentencia_declaracion>(tipo, nombre, std::move(ex));*/
}

std::unique_ptr<sentencia> parsea_if(const token_anotada*& iter){
   /*espera(iter, IF);
   std::unique_ptr<expresion> cond = parsea_expresion(iter);
   espera(iter, LLAVE_I);
   std::vector<std::unique_ptr<sentencia>> si;
   while(iter->tipo != LLAVE_D){
      si.push_back(parsea_sentencia(iter));
   }
   espera(iter, LLAVE_D);
   std::vector<std::unique_ptr<sentencia>> no;
   if(iter->tipo == ELSE){
      ++iter;
      if(iter->tipo == IF){
         no.push_back(parsea_if(iter));
      }else{
         espera(iter, LLAVE_I);
         while(iter->tipo != LLAVE_D){
            no.push_back(parsea_sentencia(iter));
         }
         espera(iter, LLAVE_D);
      }
   }
   return std::make_unique<sentencia_if>(std::move(cond), std::move(si), std::move(no));*/
}

std::unique_ptr<sentencia> parsea_return(const token_anotada*& iter){
   /*espera(iter, RETURN);
   auto ex = parsea_expresion(iter);
   espera(iter, PUNTO_Y_COMA);
   return std::make_unique<sentencia_return>(std::move(ex));*/
}

std::unique_ptr<sentencia> parsea_sentencia(const token_anotada*& iter){
   /*if(es_tipo(iter->tipo)){
      return parsea_declaracion(iter);
   }else if(iter->tipo == IF){
      return parsea_if(iter);
   }else{
      return parsea_return(iter);
   }*/
}

#endif
