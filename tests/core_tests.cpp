#include "../src/core.hpp"
#include <cassert>
#include <iostream>
#include <clocale>
using namespace zzen;
int main(){
 std::setlocale(LC_ALL,"C");
 assert(norm(L"ÁÉÍÓÚÜÑ")==L"aeiouuñ");
 assert(has(L"ESPÍRITUS",L"espiritus"));
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
 r={};r.text=L"Fin del partido";r.lines.push_back({L"Fin del partido",.7,.92,.2,.04});
 assert(classify(r)==Screen::FinalScore);
 r.lines[0].y=.3;assert(classify(r)==Screen::Unknown);
 r.text=L"RESULTADOS Objetos Espíritus Jugar revancha";
 assert(classify(r)==Screen::Rematch);
 r={};r.text=L"RESULTADOS.-\nOBJETOS\nESPÍRITUS\n0 Siguiente";
 r.lines.push_back({L"0 Siguiente",.8,.91,.12,.04});
 assert(classify(r)==Screen::Rewards);
 r.text=L"OBJETOS\nESPÍRITUS\n0 Siguiente";
 assert(classify(r)==Screen::Rewards);
 r.text=L"0 Siguiente";
 assert(classify(r)==Screen::Lineup);
 assert(classify(r,true)==Screen::Experience);
 r={};r.text=L"Elige un equipo al que enfrentarte.";
 assert(classify(r)==Screen::TeamList);
 std::cout<<"OK: limites, resultados y clasificacion de pantallas\n";
}
