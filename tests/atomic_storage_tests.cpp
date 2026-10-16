#include "../src/vision.hpp"
#include "../src/storage.hpp"
#include <cassert>
#include <thread>
#include <iostream>
int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;using namespace zzen;winrt::init_apartment(winrt::apartment_type::multi_threaded);
 fs::path root(argv[1]);fs::create_directories(root);auto path=root/L"atomic.json";
 writeText(path,L"old");
 auto lock=[&]{auto handle=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);assert(handle!=INVALID_HANDLE_VALUE);return handle;};
 auto handle=lock();std::thread release([=]{Sleep(150);CloseHandle(handle);});
 try{atomic(path,L"new");}catch(...){release.join();throw;}release.join();
 assert(readText(path)==L"new"&&readText(root/L"atomic.json.bak")==L"old");
 handle=lock();bool rejected=false;try{atomic(path,L"pending");}catch(const std::runtime_error&){rejected=true;}CloseHandle(handle);
 assert(rejected&&readText(path)==L"new");
 atomic(path,L"recovered");assert(readText(path)==L"recovered"&&readText(root/L"atomic.json.bak")==L"new");
 std::cout<<"OK: transient read lock recovery, bounded persistent lock failure, old data and backup preservation\n";
 winrt::uninit_apartment();
}
