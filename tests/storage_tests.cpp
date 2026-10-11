#include "../src/vision.hpp"
#include "../src/storage.hpp"
#include <cassert>
#include <iostream>
int wmain(int argc,wchar_t** argv){
    using namespace zzen;if(argc!=2)return 2;
    winrt::init_apartment(winrt::apartment_type::multi_threaded);
    fs::path root(argv[1]);fs::create_directories(root/L"data");
    auto history=root/L"data"/L"history.json";
    writeText(history,LR"({"matches":[{"id":"private","outcome":"Victoria"}]})");
    Store first(root);assert(first.history.empty());auto original=readText(history);
    Record r;r.id=L"test";first.history.push_back(r);first.saveHistory();
    assert(readText(history)==original);Store second(root);assert(second.history.empty());
    first.config.count=7;first.config.infinite=true;first.config.language=5;first.profile();
    Store loaded(root);assert(loaded.config.count==7&&loaded.config.infinite&&loaded.config.language==5);
    first.config.count=8;first.profile();
    writeText(root/L"data"/L"profile.json",L"broken JSON");
    Store recovered(root);assert(recovered.config.count==7);
    writeText(root/L"data"/L"profile.json",LR"({"count":999999999999999,"difficulty":-100,"occurrence":"bad"})");
    Store invalid(root);assert(invalid.config.count==100000&&invalid.config.difficulty==0&&invalid.config.occurrence==1);
    first.catalog={{L"Inazuma Eleven",L"Royal Academy",1,0},{L"Inazuma Eleven",L"Royal Academy",2,0}};first.saveCatalog();
    Store catalog(root);assert(catalog.catalog.size()==2&&catalog.catalog[1].occurrence==2);
    std::cout<<"OK: session isolation, profile, backup recovery, bounds, duplicate rivals\n";
    winrt::uninit_apartment();
}
