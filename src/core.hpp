#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include <cwctype>
#include <optional>
#include <chrono>
namespace zzen {
using Clock=std::chrono::steady_clock;
inline std::wstring norm(std::wstring s){
 // Windows starts in the C locale: towlower alone leaves accented capitals unchanged.
 for(auto& c:s){switch(c){
  case L'Á':case L'á':c=L'a';break;case L'É':case L'é':c=L'e';break;
  case L'Í':case L'í':c=L'i';break;case L'Ó':case L'ó':c=L'o';break;
  case L'Ú':case L'ú':case L'Ü':case L'ü':c=L'u';break;
  case L'Ñ':c=L'ñ';break;default:c=std::towlower(c);break;
 }}
 for(auto pair:{std::pair{L"vla lactea",L"via lactea"},std::pair{L"flota lxar",L"flota ixar"}}){size_t p=0;while((p=s.find(pair.first,p))!=s.npos){s.replace(p,wcslen(pair.first),pair.second);p+=wcslen(pair.second);}}return s;
}
inline bool has(const std::wstring& s,const std::wstring& part){return norm(s).find(norm(part))!=std::wstring::npos;}
inline std::wstring trim(std::wstring s){auto a=s.find_first_not_of(L" \r\n\t");return a==s.npos?L"":s.substr(a,s.find_last_not_of(L" \r\n\t")-a+1);}
struct Line{std::wstring text;double x=0,y=0,w=0,h=0;};
struct Reading{std::wstring text,bottom;std::vector<Line> lines;};
inline const Line* find(const Reading& r,const std::wstring& text,double minY=0,double maxY=1,double minX=0,double maxX=1){for(auto& l:r.lines)if(l.y>=minY&&l.y<maxY&&l.x>=minX&&l.x<maxX&&has(l.text,text))return &l;return nullptr;}
enum class Screen{Unknown,Formation,Rematch,FinalScore,Rewards,Experience,TeamList,Difficulty,RouteMenu,RouteMap,PreMatch,Presentation,Lineup,Kickoff,Playing,HalfTime,Station,DropRates};
inline Screen classify(const Reading& r,bool postMatch=false){
 if(find(r,L"tasa",0,.2,.5)&&(has(r.text,L"grupo")||has(r.text,L"comun")))return Screen::DropRates;
 if((has(r.bottom,L"termi")&&has(r.bottom,L"edici"))||find(r,L"terminar edici",.85))return Screen::Formation;
 if(has(r.text,L"jugar revancha"))return Screen::Rematch;
 if((has(r.text,L"resultados")&&has(r.text,L"fin del partido"))||find(r,L"fin del partido",.75))return Screen::FinalScore;
 if(has(r.text,L"objetos")&&has(r.text,L"espiritus"))return Screen::Rewards;
 if(has(r.text,L"resultados"))return Screen::Experience;
 if(has(r.text,L"elige un equipo al que"))return Screen::TeamList;
 if(find(r,L"guardar",.68,1,0,.45)&&has(r.text,L"batalla"))return Screen::Difficulty;
 if(find(r,L"ayuda",0,.6,0,.4)&&find(r,L"opciones",0,.6,0,.4))return Screen::RouteMenu;
 if(has(r.text,L"lista de equipos"))return Screen::RouteMap;
 if(find(r,L"empezar",.75)||has(r.text,L"empezar partido?"))return Screen::PreMatch;
 if(has(r.text,L"pulsa el bot"))return Screen::Presentation;
 if(find(r,L"saque de centro",.7))return Screen::Kickoff;
 // A blinking result heading must not turn a completed match into a pre-match lineup.
 if(find(r,L"siguiente",.86))return postMatch?Screen::Experience:Screen::Lineup;
 if(find(r,L"comenzar",.65)||find(r,L"reanudar",.65))return Screen::HalfTime;
 if(find(r,L"modo manual",.7)||find(r,L"modo comandante",.7))return Screen::Playing;
 if(has(r.text,L"elige una ruta")||has(r.text,L"selecciona una ruta"))return Screen::Station;
 return Screen::Unknown;
}
inline std::wstring label(Screen s){switch(s){case Screen::DropRates:return L"Tasas de aparición";case Screen::Formation:return L"Alineación · Terminar edición";case Screen::Rematch:return L"Revancha";case Screen::FinalScore:return L"Marcador final";case Screen::Rewards:return L"Recompensas";case Screen::Experience:return L"Experiencia / resultados";case Screen::TeamList:return L"Lista de equipos";case Screen::Difficulty:return L"Dificultad";case Screen::RouteMenu:return L"Menú de ruta";case Screen::RouteMap:return L"Mapa de Crónica";case Screen::PreMatch:return L"Preparación del partido";case Screen::Presentation:return L"Presentación";case Screen::Lineup:return L"Presentación de equipos";case Screen::Kickoff:return L"Saque de centro";case Screen::Playing:return L"Partido en curso";case Screen::HalfTime:return L"Descanso";case Screen::Station:return L"Estación / rutas";default:return L"Pantalla no reconocida";}}
inline bool isMatchScreen(Screen s){return s==Screen::Formation||s==Screen::Presentation||s==Screen::Lineup||s==Screen::Kickoff||s==Screen::Playing||s==Screen::HalfTime;}
inline bool repeat(int finished,int limit,bool infinite){return infinite||finished<limit;}
inline std::wstring result(int ours,int theirs){return ours>theirs?L"Victoria":ours<theirs?L"Derrota":L"Empate";}
inline std::vector<Line> teams(const Reading& r){std::vector<Line> out;for(auto l:r.lines){l.text=trim(l.text);if(norm(l.text)==L"via lactea")l.text=L"Vía Láctea";if(norm(l.text)==L"flota ixar")l.text=L"Flota Ixar";if(l.x>.057&&l.x<.38&&l.y>.26&&l.y<.83&&l.text.size()>2&&!has(l.text,L"Nv.")&&!has(l.text,L"completado"))out.push_back(l);}std::sort(out.begin(),out.end(),[](auto&a,auto&b){return a.y<b.y;});return out;}

// IDs are stable across editions and saved profiles. Gate battles use separate IDs.
inline int difficultyId(std::wstring text){
 text=norm(text);if(!has(text,L"batalla"))return -1;
 if(has(text,L"legendaria"))return 5;
 if(has(text,L"puerta"))return text.find(L'+')!=text.npos?7:6;
 if(has(text,L"heroica"))return text.find(L'+')!=text.npos?4:3;
 if(has(text,L"inusuales"))return 2;if(has(text,L"desbloqueo"))return 1;
 if(has(text,L"cronica"))return 0;return -1;
}
inline const Line* difficultyLine(const Reading& r,int id){
 for(auto& l:r.lines)if(l.x<.4&&l.y>.38&&l.y<.86&&difficultyId(l.text)==id)return &l;
 return nullptr;
}

struct Config{std::wstring team=L"Royal Academy",route=L"Inazuma Eleven";int difficulty=3,count=1,occurrence=1,language=0;bool infinite=false;};
struct Record{std::wstring id,date,team,route,difficulty,outcome=L"Pendiente",score,rewards,source=L"OCR";};
}
