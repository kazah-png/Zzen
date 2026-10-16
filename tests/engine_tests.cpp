#define ZZEN_ENGINE_TEST
#include "../src/zzen.cpp"
#include <cassert>
#include <iostream>
struct FakeVision {int calls=0;std::optional<std::pair<int,int>> score(const Image&){++calls;return std::pair{4,0};}};
struct UnreadableVision {int calls=0;std::optional<std::pair<int,int>> score(const Image&){++calls;return {};}};
static Reading reading(std::wstring text,std::wstring button=L"",double x=.7,double y=.9){
    Reading r;r.text=text;if(!button.empty())r.lines.push_back({button,x,y,.15,.03});return r;
}
static void ready(Engine& e){e.lastAction=Clock::now()-std::chrono::seconds(5);testInputs.clear();}
int wmain(int argc,wchar_t** argv){
    if(argc<2||argc>4)return 2;winrt::init_apartment(winrt::apartment_type::multi_threaded);
    Gdiplus::GdiplusStartupInput gi;ULONG_PTR token;Gdiplus::GdiplusStartup(&token,&gi,nullptr);
    store=std::make_unique<Store>(argv[1]);store->config.count=2;active=true;
    Engine e;FakeVision vision;Image image(1920,1080);e.reset(1);
    assert(Engine::nameKey(L"EasÑvind")==Engine::nameKey(L"Eastwind"));
    assert(Engine::nameKey(L"Eastwind")!=Engine::nameKey(L"West Manor"));
    e.tick(image,reading(L"Lista de equipos"),vision);assert(testInputs==std::vector<std::wstring>{L"key:86"});
    Reading formation;formation.bottom=L"Alt Terminar edición";ready(e);e.tick(image,formation,vision);assert(testInputs.size()==1&&e.inMatch);
    auto final=reading(L"RESULTADOS FIN DEL PARTIDO",L"Fin del partido");
    ready(e);e.tick(image,final,vision);assert(store->history.size()==1&&e.finished==1&&vision.calls==1);
    ready(e);e.tick(image,final,vision);assert(e.finished==1&&vision.calls==1&&testInputs.size()==1);
    // A completed match may lose its heading between OCR frames. Still advance once,
    // keep the result, and do not reset the screen watchdog for the same result page.
    auto nextOnly=reading(L"0 Siguiente",L"0 Siguiente",.8,.91);
    auto experience=reading(L"RESULTADOS",L"Siguiente",.8,.91);
    for(auto& frame:{experience,nextOnly,experience,nextOnly}){
        ready(e);e.tick(image,frame,vision);
        assert(testInputs.size()==1&&e.previous==Screen::Experience);
        assert(e.finished==1&&store->history.size()==1&&vision.calls==1);
    }
    e.entered=Clock::now()-std::chrono::seconds(80);ready(e);
    bool timedOut=false;try{e.tick(image,nextOnly,vision);}catch(const std::runtime_error&){timedOut=true;}
    assert(timedOut&&testInputs.empty());e.entered=Clock::now();
    ready(e);e.tick(image,reading(L"RESULTADOS OBJETOS ESPÍRITUS",L"Siguiente"),vision);assert(testInputs.size()==1);
    assert(e.previous==Screen::Rewards&&!paused);
    ready(e);e.tick(image,reading(L"OBJETOS ESPÍRITUS",L"Siguiente"),vision);
    assert(e.previous==Screen::Rewards&&testInputs.size()==1&&e.finished==1);
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
    // Missing a score must not stop farming or turn an unknown result into a win.
    active=true;store->config.infinite=true;e.reset(1);e.begin();
    ready(e);e.tick(image,reading(L"OBJETOS ESPÍRITUS",L"Siguiente"),vision);
    assert(e.finished==1&&store->history.back().outcome==L"Pendiente");
    auto noisyRematch=reading(L"RESULTADOS OBJETOS ESPÍRITUS Jugar revanchai Volver al menú",L"Jugar revanchai",.35,.65);
    noisyRematch.lines.push_back({L"Volver al menú",.35,.75,.3,.06});
    ready(e);e.tick(image,noisyRematch,vision);
    assert(e.awaitingNew&&active&&!paused&&testInputs.size()==1);
    assert(e.finished==1&&store->history.back().outcome==L"Pendiente"&&store->history.back().score.empty());
    ready(e);e.tick(image,noisyRematch,vision);assert(testInputs.empty()&&e.finished==1);
    // Unknown results still respect the finite match limit.
    store->config.infinite=false;store->config.count=1;e.reset(1);e.begin();
    ready(e);e.tick(image,reading(L"OBJETOS ESPÍRITUS",L"Siguiente"),vision);
    ready(e);e.tick(image,noisyRematch,vision);assert(!active&&testInputs.size()==1&&e.finished==1);
    // A recognized menu with no readable button must not arm a phantom rematch or stop.
    active=true;e.reset(1);e.begin();
    ready(e);e.tick(image,reading(L"Jugar revancha"),vision);
    assert(e.finished==1&&active&&!e.awaitingNew&&testInputs.empty());
    store->config.infinite=true;e.reset(1);e.begin();
    ready(e);e.tick(image,reading(L"Jugar revancha"),vision);
    assert(e.finished==1&&active&&!e.awaitingNew&&testInputs.empty());
    ready(e);e.tick(image,noisyRematch,vision);assert(e.awaitingNew&&testInputs.size()==1);
    // After a bounded retry, an unreadable final score is saved once and accepted.
    e.reset(1);e.begin();UnreadableVision unreadable;
    auto footerOnly=reading(L"Fin del partido",L"Fin del partido",.7,.92);
    ready(e);e.tick(image,footerOnly,unreadable);
    assert(e.previous==Screen::FinalScore&&e.finished==0&&testInputs.empty());
    e.entered=Clock::now()-std::chrono::seconds(21);
    ready(e);e.tick(image,footerOnly,unreadable);
    assert(e.scoreTimedOut&&e.finished==1&&testInputs.size()==1&&store->history.back().outcome==L"Pendiente");
    int attempts=unreadable.calls;
    ready(e);e.tick(image,footerOnly,unreadable);
    assert(unreadable.calls==attempts&&e.finished==1&&testInputs.size()==1);
    ready(e);e.tick(image,noisyRematch,unreadable);assert(e.awaitingNew&&active);
    ready(e);e.tick(image,kickoff,vision);assert(!e.scoreTimedOut&&e.inMatch&&!e.awaitingNew);
    // Observation remains read-only even after a long period on one screen.
    e.reset(3);e.previous=Screen::RouteMap;e.entered=Clock::now()-std::chrono::minutes(3);
    ready(e);e.tick(image,reading(L"Lista de equipos"),vision);assert(testInputs.empty());
    // A session cannot adopt results which were already open before it started.
    e.reset(1);ready(e);e.tick(image,rematch,vision);assert(testInputs.empty()&&e.finished==0);
    // A paused engine cannot send an action.
    paused=true;bool rejected=false;try{ready(e);e.tick(image,formation,vision);}catch(...){rejected=true;}
    assert(rejected&&testInputs.empty());paused=false;
    // Regression: first page with fourth row hovered used to scroll forever.
    Image listImage(1920,1080);std::fill(listImage.pixels.begin(),listImage.pixels.end(),static_cast<unsigned char>(0));
    for(int y=286;y<695;++y){auto p=listImage.at(909,y);p[0]=p[1]=p[2]=255;}
    auto hoverPixel=listImage.at(int(1920*.31),515);hoverPixel[0]=220;hoverPixel[1]=90;hoverPixel[2]=0;
    Reading firstPage;firstPage.text=L"Elige un equipo al que enfrentarte";
    firstPage.lines={{L"Royal Academy",.062,.277,.2,.03},{L"Wild",.062,.467,.15,.03},{L"Brain",.062,.53,.15,.03}};
    e.reset(1);e.cfg.team=L"Brain";e.cfg.occurrence=1;e.tabReady=true;active=true;
    ready(e);assert(e.selection(listImage)==3);e.selectTeam(listImage,firstPage);assert(testInputs.empty());
    ready(e);e.selectTeam(listImage,firstPage);assert(e.topPass==2&&testInputs==std::vector<std::wstring>{L"click"});
    assert(debug.action==L"Seleccionar Brain · aparición 1");
    // Being on the first highlighted row of a lower page does not imply top.
    e.clearPage();std::fill(listImage.pixels.begin(),listImage.pixels.end(),static_cast<unsigned char>(0));
    for(int y=450;y<750;++y){auto p=listImage.at(909,y);p[0]=p[1]=p[2]=255;}
    auto row0=listImage.at(int(1920*.31),311);row0[0]=220;row0[1]=90;
    ready(e);assert(!e.atTop(listImage,firstPage));assert(testInputs==std::vector<std::wstring>{L"scroll:600"});
    // No scrollbar/blank captures cannot declare top and retries are bounded.
    e.clearPage();Image blank(1920,1080);
    for(int i=0;i<24;++i){ready(e);assert(!e.atTop(blank,firstPage));}
    ready(e);bool bounded=false;try{e.atTop(blank,firstPage);}catch(const std::runtime_error&){bounded=true;}
    assert(bounded&&testInputs.empty()&&e.topPass!=2);
    // Duplicate occurrences continue to resolve from the complete first page.
    e.clearPage();e.topPass=2;e.cfg.team=L"Royal Academy";e.cfg.occurrence=2;
    firstPage.lines.push_back({L"Royal Academy",.062,.65,.2,.03});
    ready(e);e.selectTeam(listImage,firstPage);assert(testInputs.size()==1&&debug.y>.65);
    // A transitional/missing opponent heading waits without starting a match.
    e.reset(1);e.cfg.team=L"Brain";auto preMatch=reading(L"Empezar partido",L"Empezar partido",.4,.9);
    ready(e);e.tick(image,preMatch,vision);assert(testInputs.empty()&&!e.inMatch);
    e.entered=Clock::now()-std::chrono::seconds(13);ready(e);bool unverified=false;
    try{e.tick(image,preMatch,vision);}catch(const std::runtime_error&){unverified=true;}
    assert(unverified&&testInputs.empty());
    preMatch.text=L"Brain Empezar partido";ready(e);e.tick(image,preMatch,vision);assert(testInputs.size()==1);
    if(argc>=3){
        Vision realVision;auto captured=Image::load(argv[2]);auto actual=realVision.observe(captured);
        assert(classify(actual)==Screen::TeamList);
        e.clearPage();e.cfg.team=L"Brain";e.cfg.occurrence=1;e.tabReady=true;
        ready(e);e.selectTeam(captured,actual);assert(testInputs.empty());
        ready(e);e.selectTeam(captured,actual);assert(testInputs==std::vector<std::wstring>{L"click"});
        assert(debug.action==L"Seleccionar Brain · aparición 1"&&debug.y>.50&&debug.y<.58);
        std::cout<<"OK: actual Chronicle screenshot/OCR selects Brain while Wild is hovered\n";
    }
    if(argc==4){
        Vision realVision;auto captured=Image::load(argv[3]);auto actual=realVision.observe(captured);
        std::cout<<"PREMATCH OCR: "<<winrt::to_string(actual.text)<<"\n";
        assert(classify(actual)==Screen::PreMatch&&has(actual.text,L"Brain"));
        e.reset(1);e.cfg.team=L"Brain";ready(e);e.tick(captured,actual,realVision);assert(testInputs.size()==1);
        std::cout<<"OK: actual Brain pre-match banner verified before starting\n";
    }
    std::cout<<"OK: navigation, formation exit, single counting, rematch, finite/infinite, observation, stale results, pause\n";
    {Reading category;category.lines={{L"Inazuma Eleven: Heroes' Victory Road",.113,.22,.24,.03}};assert(e.readCategory(category)==category.lines[0].text);}
    assert(keyInput(VK_DOWN).ki.dwFlags&KEYEVENTF_EXTENDEDKEY);assert(keyInput(VK_UP).ki.dwFlags&KEYEVENTF_EXTENDEDKEY);assert(!(keyInput('X').ki.dwFlags&KEYEVENTF_EXTENDEDKEY));
    active=true;paused=false;e.reset(1);e.targetOnMap=true;ready(e);
    e.tick(image,reading(L"Lista de equipos"),vision);assert(testInputs==std::vector<std::wstring>{L"click"});
    e.cfg.team=L"Tormenta cortante";e.cfg.difficulty=7;Reading gate;
    gate.text=L"Tormenta cortante BATALLA DE PUERTA+ Guardar";
    gate.lines={{L"Guardar",.2,.696,.08,.02},{L"BATALLA DE PUERTA+",.14,.56,.18,.035}};
    ready(e);e.tick(image,gate,vision);assert(testInputs==std::vector<std::wstring>{L"click"});
    e.cfg.difficulty=3;ready(e);e.tick(image,gate,vision);assert(testInputs.empty());
    if(auto fixture=fs::path(argv[1]).parent_path().parent_path()/L"tests/private/royal-chronicle.png";fs::exists(fixture)){Vision real;
     auto actual=real.observe(Image::load(fixture));assert(classify(actual)==Screen::Difficulty);
     for(int id=0;id<6;++id){auto choice=difficultyLine(actual,id);assert(choice);assert(std::abs(choice->y+choice->h/2-(513.+70*id)/1080)<.02);}
     std::cout<<"OK: all six classic difficulties in actual Royal Academy screenshot\n";}
    // Refresh preserves known spelling while retaining distinct repeat occurrences.
    {store->catalog={{L"Inazuma Eleven",L"Brain",1,0},{L"Inazuma Eleven",L"Protocolo Omega 2.0",1,0}};
     e.reset(2);e.tabReady=true;e.topPass=2;e.stalePages=3;Reading pageReading;
     pageReading.lines={{L"Inazuma Eleven",.113,.22,.24,.03},{L"Braín",.062,.277,.2,.03},{L"Protocolo Omega 2,0",.062,.34,.2,.03},{L"Brain",.062,.40,.2,.03}};
     Image pageImage(1920,1080);auto pixel=pageImage.at(int(1920*.31),311);pixel[0]=220;pixel[1]=90;pixel[2]=0;
     e.lastSelection=e.signature(teams(pageReading))+L"#0";ready(e);e.scan(pageImage,pageReading,Screen::TeamList);
     assert(store->catalog.size()==3&&store->catalog[0].team==L"Brain"&&store->catalog[1].team==L"Protocolo Omega 2.0"&&store->catalog[2].occurrence==2);}
    store.reset();Gdiplus::GdiplusShutdown(token);winrt::uninit_apartment();
}
