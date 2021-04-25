//
// Created by Hernandez on 4/18/2021.
//

#ifndef SIMULADORWEBDECODIGOC_PARSER_H
#define SIMULADORWEBDECODIGOC_PARSER_H

#include "lexer.h"
#include "parser_sentencia.h"

std::vector<std::unique_ptr<sentencia>> parsea(const token_anotada* iter){
   std::vector<std::unique_ptr<sentencia>> res;
   while(iter->tipo != END_FILE){
      res.push_back(parsea_sentencia(iter));
   }
   return res;
}

#endif
