#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <commdlg.h>
#include <dwmapi.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>
#include <map>
#include "vision.hpp"
#include "storage.hpp"
using namespace zzen;
static HWND mainWindow;static std::unique_ptr<Store> store;static std::mutex dataMutex;static std::atomic<bool> quitting=false,active=false,paused=false;static std::atomic<int> requestMode=0;static HWND gameWindow=nullptr;
struct DebugState{Image image;Reading reading;Screen screen=Screen::Unknown;std::wstring action=L"Sin acciones",status=L"Listo",log;double x=-1,y=-1;int elapsedMs=0,finished=0;bool foreground=false;};static DebugState debug;
static std::wstring exceptionText(){try{throw;}catch(winrt::hresult_error const&e){return e.message().c_str();}catch(std::exception const&e){return winrt::to_hstring(e.what()).c_str();}catch(...){return L"Error desconocido";}}
static void notify(){PostMessageW(mainWindow,WM_APP+1,0,0);}
static void logText(const std::wstring& text){std::lock_guard lock(dataMutex);std::wstring line=stamp()+L"  "+text+L"\r\n";debug.log+=line;if(debug.log.size()>70000)debug.log.erase(0,20000);std::ofstream f(store->root/L"data"/L"activity.log",std::ios::app|std::ios::binary);f<<winrt::to_string(line);notify();}
static void status(const std::wstring& text){bool changed;{std::lock_guard lock(dataMutex);changed=debug.status!=text;debug.status=text;}if(changed)logText(text);else notify();}
static HWND game(){HWND found=nullptr;EnumWindows([](HWND h,LPARAM p)->BOOL{wchar_t text[512];GetWindowTextW(h,text,512);if(IsWindowVisible(h)&&std::wstring(text)==L"INAZUMA ELEVEN: Victory Road")*reinterpret_cast<HWND*>(p)=h;return TRUE;},reinterpret_cast<LPARAM>(&found));if(!found)throw std::runtime_error("Abre INAZUMA ELEVEN: Victory Road.");return found;}
#ifdef ZZEN_ENGINE_TEST
static std::vector<std::wstring> testInputs;
static void key(WORD vk){if(!active||paused||quitting)throw std::runtime_error("paused");testInputs.push_back(L"key:"+std::to_wstring(vk));}
static void click(double,double){if(!active||paused||quitting)throw std::runtime_error("paused");testInputs.push_back(L"click");}
static void scroll(int amount){if(!active||paused||quitting)throw std::runtime_error("paused");testInputs.push_back(L"scroll:"+std::to_wstring(amount));}
static void ensureInput(){}
#else
static void ensureInput(){if(!active||paused||quitting)throw std::runtime_error("Sesión pausada o detenida");if(!gameWindow||GetForegroundWindow()!=gameWindow||IsIconic(gameWindow))throw std::runtime_error("El juego perdió el foco. Pulsa Reanudar.");}
static void send(INPUT i){if(SendInput(1,&i,sizeof i)!=1)throw std::runtime_error("Windows no ha aceptado la entrada");}
static void key(WORD vk){ensureInput();INPUT i{};i.type=INPUT_KEYBOARD;i.ki.wScan=(WORD)MapVirtualKeyW(vk,MAPVK_VK_TO_VSC);i.ki.dwFlags=KEYEVENTF_SCANCODE;send(i);std::this_thread::sleep_for(std::chrono::milliseconds(110));i.ki.dwFlags|=KEYEVENTF_KEYUP;send(i);}
static void click(double x,double y){ensureInput();RECT r;GetClientRect(gameWindow,&r);POINT p{};ClientToScreen(gameWindow,&p);SetCursorPos(p.x+int(x*r.right),p.y+int(y*r.bottom));INPUT i{};i.type=INPUT_MOUSE;i.mi.dwFlags=MOUSEEVENTF_LEFTDOWN;send(i);std::this_thread::sleep_for(std::chrono::milliseconds(110));i.mi.dwFlags=MOUSEEVENTF_LEFTUP;send(i);if(GetForegroundWindow()==gameWindow)SetCursorPos(p.x+r.right-8,p.y+8);}
static void scroll(int amount){ensureInput();RECT r;GetClientRect(gameWindow,&r);POINT p{};ClientToScreen(gameWindow,&p);SetCursorPos(p.x+r.right/4,p.y+r.bottom/2);INPUT i{};i.type=INPUT_MOUSE;i.mi.dwFlags=MOUSEEVENTF_WHEEL;i.mi.mouseData=DWORD(amount);send(i);}
#endif
static void stop(const std::wstring& reason){active=false;paused=false;requestMode=0;status(reason);PostMessageW(mainWindow,WM_APP+2,0,0);}
static void evidence(const Image& im,const Reading&r,const std::wstring& id,const std::wstring& kind){if(kind!=L"resultado-pendiente")return;auto dir=store->root/L"captures"/id;fs::create_directories(dir);auto p=dir/(stamp(true)+L"-"+kind);im.save(p.wstring()+L".png");writeText(p.wstring()+L".txt",r.text);}
struct Engine{
 Config cfg;int finished=0,mode=1,scrolls=0,stalePages=0,topPass=0;bool inMatch=false,recorded=false,commander=false,awaitingNew=false,scoreTimedOut=false;std::wstring id,lastPage,lastReward;size_t current=0;Clock::time_point entered=Clock::now(),matchStart=Clock::now(),lastAction=Clock::now()-std::chrono::seconds(10),newMatchAt=Clock::now();Screen previous=Screen::Unknown;std::vector<std::wstring> scanned;
 void reset(int m){std::lock_guard lock(dataMutex);cfg=store->config;mode=m;if(m==1)store->history.clear();finished=0;inMatch=false;recorded=false;commander=false;awaitingNew=false;scoreTimedOut=false;current=0;id.clear();scrolls=stalePages=topPass=0;scanned.clear();lastPage.clear();lastReward.clear();previous=Screen::Unknown;entered=matchStart=Clock::now();lastAction=Clock::now()-std::chrono::seconds(10);debug.finished=0;categoryIndex=categoryChanges=0;tabReady=false;category.clear();lastSelection.clear();completedCategories.clear();}
 void begin(){if(inMatch)return;inMatch=true;recorded=false;scoreTimedOut=false;id=stamp(true);matchStart=Clock::now();lastReward.clear();}
 bool action(std::wstring text,std::function<void()> fn,double x=-1,double y=-1){if(Clock::now()-lastAction<std::chrono::milliseconds(1600))return false;{std::lock_guard lock(dataMutex);debug.action=text;debug.x=x;debug.y=y;}logText(L"ACCIÓN · "+text);fn();lastAction=Clock::now();return true;}
 bool tap(const Line* line,std::wstring description){if(!line)return false;double x=line->x+line->w/2,y=line->y+line->h/2;return action(description,[=]{click(x,y);},x,y);}
 void addRecord(const Image& im,const Reading&r,std::optional<std::pair<int,int>> score){begin();{std::lock_guard lock(dataMutex);if(!recorded){Record m;m.id=id;m.date=stamp();m.team=cfg.team;m.route=cfg.route;m.difficulty=difficultyName(cfg.difficulty);store->history.push_back(m);current=store->history.size()-1;recorded=true;++finished;debug.finished=finished;}if(score){auto& m=store->history[current];m.score=std::to_wstring(score->first)+L"–"+std::to_wstring(score->second);m.outcome=result(score->first,score->second);m.source=L"Marcador final";}store->saveHistory();}evidence(im,r,id,score?L"marcador":L"resultado-pendiente");notify();}
 template<class VisionReader> void tick(const Image& im,const Reading&r,VisionReader& vision){auto screen=classify(r,recorded&&!awaitingNew);auto now=Clock::now();if(screen!=previous){previous=screen;entered=now;logText(L"PANTALLA · "+label(screen));}if(mode==3){status(L"Debug · Solo observación, sin pulsaciones");return;}if(screen!=Screen::Playing&&screen!=Screen::Unknown&&now-entered>std::chrono::seconds(75))throw std::runtime_error("La pantalla no avanza. Consulta Debug y reanuda tras revisarla.");if(screen==Screen::Unknown&&now-entered>std::chrono::seconds(90))throw std::runtime_error("Pantalla desconocida durante 90 s. Se ha detenido la entrada.");if(inMatch&&now-matchStart>std::chrono::minutes(30))throw std::runtime_error("Partido superior a 30 minutos.");
  if(awaitingNew){if(isMatchScreen(screen)){awaitingNew=false;inMatch=false;begin();commander=false;}else if(now-newMatchAt>std::chrono::seconds(45))throw std::runtime_error("La revancha no ha comenzado.");else return;}
  if(mode==2){scan(im,r,screen);return;}
  if(mode==3){status(L"Debug · Solo observación, sin pulsaciones");return;}
  if(isMatchScreen(screen))begin();
  if(screen==Screen::FinalScore){
   if(!inMatch){status(L"Marcador de un partido anterior: inicia desde una ruta o un partido en curso");return;}
   bool known=false;{std::lock_guard lock(dataMutex);known=recorded&&store->history[current].outcome!=L"Pendiente";}
   if(!known&&!scoreTimedOut){
    auto score=vision.score(im);if(!active||paused)return;
    if(score){addRecord(im,r,score);status(L"Marcador leído: "+std::to_wstring(score->first)+L"–"+std::to_wstring(score->second));return;}
    if(now-entered<std::chrono::seconds(20)){status(L"Esperando a que el marcador sea legible");return;}
    addRecord(im,r,{});scoreTimedOut=true;
    status(L"Marcador ilegible: resultado pendiente guardado; continúa el farmeo");
   }
   tap(find(r,L"fin del partido",.75),L"Aceptar marcador final");return;
  }
  if(screen==Screen::Rewards||screen==Screen::Experience){if(inMatch&&!recorded)addRecord(im,r,{});if(recorded){auto next=find(r,L"siguiente",.75);if(next)tap(next,L"Avanzar resultados");else if(screen==Screen::Rewards)action(L"Continuar resultados",[]{click(.87,.93);},.87,.93);}return;}
  if(screen==Screen::Rematch){
   // Only adopt a missed result when this session already witnessed the match.
   if(!recorded&&inMatch)addRecord(im,r,{});
   if(!recorded){status(L"Pantalla de un partido anterior. Vuelve a la ruta para iniciar una sesión.");return;}
   bool pending;{std::lock_guard lock(dataMutex);pending=store->history[current].outcome==L"Pendiente";}
   if(pending)status(L"Resultado pendiente de revisión; continúa el farmeo");
   if(repeat(finished,cfg.count,cfg.infinite)){
    if(tap(find(r,L"jugar revancha"),L"Jugar revancha")){
     awaitingNew=true;newMatchAt=Clock::now();recorded=false;inMatch=false;id.clear();
    }
   }else if(tap(find(r,L"volver al men"),L"Volver al menú: límite alcanzado")){
    stop(L"Sesión completada · "+std::to_wstring(finished)+L" partidos");
   }
   return;
  }
  if(screen==Screen::Formation){action(L"Terminar edición · botón inferior",[]{click(.235,.925);},.235,.925);return;}
  if(screen==Screen::Presentation){tap(find(r,L"pulsa el bot"),L"Avanzar presentación");return;}
  if(screen==Screen::Lineup){tap(find(r,L"siguiente",.86),L"Avanzar alineación");return;}
  if(screen==Screen::HalfTime){tap(find(r,L"comenzar",.65)?find(r,L"comenzar",.65):find(r,L"reanudar",.65),L"Continuar partido");return;}
  if(screen==Screen::Kickoff){if(find(r,L"modo manual",.7,1,0,.45)||find(r,L"modo comandante",.7,1,.6))commander=true;if(!commander){action(L"Activar Comandante · U",[&]{key('U');commander=true;});}else tap(find(r,L"saque de centro",.7),L"Realizar saque");return;}
  if(screen==Screen::Playing){if(find(r,L"modo manual",.7,1,0,.45)||find(r,L"modo comandante",.7,1,.6))commander=true;else if(find(r,L"modo comandante",.7,1,0,.45))action(L"Activar Comandante · U",[&]{key('U');commander=true;});status(L"Jugando · "+cfg.team);return;}
  if(screen==Screen::RouteMenu){action(L"Cerrar menú de ruta",[]{key(VK_ESCAPE);});return;}
  if(screen==Screen::RouteMap){inMatch=false;action(L"Abrir lista de equipos · V",[]{key('V');});return;}
  if(screen==Screen::TeamList){selectTeam(im,r);return;}
  if(screen==Screen::Difficulty){if(!has(r.text,cfg.team))throw std::runtime_error("El rival de la pantalla no coincide con la selección.");double y=(511+70*cfg.difficulty)/1080.;action(L"Elegir "+difficultyName(cfg.difficulty),[=]{click(.23,y);},.23,y);return;}
  if(screen==Screen::PreMatch){if(!has(r.text,cfg.team))throw std::runtime_error("No se ha verificado el nombre del rival.");const Line* yes=nullptr;for(auto&l:r.lines)if(norm(trim(l.text))==L"si"&&l.y>.5&&l.x<.5){yes=&l;break;}if(yes){if(now-lastAction<std::chrono::milliseconds(1600))return;tap(yes,L"Confirmar partido");begin();commander=false;}else tap(find(r,L"empezar",.75),L"Empezar partido");return;}
  status(L"Esperando · "+label(screen));
 }
 #include "catalog_engine.inc"
};
static void worker(){winrt::init_apartment(winrt::apartment_type::multi_threaded);try{Vision vision;Engine engine;std::optional<Clock::time_point> suspendedAt;while(!quitting){int mode=requestMode.exchange(0);if(mode){engine.reset(mode);suspendedAt.reset();std::this_thread::sleep_for(std::chrono::milliseconds(2200));}if(!active||paused){if(!suspendedAt)suspendedAt=Clock::now();std::this_thread::sleep_for(std::chrono::milliseconds(100));continue;}if(suspendedAt){auto delay=Clock::now()-*suspendedAt;engine.entered+=delay;engine.matchStart+=delay;engine.newMatchAt+=delay;suspendedAt.reset();}try{ensureInput();auto start=Clock::now();Image im=Image::capture(gameWindow);auto reading=vision.observe(im);if(!active||paused)continue;ensureInput();{std::lock_guard lock(dataMutex);debug.image=im;debug.reading=reading;debug.screen=classify(reading,engine.recorded&&!engine.awaitingNew);debug.elapsedMs=int(std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-start).count());debug.foreground=true;}engine.tick(im,reading,vision);notify();std::this_thread::sleep_for(std::chrono::milliseconds(400));}catch(...){if(active){paused=true;status(L"Pausado · "+exceptionText());PostMessageW(mainWindow,WM_APP+2,0,0);}}}}catch(...){paused=true;status(exceptionText());}winrt::uninit_apartment();}
#ifndef ZZEN_ENGINE_TEST
#include "ui.hpp"
#endif
