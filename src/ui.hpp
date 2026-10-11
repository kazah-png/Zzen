#pragma once
#include <uxtheme.h>
#include "i18n.hpp"
#pragma comment(lib,"uxtheme.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace ui {
constexpr COLORREF bg=RGB(14,16,22), panel=RGB(22,25,34), ink=RGB(235,237,247), muted=RGB(144,152,174), accent=RGB(164,150,255), line=RGB(43,48,64);
HBRUSH background=CreateSolidBrush(bg), fieldBrush=CreateSolidBrush(panel);
HFONT font,smallFont,titleFont,numberFont;
HWND languageBox;std::map<HWND,std::wstring> captions;
std::wstring tr(const std::wstring& s){return i18n::tr(s);}
HWND routeBox,teamBox,diffBox,countBox,occBox,infiniteBox,statusBox,pageTitle,pageSubtitle;
HWND historyList,details,preview,ocrBox,logBox,debugInfo;
HWND statNumber[4],statLabel[4];
std::vector<HWND> sessionControls,debugControls,helpControls;
int page=0;bool refreshing=false;std::wstring historyKey,lastOCR,lastLog;
std::wstring value(HWND h){int n=GetWindowTextLengthW(h);std::wstring s(n+1,L'\0');GetWindowTextW(h,s.data(),n+1);s.resize(n);return s;}
void text(HWND h,const std::wstring& s){auto t=tr(s);if(value(h)!=t)SetWindowTextW(h,t.c_str());}
HWND control(const wchar_t* cls,const wchar_t* text,int id,DWORD style=0){
    HWND c=CreateWindowExW(0,cls,tr(text).c_str(),WS_CHILD|WS_VISIBLE|style,0,0,10,10,mainWindow,(HMENU)(INT_PTR)id,GetModuleHandleW(nullptr),nullptr);
    SendMessageW(c,WM_SETFONT,(WPARAM)font,TRUE);if(*text)captions[c]=text;return c;
}
HWND label(const wchar_t* text,int id=0){return control(L"STATIC",text,id);}
HWND button(const wchar_t* text,int id){return control(L"BUTTON",text,id,BS_OWNERDRAW|WS_TABSTOP);}
void place(HWND h,int x,int y,int w,int hh){MoveWindow(h,x,y,w,hh,TRUE);}
void placeID(int id,int x,int y,int w,int h){place(GetDlgItem(mainWindow,id),x,y,w,h);}
std::wstring crlf(std::wstring s){size_t p=0;while((p=s.find(L'\n',p))!=s.npos){if(p==0||s[p-1]!=L'\r'){s.insert(p,L"\r");++p;}++p;}return s;}
int selected(){return ListView_GetNextItem(historyList,-1,LVNI_SELECTED);}

void catalogUI(){
    std::vector<CatalogEntry> entries;{std::lock_guard lock(dataMutex);entries=store->catalog;}
    auto route=value(routeBox),team=value(teamBox);if(route.empty())route=store->config.route;if(team.empty())team=store->config.team;std::vector<std::wstring> routes;std::set<std::wstring> teams;
    for(auto& c:entries){if(std::find(routes.begin(),routes.end(),c.route)==routes.end())routes.push_back(c.route);if(c.route==route)teams.insert(c.team);}
    SendMessageW(routeBox,CB_RESETCONTENT,0,0);SendMessageW(teamBox,CB_RESETCONTENT,0,0);
    for(auto& s:routes)SendMessageW(routeBox,CB_ADDSTRING,0,(LPARAM)s.c_str());
    for(auto& s:teams)SendMessageW(teamBox,CB_ADDSTRING,0,(LPARAM)s.c_str());
    if(!teams.empty()&&!teams.contains(team))team=*teams.begin();
    SendMessageW(routeBox,CB_SELECTSTRING,WPARAM(-1),(LPARAM)route.c_str());SendMessageW(teamBox,CB_SELECTSTRING,WPARAM(-1),(LPARAM)team.c_str());
}
void saveConfig(){
    Config c;c.language=i18n::language;c.route=trim(value(routeBox));c.team=trim(value(teamBox));
    c.difficulty=int(SendMessageW(diffBox,CB_GETCURSEL,0,0));
    c.count=GetDlgItemInt(mainWindow,104,nullptr,FALSE);c.occurrence=GetDlgItemInt(mainWindow,105,nullptr,FALSE);
    c.infinite=SendMessageW(infiniteBox,BM_GETCHECK,0,0)==BST_CHECKED;
    if(c.infinite&&c.count<1)c.count=1;
    if(c.route.empty()||c.team.empty()||c.difficulty<0||c.count<1||c.count>100000||c.occurrence<1||c.occurrence>20)
        throw std::runtime_error("Revisa rival, dificultad, cantidad (1–100000) y aparición (1–20).");
    std::lock_guard lock(dataMutex);store->config=c;store->profile();
}
void startMode(int mode){
    if(active)throw std::runtime_error("Detén la sesión actual antes de iniciar otra.");
    saveConfig();gameWindow=game();active=true;paused=false;requestMode=mode;
    status(mode==1?L"Preparando sesión · F8 para detener":mode==2?L"Actualizando catálogo de Crónica":L"Observando sin enviar entradas");
    ShowWindow(mainWindow,SW_MINIMIZE);SetForegroundWindow(gameWindow);
}
void showPage(){
    for(auto h:sessionControls)ShowWindow(h,page==0?SW_SHOW:SW_HIDE);
    for(auto h:debugControls)ShowWindow(h,page==1?SW_SHOW:SW_HIDE);
    for(auto h:helpControls)ShowWindow(h,page==2?SW_SHOW:SW_HIDE);
    text(pageTitle,page==0?L"Tu próxima sesión":page==1?L"Diagnóstico en directo":L"Todo listo para empezar");
    text(pageSubtitle,page==0?L"Elige el partido. Zzen se encarga de repetirlo.":page==1?L"Pantalla detectada, lectura y acciones del motor.":L"Crónica · Interfaz del juego en español · Teclado y ratón");
    InvalidateRect(mainWindow,nullptr,TRUE);
}
void showRecord(){
    if(refreshing)return;int i=selected();Record r;
    {std::lock_guard lock(dataMutex);if(i<0||size_t(i)>=store->history.size())return;r=store->history[i];}
    text(details,tr(r.outcome)+L"  "+r.score+L"  ·  "+r.team+L"  ·  "+tr(r.difficulty)+L"\r\n"+tr(r.outcome==L"Pendiente"?L"Revisa Debug o la captura antes de corregir el resultado.":L"Los resultados de Gratis se conservan solo durante esta sesión."));
}
void refresh(){
    std::vector<Record> rows;std::wstring statusText,reading,log,action;Screen screen;int elapsed,completed;Config cfg;
    {std::lock_guard lock(dataMutex);rows=store->history;statusText=debug.status;reading=debug.reading.text;log=debug.log;action=debug.action;screen=debug.screen;elapsed=debug.elapsedMs;completed=debug.finished;cfg=store->config;}
    text(statusBox,statusText);int wins=0,losses=0,other=0;
    for(auto& r:rows){wins+=r.outcome==L"Victoria";losses+=r.outcome==L"Derrota";other+=r.outcome==L"Empate"||r.outcome==L"Pendiente";}
    text(statNumber[0],std::to_wstring(completed)+(cfg.infinite?L" / ∞":L" / "+std::to_wstring(cfg.count)));
    text(statNumber[1],std::to_wstring(wins));text(statNumber[2],std::to_wstring(losses));text(statNumber[3],std::to_wstring(other));
    for(int id:{101,102,103,104,105,106,110,114,115})EnableWindow(GetDlgItem(mainWindow,id),!active);
    EnableWindow(countBox,!active&&SendMessageW(infiniteBox,BM_GETCHECK,0,0)!=BST_CHECKED);
    EnableWindow(GetDlgItem(mainWindow,111),active&&!paused);EnableWindow(GetDlgItem(mainWindow,112),active&&paused);EnableWindow(GetDlgItem(mainWindow,113),active);
    std::wstring key;for(auto& r:rows)key+=r.id+r.outcome+r.score;
    if(key!=historyKey){
        int previous=selected();refreshing=true;ListView_DeleteAllItems(historyList);
        for(size_t i=0;i<rows.size();++i){auto& r=rows[i];LVITEMW item{};item.mask=LVIF_TEXT;item.iItem=int(i);item.pszText=r.date.data();ListView_InsertItem(historyList,&item);
            std::wstring cells[]={r.team,tr(r.difficulty),tr(r.outcome),r.score};for(int j=0;j<4;++j)ListView_SetItemText(historyList,int(i),j+1,cells[j].data());}
        historyKey=key;refreshing=false;
        if(previous>=0&&size_t(previous)<rows.size())ListView_SetItemState(historyList,previous,LVIS_SELECTED,LVIS_SELECTED);
        if(rows.empty())text(details,L"Sin partidos todavía. Abre Crónica en el juego y pulsa Iniciar sesión.");
        else if(previous<0||size_t(previous)>=rows.size())text(details,L"Selecciona un partido para ver su resultado. Los resultados de Gratis duran esta sesión.");
    }
    if(page==1){
        if(reading!=lastOCR){lastOCR=reading;text(ocrBox,crlf(reading));}
        if(log!=lastLog){lastLog=log;text(logBox,log);SendMessageW(logBox,EM_SETSEL,log.size(),log.size());SendMessageW(logBox,EM_SCROLLCARET,0,0);}
        text(debugInfo,tr(zzen::label(screen))+L"   ·   "+std::to_wstring(elapsed)+L" ms\r\n"+action);InvalidateRect(preview,nullptr,FALSE);
    }
}
void diagnostic(){
    DebugState d;{std::lock_guard lock(dataMutex);d=debug;}
    if(!d.image.width)throw std::runtime_error("Primero utiliza Observar juego, en Diagnóstico.");
    auto p=store->root/L"debug"/stamp(true);fs::create_directories(p);d.image.save(p/L"pantalla.png");
    writeText(p/L"diagnostico.txt",zzen::label(d.screen)+L"\r\n"+d.action+L"\r\n"+d.reading.text+L"\r\n"+d.log);
    status(L"Diagnóstico guardado en "+p.wstring());
}
void correctResult(){
    int i=selected();if(i<0)return;
    HMENU menu=CreatePopupMenu();AppendMenuW(menu,MF_STRING,1,tr(L"Victoria").c_str());AppendMenuW(menu,MF_STRING,2,tr(L"Derrota").c_str());AppendMenuW(menu,MF_STRING,3,tr(L"Empate").c_str());
    POINT p;GetCursorPos(&p);int c=TrackPopupMenu(menu,TPM_RETURNCMD,p.x,p.y,0,mainWindow,nullptr);DestroyMenu(menu);
    if(c){std::lock_guard lock(dataMutex);if(size_t(i)<store->history.size()){store->history[i].outcome=c==1?L"Victoria":c==2?L"Derrota":L"Empate";store->history[i].source=L"Revisión manual";}}
    refresh();showRecord();
}
LRESULT CALLBACK PreviewProc(HWND h,UINT msg,WPARAM w,LPARAM l){
    if(msg!=WM_PAINT)return DefWindowProcW(h,msg,w,l);
    PAINTSTRUCT ps;auto dc=BeginPaint(h,&ps);RECT rect;GetClientRect(h,&rect);FillRect(dc,&rect,fieldBrush);
    {std::lock_guard lock(dataMutex);auto& im=debug.image;
        if(im.width){Gdiplus::Graphics g(dc);Gdiplus::Bitmap b(im.width,im.height,im.width*4,PixelFormat32bppARGB,im.pixels.data());float scale=std::min(float(rect.right)/im.width,float(rect.bottom)/im.height);float iw=im.width*scale,ih=im.height*scale;g.DrawImage(&b,0.f,0.f,iw,ih);
            Gdiplus::Pen box(Gdiplus::Color(190,99,230,180));for(auto& boxLine:debug.reading.lines)g.DrawRectangle(&box,float(boxLine.x*iw),float(boxLine.y*ih),float(boxLine.w*iw),float(boxLine.h*ih));
            if(debug.x>=0){Gdiplus::Pen pen(Gdiplus::Color(255,255,130,100),3);g.DrawEllipse(&pen,float(debug.x*iw-8),float(debug.y*ih-8),16.f,16.f);}}
        else{SetTextColor(dc,muted);SetBkMode(dc,TRANSPARENT);SelectObject(dc,font);DrawTextW(dc,tr(L"La captura del juego aparecerá aquí").c_str(),-1,&rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);}}
    EndPaint(h,&ps);return 0;
}
void rounded(HDC dc,RECT r,COLORREF fill,int radius=16,COLORREF border=line){
    auto b=CreateSolidBrush(fill);auto p=CreatePen(PS_SOLID,1,border);auto ob=SelectObject(dc,b),op=SelectObject(dc,p);
    RoundRect(dc,r.left,r.top,r.right,r.bottom,radius,radius);SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(b);DeleteObject(p);
}
void layout(){
    RECT r;GetClientRect(mainWindow,&r);int x=250,w=r.right-x-40,h=r.bottom;
    placeID(1,30,30,165,48);placeID(2,32,80,165,24);placeID(10,18,166,185,46);placeID(11,18,223,185,46);placeID(12,18,280,185,46);placeID(3,30,h-106,176,72);
    place(pageTitle,x,32,w-230,42);place(pageSubtitle,x,82,w,30);
    placeID(320,x+w-203,23,200,22);place(languageBox,x+w-203,50,203,250);
    placeID(308,x+19,126,w-38,24);placeID(309,x+19,155,w-38,24);
    int gap=24,a=(w-48)/3;
    for(int i=0;i<3;++i)placeID(301+i,x+i*(a+gap),211,a,22);
    place(routeBox,x,241,a,290);place(teamBox,x+a+gap,241,a,310);place(diffBox,x+2*(a+gap),241,a,260);
    placeID(304,x,300,135,22);place(countBox,x+12,332,100,29);placeID(305,x+160,300,205,24);place(occBox,x+172,332,91,29);
    place(infiniteBox,x+320,329,180,32);placeID(306,x+525,306,w-525,62);
    placeID(110,x,391,207,44);placeID(111,x+223,391,125,44);placeID(112,x+360,391,135,44);placeID(113,x+w-161,391,161,44);
    int cw=(w-48)/4;for(int i=0;i<4;++i){place(statNumber[i],x+i*(cw+16)+19,476,cw-38,41);place(statLabel[i],x+i*(cw+16)+19,519,cw-38,24);}
    placeID(307,x,580,w-260,24);placeID(114,x+w-230,568,230,37);
    place(historyList,x,620,w,std::max(100,h-771));place(details,x,h-127,w-215,61);placeID(204,x+w-205,h-116,205,37);
    place(debugInfo,x,143,w-235,52);placeID(115,x+w-223,140,223,40);
    int pw=(w-22)*2/3;place(preview,x,218,pw,(h-354)*2/3);place(ocrBox,x+pw+22,218,w-pw-22,h-325);
    int ly=235+(h-354)*2/3;place(logBox,x,ly,pw,h-ly-107);placeID(214,x,h-87,245,37);
    placeID(220,x,140,w,h-234);place(statusBox,x+19,h-40,w-38,27);
    int cols[]={174,w-595,145,158,118};for(int i=0;i<5;++i)ListView_SetColumnWidth(historyList,i,cols[i]);
}
void paint(){
    PAINTSTRUCT ps;HDC dc=BeginPaint(mainWindow,&ps);RECT r;GetClientRect(mainWindow,&r);FillRect(dc,&r,background);
    RECT sidebar{0,0,221,r.bottom};FillRect(dc,&sidebar,fieldBrush);
    auto p=CreatePen(PS_SOLID,1,line);auto op=SelectObject(dc,p);MoveToEx(dc,221,0,nullptr);LineTo(dc,221,r.bottom);SelectObject(dc,op);DeleteObject(p);
    int x=250,w=r.right-x-40;
    if(page==0){
        rounded(dc,{x,119,x+w,187},RGB(34,30,53),16,RGB(55,47,81));
        rounded(dc,{x-16,197,x+w+16,375},panel,20);
        rounded(dc,{x,326,x+125,367},panel,10);rounded(dc,{x+160,326,x+278,367},panel,10);
        int cw=(w-48)/4;for(int i=0;i<4;++i)rounded(dc,{x+i*(cw+16),459,x+i*(cw+16)+cw,557},panel,18);
    }
    rounded(dc,{x,r.bottom-47,x+w,r.bottom-12},RGB(24,28,38),12,RGB(24,28,38));
    EndPaint(mainWindow,&ps);
}
void drawButton(const DRAWITEMSTRUCT& d){
    bool nav=d.CtlID>=10&&d.CtlID<=12,chosen=nav&&int(d.CtlID)-10==page,primary=d.CtlID==110;
    bool disabled=(d.itemState&ODS_DISABLED)!=0;COLORREF fill=primary?accent:chosen?RGB(43,36,65):panel;
    if(disabled)fill=RGB(26,29,38);if(d.itemState&ODS_SELECTED)fill=RGB(65,53,97);
    FillRect(d.hDC,&d.rcItem,nav?fieldBrush:background);
    rounded(d.hDC,d.rcItem,fill,12,nav?fill:primary?fill:line);
    SetBkMode(d.hDC,TRANSPARENT);SetTextColor(d.hDC,disabled?RGB(89,95,113):primary?bg:chosen?accent:ink);SelectObject(d.hDC,font);RECT r=d.rcItem;
    auto t=value(d.hwndItem);if(nav){r.left+=17;DrawTextW(d.hDC,t.c_str(),-1,&r,DT_LEFT|DT_VCENTER|DT_SINGLELINE);}else DrawTextW(d.hDC,t.c_str(),-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    if(d.itemState&ODS_FOCUS){InflateRect(&r,-4,-4);DrawFocusRect(d.hDC,&r);}
}
void drawCombo(const DRAWITEMSTRUCT& d){
    FillRect(d.hDC,&d.rcItem,fieldBrush);SetBkMode(d.hDC,TRANSPARENT);SetTextColor(d.hDC,(d.itemState&ODS_DISABLED)?muted:ink);SelectObject(d.hDC,font);
    if(d.itemID==UINT(-1))return;wchar_t t[512]{};SendMessageW(d.hwndItem,CB_GETLBTEXT,d.itemID,(LPARAM)t);RECT r=d.rcItem;r.left+=10;
    if(d.itemState&ODS_SELECTED){auto b=CreateSolidBrush(RGB(43,36,65));FillRect(d.hDC,&d.rcItem,b);DeleteObject(b);}
    DrawTextW(d.hDC,t,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
}
void localize(){
    for(auto& [h,key]:captions)if(h!=countBox&&h!=occBox&&h!=details&&h!=statusBox)text(h,key);
    text(GetDlgItem(mainWindow,3),tr(L"Solo Crónica")+L"\r\n"+tr(L"F8 detiene la sesión"));
    int diff=int(SendMessageW(diffBox,CB_GETCURSEL,0,0));SendMessageW(diffBox,CB_RESETCONTENT,0,0);
    for(int i=0;i<6;++i){auto t=tr(difficultyName(i));SendMessageW(diffBox,CB_ADDSTRING,0,(LPARAM)t.c_str());}SendMessageW(diffBox,CB_SETCURSEL,diff<0?3:diff,0);
    const wchar_t* names[]={L"Fecha",L"Rival",L"Dificultad",L"Resultado",L"Marcador"};
    for(int i=0;i<5;++i){auto t=tr(names[i]);LVCOLUMNW c{};c.mask=LVCF_TEXT;c.pszText=t.data();ListView_SetColumn(historyList,i,&c);}
    text(GetDlgItem(mainWindow,220),i18n::help());historyKey=L"!";showPage();refresh();showRecord();InvalidateRect(mainWindow,nullptr,TRUE);
}
LRESULT CALLBACK ComboProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR){
    auto result=DefSubclassProc(h,msg,w,l);
    if(msg==WM_PAINT){auto dc=GetDC(h);RECT r;GetClientRect(h,&r);FillRect(dc,&r,fieldBrush);rounded(dc,r,panel,9);
        SetBkMode(dc,TRANSPARENT);SetTextColor(dc,IsWindowEnabled(h)?ink:muted);SelectObject(dc,font);RECT t=r;t.left+=12;t.right-=30;auto s=value(h);DrawTextW(dc,s.c_str(),-1,&t,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
        auto p=CreatePen(PS_SOLID,1,muted);auto old=SelectObject(dc,p);int x=r.right-17,y=r.bottom/2;MoveToEx(dc,x-4,y-2,nullptr);LineTo(dc,x,y+2);LineTo(dc,x+4,y-2);SelectObject(dc,old);DeleteObject(p);ReleaseDC(h,dc);}
    return result;
}
LRESULT CALLBACK HeaderProc(HWND h,UINT msg,WPARAM w,LPARAM l,UINT_PTR,DWORD_PTR){
    if(msg!=WM_PAINT)return DefSubclassProc(h,msg,w,l);
    PAINTSTRUCT ps;auto dc=BeginPaint(h,&ps);RECT all;GetClientRect(h,&all);FillRect(dc,&all,fieldBrush);SelectObject(dc,smallFont);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,muted);
    for(int i=0;i<Header_GetItemCount(h);++i){RECT r;Header_GetItemRect(h,i,&r);wchar_t s[200]{};HDITEMW item{};item.mask=HDI_TEXT;item.pszText=s;item.cchTextMax=200;Header_GetItem(h,i,&item);r.left+=8;DrawTextW(dc,s,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);}
    EndPaint(h,&ps);return 0;
}
void create(){
    i18n::language=store->config.language;
    font=CreateFontW(-16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    smallFont=CreateFontW(-14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    titleFont=CreateFontW(-30,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    numberFont=CreateFontW(-29,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    auto brand=label(L"Zzen",1);SendMessageW(brand,WM_SETFONT,(WPARAM)titleFont,TRUE);label(L"FREE  /  0.2",2);label(L"Solo Crónica\r\nF8 detiene la sesión",3);
    button(L"Sesión",10);button(L"Diagnóstico",11);button(L"Cómo usar Zzen",12);
    pageTitle=label(L"");SendMessageW(pageTitle,WM_SETFONT,(WPARAM)titleFont,TRUE);pageSubtitle=label(L"");SendMessageW(pageSubtitle,WM_SETFONT,(WPARAM)smallFont,TRUE);
    const wchar_t* fieldCaptions[]={L"Categoría",L"Rival",L"Dificultad",L"Partidos",L"Aparición del rival",L"El número distingue rivales repetidos en la ruta.",L"PARTIDOS DE ESTA SESIÓN"};
    for(int i=0;i<7;++i)sessionControls.push_back(label(fieldCaptions[i],301+i));
    routeBox=control(L"COMBOBOX",L"",101,CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL|WS_TABSTOP);teamBox=control(L"COMBOBOX",L"",102,CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_VSCROLL|WS_TABSTOP);
    diffBox=control(L"COMBOBOX",L"",103,CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_TABSTOP);countBox=control(L"EDIT",L"1",104,ES_NUMBER|WS_TABSTOP);occBox=control(L"EDIT",L"1",105,ES_NUMBER|WS_TABSTOP);
    infiniteBox=control(L"BUTTON",L"Sin límite",106,BS_AUTOCHECKBOX|WS_TABSTOP);SetWindowTheme(infiniteBox,L"",L"");
    for(auto h:{routeBox,teamBox,diffBox,countBox,occBox,infiniteBox})sessionControls.push_back(h);
    for(int i=0;i<6;++i){auto s=tr(difficultyName(i));SendMessageW(diffBox,CB_ADDSTRING,0,(LPARAM)s.c_str());}
    auto& c=store->config;SetWindowTextW(routeBox,c.route.c_str());SetWindowTextW(teamBox,c.team.c_str());SetWindowTextW(countBox,std::to_wstring(c.count).c_str());SetWindowTextW(occBox,std::to_wstring(c.occurrence).c_str());SendMessageW(diffBox,CB_SETCURSEL,c.difficulty,0);SendMessageW(infiniteBox,BM_SETCHECK,c.infinite?BST_CHECKED:BST_UNCHECKED,0);catalogUI();
    for(auto pair:{std::pair{110,L"Iniciar sesión"},std::pair{111,L"Pausar"},std::pair{112,L"Reanudar"},std::pair{113,L"Detener · F8"},std::pair{114,L"Actualizar catálogo"},std::pair{204,L"Corregir resultado"}})sessionControls.push_back(button(pair.second,pair.first));
    const wchar_t* names[]={L"COMPLETADOS",L"VICTORIAS",L"DERROTAS",L"EMPATES / PENDIENTES"};
    for(int i=0;i<4;++i){statNumber[i]=label(L"0",400+i);statLabel[i]=label(names[i],410+i);SendMessageW(statNumber[i],WM_SETFONT,(WPARAM)numberFont,TRUE);SendMessageW(statLabel[i],WM_SETFONT,(WPARAM)smallFont,TRUE);sessionControls.push_back(statNumber[i]);sessionControls.push_back(statLabel[i]);}
    historyList=control(WC_LISTVIEWW,L"",201,LVS_REPORT|LVS_SINGLESEL|WS_TABSTOP);ListView_SetExtendedListViewStyle(historyList,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
    ListView_SetBkColor(historyList,panel);ListView_SetTextBkColor(historyList,panel);ListView_SetTextColor(historyList,ink);SetWindowTheme(historyList,L"Explorer",nullptr);
    const wchar_t* cols[]={L"Fecha",L"Rival",L"Dificultad",L"Resultado",L"Marcador"};for(int i=0;i<5;++i){LVCOLUMNW col{};col.mask=LVCF_TEXT|LVCF_WIDTH;col.cx=150;auto translated=tr(cols[i]);col.pszText=translated.data();ListView_InsertColumn(historyList,i,&col);}sessionControls.push_back(historyList);
    details=control(L"EDIT",L"Sin partidos todavía. Abre Crónica en el juego y pulsa Iniciar sesión.",207,ES_MULTILINE|ES_READONLY);sessionControls.push_back(details);
    preview=control(L"ZzenPreview",L"",211);ocrBox=control(L"EDIT",L"Texto reconocido",212,ES_MULTILINE|ES_READONLY|WS_VSCROLL);logBox=control(L"EDIT",L"",213,ES_MULTILINE|ES_READONLY|WS_VSCROLL);debugInfo=label(L"Sin captura",210);
    debugControls={preview,ocrBox,logBox,debugInfo,button(L"Observar juego",115),button(L"Guardar diagnóstico",214)};
    auto help=control(L"EDIT",i18n::help().c_str(),220,ES_MULTILINE|ES_READONLY|WS_VSCROLL);helpControls={help};
    languageBox=control(L"COMBOBOX",L"",321,CBS_DROPDOWNLIST|CBS_OWNERDRAWFIXED|CBS_HASSTRINGS|WS_TABSTOP);
    for(auto name:i18n::languages)SendMessageW(languageBox,CB_ADDSTRING,0,(LPARAM)name);
    SendMessageW(languageBox,CB_SETCURSEL,i18n::language,0);label(L"Idioma de Zzen",320);
    for(auto h:{routeBox,teamBox,diffBox,languageBox}){SendMessageW(h,CB_SETITEMHEIGHT,WPARAM(-1),30);SendMessageW(h,CB_SETITEMHEIGHT,0,30);SetWindowSubclass(h,ComboProc,1,0);}
    SetWindowSubclass(ListView_GetHeader(historyList),HeaderProc,1,0);
    sessionControls.push_back(label(L"El juego debe estar en español",308));sessionControls.push_back(label(L"El idioma de la aplicación no cambia el idioma reconocido del juego.",309));
    statusBox=label(L"Listo");SendMessageW(statusBox,WM_SETFONT,(WPARAM)smallFont,TRUE);showPage();layout();localize();refresh();
}
LRESULT CALLBACK WindowProc(HWND h,UINT msg,WPARAM w,LPARAM l){
    try{switch(msg){
    case WM_CREATE: mainWindow=h;create();if(!RegisterHotKey(h,1,MOD_NOREPEAT,VK_F8))status(L"F8 está ocupado por otra aplicación; utiliza Detener.");SetTimer(h,1,150,nullptr);return 0;
    case WM_PAINT:paint();return 0;
    case WM_ERASEBKGND:return 1;
    case WM_SIZE:if(routeBox)layout();InvalidateRect(h,nullptr,TRUE);return 0;
    case WM_GETMINMAXINFO:((MINMAXINFO*)l)->ptMinTrackSize={1280,930};return 0;
    case WM_MEASUREITEM:((MEASUREITEMSTRUCT*)l)->itemHeight=32;return TRUE;
    case WM_DRAWITEM:if(((DRAWITEMSTRUCT*)l)->CtlType==ODT_COMBOBOX){drawCombo(*(DRAWITEMSTRUCT*)l);return TRUE;}if(((DRAWITEMSTRUCT*)l)->CtlType==ODT_BUTTON){drawButton(*(DRAWITEMSTRUCT*)l);return TRUE;}break;
    case WM_CTLCOLORBTN:case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:{
        HDC dc=(HDC)w;int id=GetDlgCtrlID((HWND)l);SetTextColor(dc,id==1||id==308?accent:((id>=301&&id<=307)||(id>=410&&id<=413)||id==2||id==320)?muted:ink);SetBkMode(dc,TRANSPARENT);SetBkColor(dc,panel);
        if(id>=301&&id<=306||id>=400&&id<=413||id==106||id==104||id==105||id>=1&&id<=3)return (LRESULT)fieldBrush;
        if(id==308||id==309){static HBRUSH banner=CreateSolidBrush(RGB(34,30,53));return (LRESULT)banner;}
        if((HWND)l==statusBox){static HBRUSH footer=CreateSolidBrush(RGB(24,28,38));return (LRESULT)footer;}
        return (LRESULT)(msg==WM_CTLCOLORSTATIC?background:fieldBrush);}
    case WM_NOTIFY:{auto n=(NMHDR*)l;
        if(n->hwndFrom==historyList&&n->code==LVN_ITEMCHANGED)showRecord();
        if(n->hwndFrom==ListView_GetHeader(historyList)&&n->code==NM_CUSTOMDRAW){auto d=(NMCUSTOMDRAW*)l;if(d->dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYITEMDRAW;if(d->dwDrawStage==CDDS_ITEMPREPAINT){FillRect(d->hdc,&d->rc,fieldBrush);wchar_t title[200]{};HDITEMW item{};item.mask=HDI_TEXT;item.pszText=title;item.cchTextMax=200;Header_GetItem(n->hwndFrom,int(d->dwItemSpec),&item);SetTextColor(d->hdc,muted);SetBkMode(d->hdc,TRANSPARENT);SelectObject(d->hdc,smallFont);RECT r=d->rc;r.left+=12;DrawTextW(d->hdc,title,-1,&r,DT_SINGLELINE|DT_VCENTER);return CDRF_SKIPDEFAULT;}}
        if(n->hwndFrom==historyList&&n->code==NM_CUSTOMDRAW){auto d=(NMLVCUSTOMDRAW*)l;if(d->nmcd.dwDrawStage==CDDS_PREPAINT)return CDRF_NOTIFYPOSTPAINT;if(d->nmcd.dwDrawStage==CDDS_POSTPAINT&&ListView_GetItemCount(historyList)==0){RECT r;GetClientRect(historyList,&r);r.top+=35;SetBkMode(d->nmcd.hdc,TRANSPARENT);SetTextColor(d->nmcd.hdc,muted);SelectObject(d->nmcd.hdc,smallFont);auto t=tr(L"ESPERANDO TU PRIMER PARTIDO");DrawTextW(d->nmcd.hdc,t.c_str(),-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);}}
        return 0;}
    case WM_COMMAND:{int id=LOWORD(w);
        if(id==321&&HIWORD(w)==CBN_SELCHANGE){i18n::language=int(SendMessageW(languageBox,CB_GETCURSEL,0,0));{std::lock_guard lock(dataMutex);store->config.language=i18n::language;store->profile();}localize();return 0;}
        if(id==101&&HIWORD(w)==CBN_SELCHANGE){int i=int(SendMessageW(routeBox,CB_GETCURSEL,0,0));wchar_t s[512];SendMessageW(routeBox,CB_GETLBTEXT,i,(LPARAM)s);catalogUI();}
        if(HIWORD(w)!=BN_CLICKED)return 0;
        if(id>=10&&id<=12){page=id-10;showPage();refresh();return 0;}
        switch(id){case 106:refresh();break;case 110:startMode(1);break;case 111:paused=true;status(L"Pausado por el usuario");break;
        case 112:if(active&&paused){gameWindow=game();ShowWindow(h,SW_MINIMIZE);SetForegroundWindow(gameWindow);SetTimer(h,2,2200,nullptr);}break;
        case 113:KillTimer(h,2);stop(L"Detenido por el usuario");break;case 114:startMode(2);break;case 115:startMode(3);break;
        case 204:correctResult();break;case 214:diagnostic();break;}refresh();return 0;}
    case WM_HOTKEY:KillTimer(h,2);stop(L"Detenido · F8");return 0;
    case WM_TIMER:if(w==2){KillTimer(h,2);if(active){paused=false;status(L"Reanudando");}}else if(active&&(GetAsyncKeyState(VK_F8)&0x8000))stop(L"Detenido · F8");return 0;
    case WM_APP+1:refresh();return 0;
    case WM_APP+2:ShowWindow(h,SW_RESTORE);SetForegroundWindow(h);refresh();return 0;
    case WM_APP+3:catalogUI();return 0;
    case WM_CLOSE:quitting=true;active=false;DestroyWindow(h);return 0;
    case WM_DESTROY:UnregisterHotKey(h,1);PostQuitMessage(0);return 0;
    }}catch(...){auto s=tr(exceptionText());MessageBoxW(h,s.c_str(),L"Zzen",MB_ICONINFORMATION);}
    return DefWindowProcW(h,msg,w,l);
}
}

int WINAPI wWinMain(HINSTANCE instance,HINSTANCE,PWSTR,int){
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    Gdiplus::GdiplusStartupInput gi;ULONG_PTR token;Gdiplus::GdiplusStartup(&token,&gi,nullptr);
    winrt::init_apartment(winrt::apartment_type::multi_threaded);int code=0;
    try{
        int argc;auto argv=CommandLineToArgvW(GetCommandLineW(),&argc);wchar_t exe[32768];GetModuleFileNameW(nullptr,exe,32768);fs::path root=fs::path(exe).parent_path();
        if(argc>=4&&std::wstring(argv[1])==L"--analyze"){
            Vision vision;auto im=Image::load(argv[2]);auto r=vision.observe(im);auto report=zzen::label(classify(r))+L"\r\n"+r.text;
            if(classify(r)==Screen::FinalScore){auto score=vision.score(im);report+=score?L"\r\nSCORE="+std::to_wstring(score->first)+L"-"+std::to_wstring(score->second):L"\r\nSCORE=unreadable";}writeText(argv[3],report);
        }else{
            HANDLE mutex=CreateMutexW(nullptr,FALSE,L"Local\\ZzenDesktop");if(GetLastError()==ERROR_ALREADY_EXISTS)throw std::runtime_error("Ya hay una edición de Zzen abierta. Ciérrala antes de continuar.");
            store=std::make_unique<Store>(root);INITCOMMONCONTROLSEX ic{sizeof ic,ICC_WIN95_CLASSES};InitCommonControlsEx(&ic);
            WNDCLASSW wc{};wc.hInstance=instance;wc.hIcon=LoadIconW(instance,MAKEINTRESOURCEW(1));wc.hCursor=LoadCursorW(nullptr,IDC_ARROW);wc.hbrBackground=ui::background;wc.lpfnWndProc=ui::PreviewProc;wc.lpszClassName=L"ZzenPreview";RegisterClassW(&wc);
            wc.lpfnWndProc=ui::WindowProc;wc.lpszClassName=L"ZzenFree";RegisterClassW(&wc);
            mainWindow=CreateWindowW(L"ZzenFree",L"Zzen · Free 0.2",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1400,990,nullptr,nullptr,instance,nullptr);
            BOOL dark=TRUE;DwmSetWindowAttribute(mainWindow,20,&dark,sizeof dark);ShowWindow(mainWindow,SW_SHOW);
            std::thread thread(worker);MSG msg;
            while(GetMessageW(&msg,nullptr,0,0)>0){if(!IsDialogMessageW(mainWindow,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
            quitting=true;thread.join();CloseHandle(mutex);
        }
        LocalFree(argv);
    }catch(...){auto s=exceptionText();MessageBoxW(nullptr,s.c_str(),L"Zzen",MB_ICONERROR);code=1;}
    winrt::uninit_apartment();Gdiplus::GdiplusShutdown(token);return code;
}
