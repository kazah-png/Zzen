#pragma once
#include <winrt/Windows.Data.Json.h>
#include <filesystem>
#include <fstream>
#include <set>
#include <cmath>
#include "core.hpp"
namespace zzen {
namespace fs = std::filesystem;
using namespace winrt::Windows::Data::Json;
inline std::wstring readText(const fs::path& p) {
    std::ifstream f(p, std::ios::binary);
    if (!f) return L"";
    std::string s((std::istreambuf_iterator<char>(f)), {});
    if (s.starts_with("\xef\xbb\xbf")) s.erase(0, 3);
    return winrt::to_hstring(s).c_str();
}
inline void writeText(const fs::path& p, const std::wstring& t) {
    auto s = winrt::to_string(t);
    std::ofstream f(p, std::ios::binary | std::ios::trunc);
    f.write(s.data(), s.size());
    if (!f) throw std::runtime_error("No se puede guardar el archivo");
}
inline void atomic(const fs::path& p, const std::wstring& t) {
    auto tmp = p; tmp += L".tmp"; writeText(tmp, t);
    if (fs::exists(p)) {
        auto bak = p; bak += L".bak";
        if (!ReplaceFileW(p.c_str(), tmp.c_str(), bak.c_str(), 0, nullptr, nullptr))
            throw std::runtime_error("No se pudo reemplazar el archivo de datos");
    } else if (!MoveFileW(tmp.c_str(), p.c_str())) throw std::runtime_error("No se pudo crear el archivo de datos");
}
inline std::wstring stamp(bool id=false) {
    SYSTEMTIME t; GetLocalTime(&t); wchar_t s[80];
    if (id) swprintf_s(s,L"%04u%02u%02u-%02u%02u%02u-%03u",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond,t.wMilliseconds);
    else swprintf_s(s,L"%04u-%02u-%02u %02u:%02u:%02u",t.wYear,t.wMonth,t.wDay,t.wHour,t.wMinute,t.wSecond);
    return s;
}
inline std::wstring js(const JsonObject& o,const wchar_t* k,const wchar_t* d=L""){return o.GetNamedString(k,d).c_str();}
inline void put(JsonObject& o,const wchar_t* k,const std::wstring& v){o.SetNamedValue(k,JsonValue::CreateStringValue(v));}
inline JsonObject loadObj(const fs::path& p) {
    for(auto suffix:{L"",L".bak"}) {
        auto path=p; path+=suffix; JsonObject o;
        if(JsonObject::TryParse(readText(path),o)) return o;
    }
    return JsonObject();
}
inline int boundedNumber(const JsonObject& o,const wchar_t* key,int fallback,int low,int high) {
    auto v=o.TryLookup(key);
    if(!v || v.ValueType()!=JsonValueType::Number) return fallback;
    double n=v.GetNumber();
    if(!std::isfinite(n)) return fallback;
    return int(std::clamp(n,double(low),double(high)));
}
struct CatalogEntry {std::wstring route,team;int occurrence=1,tab=0;};
struct Store {
    fs::path root;
    Config config;
    // Free edition keeps match results only in memory. Never load/save history.json.
    std::vector<Record> history;
    std::vector<CatalogEntry> catalog;
    explicit Store(fs::path p):root(std::move(p)) {
        fs::create_directories(root/L"data");
        auto o=loadObj(root/L"data"/L"profile.json");
        config.team=js(o,L"team",L"Royal Academy");
        config.route=js(o,L"route",L"Inazuma Eleven");
        config.difficulty=boundedNumber(o,L"difficulty",3,0,5);
        config.count=boundedNumber(o,L"count",1,1,100000);
        config.occurrence=boundedNumber(o,L"occurrence",1,1,20);
        config.language=boundedNumber(o,L"language",0,0,5);
        config.infinite=o.GetNamedBoolean(L"infinite",false);
        auto c=loadObj(root/L"data"/L"catalog.json").GetNamedArray(L"teams",JsonArray());
        for(auto value:c) {
            if(value.ValueType()!=JsonValueType::Object) continue;
            auto j=value.GetObject(); auto route=js(j,L"route"),team=js(j,L"team");
            if(route.empty()||team.empty())continue;
            catalog.push_back({route,team,boundedNumber(j,L"occurrence",1,1,20),boundedNumber(j,L"tab",0,0,7)});
        }
    }
    void profile() {
        JsonObject o;put(o,L"team",config.team);put(o,L"route",config.route);
        o.SetNamedValue(L"language",JsonValue::CreateNumberValue(config.language));
        o.SetNamedValue(L"difficulty",JsonValue::CreateNumberValue(config.difficulty));
        o.SetNamedValue(L"count",JsonValue::CreateNumberValue(config.count));
        o.SetNamedValue(L"occurrence",JsonValue::CreateNumberValue(config.occurrence));
        o.SetNamedValue(L"infinite",JsonValue::CreateBooleanValue(config.infinite));
        atomic(root/L"data"/L"profile.json",o.Stringify().c_str());
    }
    void saveHistory() {} // Shared engine hook; deliberately session-only.
    void saveCatalog() {
        JsonArray a;
        for(auto& c:catalog) {
            JsonObject j;put(j,L"route",c.route);put(j,L"team",c.team);
            j.SetNamedValue(L"occurrence",JsonValue::CreateNumberValue(c.occurrence));
            j.SetNamedValue(L"tab",JsonValue::CreateNumberValue(c.tab));a.Append(j);
        }
        JsonObject o;o.SetNamedValue(L"teams",a);
        atomic(root/L"data"/L"catalog.json",o.Stringify().c_str());
    }
};
inline std::wstring difficultyName(int i) {
    static const wchar_t* names[]={L"Crónica",L"Desbloqueo de ruta",L"Objetos inusuales",L"Heroica",L"Heroica +",L"Legendaria"};
    return names[std::clamp(i,0,5)];
}
}
