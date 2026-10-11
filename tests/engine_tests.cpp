#define ZZEN_ENGINE_TEST
#include "../src/zzen.cpp"
#include <cassert>
#include <iostream>
struct FakeVision {int calls=0;std::optional<std::pair<int,int>> score(const Image&){++calls;return std::pair{4,0};}};
static Reading reading(std::wstring text,std::wstring button=L"",double x=.7,double y=.9){
    Reading r;r.text=text;if(!button.empty())r.lines.push_back({button,x,y,.15,.03});return r;
}
static void ready(Engine& e){e.lastAction=Clock::now()-std::chrono::seconds(5);testInputs.clear();}
int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 2;winrt::init_apartment(winrt::apartment_type::multi_threaded);
    Gdiplus::GdiplusStartupInput gi;ULONG_PTR token;Gdiplus::GdiplusStartup(&token,&gi,nullptr);
    store=std::make_unique<Store>(argv[1]);store->config.count=2;active=true;
    Engine e;FakeVision vision;Image image(1920,1080);e.reset(1);
    e.tick(image,reading(L"Lista de equipos"),vision);assert(testInputs==std::vector<std::wstring>{L"key:86"});
    Reading formation;formation.bottom=L"Alt Terminar edición";ready(e);e.tick(image,formation,vision);assert(testInputs.size()==1&&e.inMatch);
    auto final=reading(L"RESULTADOS FIN DEL PARTIDO",L"Fin del partido");
    ready(e);e.tick(image,final,vision);assert(store->history.size()==1&&e.finished==1&&vision.calls==1);
    ready(e);e.tick(image,final,vision);assert(e.finished==1&&vision.calls==1&&testInputs.size()==1);
    ready(e);e.tick(image,reading(L"RESULTADOS OBJETOS ESPÍRITUS",L"Siguiente"),vision);assert(testInputs.size()==1);
    auto rematch=reading(L"RESULTADOS Jugar revancha",L"Jugar revancha",.35,.65);
    rematch.lines.push_back({L"Volver al menú",.35,.75,.3,.06});
    ready(e);e.tick(image,rematch,vision);assert(e.awaitingNew&&testInputs.size()==1);
    ready(e);e.tick(image,rematch,vision);assert(testInputs.empty()&&e.finished==1);
    auto kickoff=reading(L"Saque de centro",L"Saque de centro",.4,.9);
    kickoff.lines.push_back({L"Modo manual",.02,.9,.25,.05});
    ready(e);e.tick(image,kickoff,vision);assert(e.inMatch&&!e.awaitingNew&&e.commander);
    ready(e);e.tick(image,kickoff,vision);assert(testInputs.size()==1&&testInputs[0]==L"click");
    ready(e);e.tick(image,final,vision);assert(e.finished==2&&store->history.size()==2);
    ready(e);e.tick(image,rematch,vision);assert(!active&&testInputs.size()==1);
    // Infinite sessions must repeat even when their numeric target was reached.
    active=true;store->config.infinite=true;e.reset(1);e.addRecord(image,final,std::pair{0,1});e.finished=100001;
    ready(e);e.tick(image,rematch,vision);assert(e.awaitingNew&&active);
    // Observation remains read-only even after a long period on one screen.
    e.reset(3);e.previous=Screen::RouteMap;e.entered=Clock::now()-std::chrono::minutes(3);
    ready(e);e.tick(image,reading(L"Lista de equipos"),vision);assert(testInputs.empty());
    // A session cannot adopt results which were already open before it started.
    e.reset(1);ready(e);e.tick(image,rematch,vision);assert(testInputs.empty()&&e.finished==0);
    // A paused engine cannot send an action.
    paused=true;bool rejected=false;try{ready(e);e.tick(image,formation,vision);}catch(...){rejected=true;}
    assert(rejected&&testInputs.empty());paused=false;
    std::cout<<"OK: navigation, formation exit, single counting, rematch, finite/infinite, observation, stale results, pause\n";
    store.reset();Gdiplus::GdiplusShutdown(token);winrt::uninit_apartment();
}
