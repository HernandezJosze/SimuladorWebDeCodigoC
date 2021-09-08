//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_SENTENCIA_H
#define SIMULADORWEBDECODIGOC_PARSER_SENTENCIA_H

#include "parser_aux.h"
#include "parser_expresion.h"

struct sentencia{
   const token_anotada* pos;

   sentencia(const token_anotada* p)
   : pos(p){
   }
   virtual ~sentencia() = 0;
};
sentencia::~sentencia(){}

struct sentencia_expresiones : sentencia{
   std::vector<std::unique_ptr<expresion>> ex;

   sentencia_expresiones(const token_anotada* p, std::vector<std::unique_ptr<expresion>>&& e)
   : sentencia(p), ex(std::move(e)) {
   }
};
struct sentencia_declaraciones : sentencia{
   struct subdeclaracion {
      const token_anotada* nombre;
      std::pair<std::unique_ptr<expresion>, bool> arreglo;
      std::unique_ptr<expresion> inicializador;
   };
   const token_anotada* tipo;
   std::vector<subdeclaracion> subdeclaraciones;

   sentencia_declaraciones(const token_anotada* p, const token_anotada* t, std::vector<subdeclaracion>&& d)
   : sentencia(p), tipo(t), subdeclaraciones(std::move(d)) {
   }
};
struct sentencia_if : sentencia{
   std::vector<std::unique_ptr<expresion>> condicion;
   std::vector<std::unique_ptr<sentencia>> parte_si;
   std::vector<std::unique_ptr<sentencia>> parte_no;

   sentencia_if(const token_anotada* p, std::vector<std::unique_ptr<expresion>>&& cond, std::vector<std::unique_ptr<sentencia>>&& si, std::vector<std::unique_ptr<sentencia>>&& no)
   : sentencia(p), condicion(std::move(cond)), parte_si(std::move(si)), parte_no(std::move(no)) {
   }
};
struct sentencia_for : sentencia{
   std::unique_ptr<sentencia> inicializacion;   // puede ser sentencia_expresiones o sentencia_declaraciones
   std::vector<std::unique_ptr<expresion>> condicion;
   std::vector<std::unique_ptr<expresion>> actualizacion;
   std::vector<std::unique_ptr<sentencia>> sentencias;

   sentencia_for(const token_anotada* p, std::unique_ptr<sentencia>&& ini, std::vector<std::unique_ptr<expresion>>&& cond, std::vector<std::unique_ptr<expresion>>&& act, std::vector<std::unique_ptr<sentencia>>&& ejecutar)
   : sentencia(p), inicializacion(std::move(ini)), condicion(std::move(cond)), actualizacion(std::move(act)), sentencias(std::move(ejecutar)) {
   }
};
struct sentencia_do : sentencia{
   std::vector<std::unique_ptr<expresion>> condicion;
   std::vector<std::unique_ptr<sentencia>> sentencias;

   sentencia_do(const token_anotada* p, std::vector<std::unique_ptr<expresion>>&& cond, std::vector<std::unique_ptr<sentencia>>&& s)
   : sentencia(p), condicion(std::move(cond)), sentencias(std::move(s)) {
   }
};
struct sentencia_while : sentencia{
   std::vector<std::unique_ptr<expresion>> condicion;
   std::vector<std::unique_ptr<sentencia>> sentencias;

   sentencia_while(const token_anotada* p, std::vector<std::unique_ptr<expresion>>&& cond, std::vector<std::unique_ptr<sentencia>>&& s)
   : sentencia(p), condicion(std::move(cond)), sentencias(std::move(s)) {
   }
};
struct sentencia_break : sentencia{

   sentencia_break(const token_anotada* p)
   : sentencia(p){
   }
};
struct sentencia_continue : sentencia{

   sentencia_continue(const token_anotada* p)
   : sentencia(p){
   }
};


std::unique_ptr<sentencia> parsea_sentencia(const token_anotada*&);

std::unique_ptr<sentencia> parsea_expresiones(const token_anotada*& iter) {
   auto pos = iter;
   return std::make_unique<sentencia_expresiones>(pos, parsea_lista_expresiones(iter, true));
}
std::unique_ptr<sentencia> parsea_declaraciones(const token_anotada*& iter) {
   auto parsea_subdeclaracion = [&]{
      auto nombre = espera(iter, IDENTIFICADOR, "Se esperaba un identificador");
      std::pair<std::unique_ptr<expresion>, bool> arreglo = { };
      if(iter->tipo == CORCHETE_I) {
         espera(iter, CORCHETE_I, "Se espera un [");
         arreglo = {(iter->tipo != CORCHETE_D ? parsea_expresion(iter) : nullptr), true};
         espera(iter, CORCHETE_D, "Se espera un ]");
      }
      std::unique_ptr<expresion> ex;
      if(iter->tipo == ASIGNACION){
         espera(iter, ASIGNACION, "Se esperaba una asignacion");
         ex = parsea_expresion(iter);
      }
      return sentencia_declaraciones::subdeclaracion(nombre, std::move(arreglo), std::move(ex));
   };

   auto pos = iter, tipo = espera(iter, es_tipo, "Se esperaba un tipo");
   std::vector<sentencia_declaraciones::subdeclaracion> d;
   d.push_back(parsea_subdeclaracion( ));
   while (iter->tipo == COMA) {
      ++iter, d.push_back(parsea_subdeclaracion( ));
   }
   return std::make_unique<sentencia_declaraciones>(pos, tipo, std::move(d));
}

std::unique_ptr<sentencia> parsea_if(const token_anotada*& iter){
   auto pos = iter;
   espera(iter, IF, "Se esperaba un if");
   espera(iter, PARENTESIS_I, "Se esperaba un (");
   std::vector<std::unique_ptr<expresion>> cond = parsea_lista_expresiones(iter, false);
   espera(iter, PARENTESIS_D, "Se esperaba un )");
   espera(iter, LLAVE_I, "Se esperaba un {");
   std::vector<std::unique_ptr<sentencia>> si;
   while(iter->tipo != LLAVE_D){
      si.push_back(parsea_sentencia(iter));
   }
   espera(iter, LLAVE_D, "Se esperaba un }");
   std::vector<std::unique_ptr<sentencia>> no;
   if(iter->tipo == ELSE){
      ++iter;
      if(iter->tipo == IF){
         no.push_back(parsea_if(iter));
      }else{
         espera(iter, LLAVE_I, "Se esperaba un {");
         while(iter->tipo != LLAVE_D){
            no.push_back(parsea_sentencia(iter));
         }
         espera(iter, LLAVE_D, "Se esperaba un }");
      }
   }
   return std::make_unique<sentencia_if>(pos, std::move(cond), std::move(si), std::move(no));
}
std::unique_ptr<sentencia> parsea_for(const token_anotada*& iter){
   auto pos = iter;
   espera(iter, FOR, "Se esperaba un for");
   espera(iter, PARENTESIS_I, "Se esperaba un (");

   std::unique_ptr<sentencia> ini = (es_tipo(iter->tipo) ? std::unique_ptr<sentencia>(parsea_declaraciones(iter)) : std::unique_ptr<sentencia>(parsea_expresiones(iter)));
   espera(iter, PUNTO_Y_COMA, "Se esperaba un ;");
   std::vector<std::unique_ptr<expresion>> cond = parsea_lista_expresiones(iter, true);
   espera(iter, PUNTO_Y_COMA, "Se esperaba un ;");
   std::vector<std::unique_ptr<expresion>> act = parsea_lista_expresiones(iter, true);

   espera(iter, PARENTESIS_D, "Se esperaba un )");
   espera(iter, LLAVE_I, "Se esperaba un {");
   std::vector<std::unique_ptr<sentencia>> ejecutar;
   while(iter->tipo != LLAVE_D){
      ejecutar.push_back(parsea_sentencia(iter));
   }
   espera(iter, LLAVE_D, "Se esperaba un }");
   return std::make_unique<sentencia_for>(pos, std::move(ini), std::move(cond), std::move(act), std::move(ejecutar));
}
std::unique_ptr<sentencia> parsea_do(const token_anotada*& iter){
   auto pos = iter;
   espera(iter, DO, "Se esperaba un do");
   espera(iter, LLAVE_I, "Se esperaba un {");
   std::vector<std::unique_ptr<sentencia>> sentencias;
   while(iter->tipo != LLAVE_D){
      sentencias.push_back(parsea_sentencia(iter));
   }
   espera(iter, LLAVE_D, "Se esperaba una }");
   espera(iter, WHILE, "Se esperaba un while");
   espera(iter, PARENTESIS_I, "Se esperaba un (");
   std::vector<std::unique_ptr<expresion>> cond = parsea_lista_expresiones(iter, false);
   espera(iter, PARENTESIS_D, "Se esperaba un )");
   espera(iter, PUNTO_Y_COMA, "Se esperaba ;");
   return std::make_unique<sentencia_do>(pos, std::move(cond), std::move(sentencias));
}
std::unique_ptr<sentencia> parsea_while(const token_anotada*& iter){
   auto pos = iter;
   espera(iter, WHILE, "Se esperaba un while");
   espera(iter, PARENTESIS_I, "Se esperaba un (");
   std::vector<std::unique_ptr<expresion>> cond = parsea_lista_expresiones(iter, false);
   espera(iter, PARENTESIS_D, "Se esperaba un )");
   espera(iter, LLAVE_I, "Se esperaba un {");
   std::vector<std::unique_ptr<sentencia>> sentencias;
   while(iter->tipo != LLAVE_D){
      sentencias.push_back(parsea_sentencia(iter));
   }
   espera(iter, LLAVE_D, "Se esperaba una }");
   return std::make_unique<sentencia_while>(pos, std::move(cond), std::move(sentencias));
}
std::unique_ptr<sentencia> parsea_break(const token_anotada*& iter){
   auto pos = iter;
   espera(iter, BREAK, "Se esperaba break");
   espera(iter, PUNTO_Y_COMA, "Se esperaba un ;");
   return std::make_unique<sentencia_break>(pos);
}
std::unique_ptr<sentencia> parsea_continue(const token_anotada*& iter){
   auto pos = iter;
   espera(iter, CONTINUE, "Se esperaba continue");
   espera(iter, PUNTO_Y_COMA, "Se esperaba un ;");
   return std::make_unique<sentencia_continue>(pos);
}

std::unique_ptr<sentencia> parsea_sentencia(const token_anotada*& iter){
   if(iter->tipo == IF){
      return parsea_if(iter);
   }else if(iter->tipo == FOR){
      return parsea_for(iter);
   }else if(iter->tipo == DO){
      return parsea_do(iter);
   }else if(iter->tipo == WHILE){
      return parsea_while(iter);
   }else if(iter->tipo == BREAK){
      return parsea_break(iter);
   }else if(iter->tipo == CONTINUE){
      return parsea_continue(iter);
   }else if(es_tipo(iter->tipo)) {
      auto stmt = parsea_declaraciones(iter);
      espera(iter, PUNTO_Y_COMA, "Se esperaba ;");
      return stmt;
   } else {
      auto stmt = parsea_expresiones(iter);
      espera(iter, PUNTO_Y_COMA, "Se esperaba ;");
      return stmt;
   }
}

#endif
