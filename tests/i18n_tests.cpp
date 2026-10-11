#include "../src/i18n.hpp"
#include "../src/core.hpp"
#include <cassert>
#include <iostream>
int main(){
 using namespace zzen;
 assert(i18n::catalog().size()>80);
 for(auto& [key,translations]:i18n::catalog())for(auto& text:translations)assert(!text.empty());
 for(int lang=0;lang<6;++lang){
  i18n::language=lang;
  assert(!i18n::help().empty());
  assert(!i18n::tr(L"El juego debe estar en español").empty());
  assert(i18n::tr(L"Royal Academy")==L"Royal Academy");
  assert(classify({L"Resultados Objetos Espíritus"})==Screen::Rewards);
 }
 i18n::language=1;
 assert(i18n::tr(L"Listo")==L"Ready");
 assert(i18n::tr(L"Jugando · Royal Academy")==L"Playing · Royal Academy");
 std::cout<<"OK: six languages, complete resources, stable Spanish recognition\n";
}
