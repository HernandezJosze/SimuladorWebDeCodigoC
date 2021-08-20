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

struct sentencia_expresion : sentencia{
   std::unique_ptr<expresion> ex;

   sentencia_expresion(std::unique_ptr<expresion>&& e)
   : ex(std::move(e)) {
   }
};

/*struct sentencia_declaracion : sentencia{
   const token_anotada* tipo;
   std::vector<const token_anotada*> nombre;
   std::vector<std::unique_ptr<expresion>> tamanio;
   std::vector<std::unique_ptr<expresion>> inicializador;

   sentencia_declaracion(const token_anotada* t, std::vector<const token_anotada*> n, std::vector<std::unique_ptr<expresion>>&& tam, std::vector<std::unique_ptr<expresion>>&& i)
   : tipo(t), nombre(n), tamanio(std::move(tam)), inicializador(std::move(i)) {
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
struct sentencia_for : sentencia{
   std::unique_ptr<sentencia> inicializacion;
   std::unique_ptr<expresion> condicion;
   std::vector<std::unique_ptr<expresion>> incrementos;
   std::vector<std::unique_ptr<sentencia>> aEjecutar;

   sentencia_for(std::unique_ptr<sentencia>&& ini, std::unique_ptr<expresion>&& cond, std::vector<std::unique_ptr<expresion>>&& inc, std::vector<std::unique_ptr<sentencia>>&& ejecutar)
   : inicializacion(std::move(ini)), condicion(std::move(cond)), incrementos(std::move(inc)), aEjecutar(std::move(ejecutar)) {
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
   auto tipo = espera(iter, es_tipo, "Se esperaba una declaracion");

   std::vector<const token_anotada*> nombre;
   std::vector<std::unique_ptr<expresion>> vtam, vex;

   bool _continue = true;
   do{
      nombre.push_back(espera(iter, IDENTIFICADOR, "Se esperaba un identificador"));
      std::unique_ptr<expresion> tam = nullptr, ex = nullptr;

      if(iter->tipo == CORCHETE_I){
         espera(iter, CORCHETE_I, "Se espera un [");
         tam = (parsea_expresion(iter));
         espera(iter, CORCHETE_D, "Se espera un ]");
      }

      if(iter->tipo == ASIGNACION){
           espera(iter, ASIGNACION, "Se esperaba una asignacion");
           ex = (parsea_expresion(iter));
      }

      vtam.push_back(std::move(tam));
      vex.push_back(std::move(ex));

      if(iter->tipo == COMA){
         espera(iter, COMA, "Se esperaba una ,");
      }else{
         _continue = false;
      }
   }while(_continue);

    espera(iter, PUNTO_Y_COMA, "Se esperaba un ;");
    return std::make_unique<sentencia_declaracion>(tipo, nombre, std::move(vtam), std::move(vex));
}

std::unique_ptr<sentencia> parsea_if(const token_anotada*& iter){
   espera(iter, IF, "Se esperaba una if");
   std::unique_ptr<expresion> cond = parsea_expresion(iter);
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
   return std::make_unique<sentencia_if>(std::move(cond), std::move(si), std::move(no));
}

std::unique_ptr<sentencia> parsea_return(const token_anotada*& iter){
   espera(iter, RETURN, "Se esperaba un return");
   auto ex = parsea_expresion(iter);
   espera(iter, PUNTO_Y_COMA, "Se esperaba un ;");
   return std::make_unique<sentencia_return>(std::move(ex));
}
std::unique_ptr<sentencia> parsea_continue(const token_anotada*& iter){
    espera(iter, CONTINUE, "Se esperaba un continue");

    espera(iter, PUNTO_Y_COMA, "Se esperaba un ;contunue");
    //return std::make_unique<sentencia_return>(std::move(ex));
}
std::unique_ptr<sentencia> parsea_for(const token_anotada*& iter){
   espera(iter, FOR, "Se esperaba un for");
   espera(iter, PARENTESIS_I, "Se esperaba un (");
   std::unique_ptr<sentencia> ini = parsea_sentencia(iter);
   std::unique_ptr<expresion> cond = parsea_expresion(iter);

   espera(iter, PUNTO_Y_COMA, "Se esperaba un ;for");

   std::vector<std::unique_ptr<expresion>> inc;
   while(iter->tipo != PARENTESIS_D){
       inc.push_back(parsea_expresion(iter));
       if(iter->tipo == COMA){
           espera(iter, COMA, "Se esperaba una ,");
       }
   }
   espera(iter, PARENTESIS_D, "Se esperaba un )");
   espera(iter, LLAVE_I, "Se esperaba un {");
   std::vector<std::unique_ptr<sentencia>> ejecutar;
   while(iter->tipo != LLAVE_D){
      ejecutar.push_back(parsea_sentencia(iter));
   }
   espera(iter, LLAVE_D, "Se esperaba un }");
   return std::make_unique<sentencia_for>(std::move(ini), std::move(cond), std::move(inc), std::move(ejecutar));
}*/

std::unique_ptr<sentencia> parsea_sentencia(const token_anotada*& iter){
   /*if(es_tipo(iter->tipo)){
      return parsea_declaracion(iter);
   }else if(iter->tipo == IF){
      return parsea_if(iter);
   }else if(iter->tipo == FOR){
       return parsea_for(iter);
   }else if(iter->tipo == DO){

   }else if(iter->tipo == WHILE){

   }else if(iter->tipo == CONTINUE){
      return parsea_continue(iter);
   }else if (iter->tipo == RETURN) {
      return parsea_return(iter);
   }else {*/
      auto ex = parsea_expresion(iter);
      espera(iter, PUNTO_Y_COMA, "Se esperaba ;");
      return std::make_unique<sentencia_expresion>(std::move(ex));
    //}
}

#endif
