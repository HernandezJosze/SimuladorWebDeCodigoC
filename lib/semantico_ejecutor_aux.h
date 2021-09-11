//
// Created by :)-|-< on 8/25/2021.
//

#ifndef SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_AUX_H
#define SIMULADORWEBDECODIGOC_SEMANTICO_EJECUTOR_AUX_H

#include <cstdint>
#include <type_traits>
#include <tuple>

namespace impl {
   template<auto N>
   struct constexpr_integer {
      constexpr operator decltype(N)( ) const {
         return N;
      }
   };

   template<auto I, auto F, std::intmax_t S = (I <= F ? +1 : -1), typename T>
   constexpr void unrolled_for(T&& f) {
      if constexpr(I == F) {
         return;
      } else if constexpr(std::is_same_v<void, decltype(f(constexpr_integer<I>( )))>) {
         f(constexpr_integer<I>( ));
         unrolled_for<I + S, F, S>(f);
      } else if constexpr(std::is_same_v<bool, decltype(f(constexpr_integer<I>( )))>) {
         if (f(constexpr_integer<I>( ))) {
            unrolled_for<I + S, F, S>(f);
         }
      }
   }
}

template<typename... TS>
bool valida_ejecuta(auto p, auto f) {
   bool res = false;
   impl::unrolled_for<0, sizeof...(TS)>([&](auto i) {
      if (auto checar = dynamic_cast<std::tuple_element_t<i, std::tuple<TS...>>>(p); checar != nullptr) {
         f(checar), res = true;
      }
   });
   return res;
}

template<typename... TS>
bool valida(auto p) {
   return valida_ejecuta<TS...>(p, [](auto checado) {
      return;
   });
}

bool es_operador_asignacion(token t){
   return t == ASIGNACION || t == MAS_IGUAL || t == MENOS_IGUAL || t == MULTIPLICA_IGUAL || t == DIVIDE_IGUAL || t == MODULO_IGUAL;
}

#endif
