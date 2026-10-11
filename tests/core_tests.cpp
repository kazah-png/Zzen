#include "../src/core.hpp"
#include <cassert>
#include <iostream>
using namespace zzen;
int main(){
 assert(repeat(0,1,false));
 assert(!repeat(1,1,false));
 assert(!repeat(5,2,false));
 assert(repeat(100000,1,true));
 assert(result(4,0)==L"Victoria");
 assert(result(0,4)==L"Derrota");
 assert(result(2,2)==L"Empate");
 Reading r;r.bottom=L"Alt Termioer edicióhgxt Aceptar";
 assert(classify(r)==Screen::Formation);
 r={};r.text=L"DERROTA";
 assert(classify(r)!=Screen::FinalScore);
 r.text=L"RESULTADOS FIN DEL PARTIDO";
 assert(classify(r)==Screen::FinalScore);
 r.text=L"RESULTADOS Objetos Espíritus Jugar revancha";
 assert(classify(r)==Screen::Rematch);
 r={};r.text=L"Elige un equipo al que enfrentarte.";
 assert(classify(r)==Screen::TeamList);
 std::cout<<"OK: limites, resultados y clasificacion de pantallas\n";
}
