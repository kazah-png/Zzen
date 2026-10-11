#pragma once
#include <uxtheme.h>
#pragma comment(lib,"uxtheme.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

namespace ui {
constexpr COLORREF bg=RGB(14,19,28), panel=RGB(22,30,42), ink=RGB(232,240,247), muted=RGB(147,165,184), accent=RGB(99,230,180), line=RGB(43,57,73);
HBRUSH background=CreateSolidBrush(bg), fieldBrush=CreateSolidBrush(panel);
HFONT font,smallFont,titleFont,numberFont;
HWND routeBox,teamBox,diffBox,countBox,occBox,infiniteBox,statusBox,pageTitle,pageSubtitle;
HWND historyList,details,preview,ocrBox,logBox,debugInfo;
HWND statNumber[4],statLabel[4];
std::vector<HWND> sessionControls,debugControls,helpControls;
int page=0;bool refreshing=false;std::wstring historyKey,lastOCR,lastLog;
std::wstring value(HWND h){int n=GetWindowTextLengthW(h);std::wstring s(n+1,L'\0');GetWindowTextW(h,s.data(),n+1);s.resize(n);return s;}
void text(HWND h,const std::wstring& s){if(value(h)!=s)SetWindowTextW(h,s.c_str());}
HWND control(const wchar_t* cls,const wchar_t* text,int id,DWORD style=0){
    HWND c=CreateWindowExW(0,cls,text,WS_CHILD|WS_VISIBLE|style,0,0,10,10,mainWindow,(HMENU)(INT_PTR)id,GetModuleHandleW(nullptr),nullptr);
    SendMessageW(c,WM_SETFONT,(WPARAM)font,TRUE);return c;
}
HWND label(const wchar_t* text,int id=0){return control(L"STATIC",text,id);}
HWND button(const wchar_t* text,int id){return control(L"BUTTON",text,id,BS_OWNERDRAW|WS_TABSTOP);}
void place(HWND h,int x,int y,int w,int hh){MoveWindow(h,x,y,w,hh,TRUE);}
void placeID(int id,int x,int y,int w,int h){place(GetDlgItem(mainWindow,id),x,y,w,h);}
std::wstring crlf(std::wstring s){size_t p=0;while((p=s.find(L'\n',p))!=s.npos){if(p==0||s[p-1]!=L'\r'){s.insert(p,L"\r");++p;}++p;}return s;}
int selected(){return ListView_GetNextItem(historyList,-1,LVNI_SELECTED);}

void catalogUI(){
    std::vector<CatalogEntry> entries;{std::lock_guard lock(dataMutex);entries=store->catalog;}
    auto route=value(routeBox),team=value(teamBox);std::vector<std::wstring> routes;std::set<std::wstring> teams;
    for(auto& c:entries){if(std::find(routes.begin(),routes.end(),c.route)==routes.end())routes.push_back(c.route);if(c.route==route)teams.insert(c.team);}
    SendMessageW(routeBox,CB_RESETCONTENT,0,0);SendMessageW(teamBox,CB_RESETCONTENT,0,0);
    for(auto& s:routes)SendMessageW(routeBox,CB_ADDSTRING,0,(LPARAM)s.c_str());
    for(auto& s:teams)SendMessageW(teamBox,CB_ADDSTRING,0,(LPARAM)s.c_str());
    if(!teams.empty()&&!teams.contains(team))team=*teams.begin();
    SetWindowTextW(routeBox,route.c_str());SetWindowTextW(teamBox,team.c_str());
}
void saveConfig(){
    Config c;c.route=trim(value(routeBox));c.team=trim(value(teamBox));
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
    text(details,r.outcome+L"  "+r.score+L"  ·  "+r.team+L"  ·  "+r.difficulty+L"\r\n"+(r.outcome==L"Pendiente"?L"Revisa Debug o la captura antes de corregir el resultado.":L"Los resultados de Gratis se conservan solo durante esta sesión."));
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
            std::wstring cells[]={r.team,r.difficulty,r.outcome,r.score};for(int j=0;j<4;++j)ListView_SetItemText(historyList,int(i),j+1,cells[j].data());}
        historyKey=key;refreshing=false;
        if(previous>=0&&size_t(previous)<rows.size())ListView_SetItemState(historyList,previous,LVIS_SELECTED,LVIS_SELECTED);
        if(rows.empty())text(details,L"Sin partidos todavía. Abre Crónica en el juego y pulsa Iniciar sesión.");
    }
    if(page==1){
        if(reading!=lastOCR){lastOCR=reading;text(ocrBox,crlf(reading));}
        if(log!=lastLog){lastLog=log;text(logBox,log);SendMessageW(logBox,EM_SETSEL,log.size(),log.size());SendMessageW(logBox,EM_SCROLLCARET,0,0);}
        text(debugInfo,zzen::label(screen)+L"   ·   "+std::to_wstring(elapsed)+L" ms\r\n"+action);InvalidateRect(preview,nullptr,FALSE);
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
    HMENU menu=CreatePopupMenu();AppendMenuW(menu,MF_STRING,1,L"Victoria");AppendMenuW(menu,MF_STRING,2,L"Derrota");AppendMenuW(menu,MF_STRING,3,L"Empate");
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
        else{SetTextColor(dc,muted);SetBkMode(dc,TRANSPARENT);SelectObject(dc,font);DrawTextW(dc,L"La captura del juego aparecerá aquí",-1,&rect,DT_CENTER|DT_VCENTER|DT_SINGLELINE);}}
    EndPaint(h,&ps);return 0;
}
void layout(){
    RECT r;GetClientRect(mainWindow,&r);int x=218,w=r.right-x-28,h=r.bottom;
    placeID(1,24,28,150,48);placeID(2,26,78,140,22);placeID(10,20,145,168,44);placeID(11,20,201,168,44);placeID(12,20,257,168,44);placeID(3,26,h-105,158,52);
    place(pageTitle,x,29,w,36);place(pageSubtitle,x,73,w,25);
    int gap=18,a=(w-2*gap)/3;
    for(int i=0;i<3;++i)placeID(301+i,x+i*(a+gap),129,a,22);
    place(routeBox,x,158,a,280);place(teamBox,x+a+gap,158,a,310);place(diffBox,x+2*(a+gap),158,a,260);
    placeID(304,x,209,125,22);place(countBox,x,237,110,28);placeID(305,x+140,209,135,22);place(occBox,x+140,237,100,28);
    place(infiniteBox,x+270,237,180,30);placeID(306,x+470,210,w-470,57);
    placeID(110,x,291,190,43);placeID(111,x+204,291,112,43);placeID(112,x+328,291,126,43);placeID(113,x+w-145,291,145,43);
    int cw=(w-54)/4;for(int i=0;i<4;++i){place(statNumber[i],x+i*(cw+18)+16,368,cw-32,40);place(statLabel[i],x+i*(cw+18)+16,414,cw-32,22);}
    placeID(307,x,471,w-230,24);placeID(114,x+w-211,463,211,36);
    place(historyList,x,509,w,std::max(100,h-679));place(details,x,h-151,w-210,60);placeID(204,x+w-195,h-141,195,37);
    place(debugInfo,x,126,w-230,50);placeID(115,x+w-215,124,215,40);
    int pw=(w-18)*2/3;place(preview,x,194,pw,(h-360)*2/3);place(ocrBox,x+pw+18,194,w-pw-18,h-301);
    int ly=210+(h-360)*2/3;place(logBox,x,ly,pw,h-ly-107);placeID(214,x,h-88,210,36);
    placeID(220,x,128,w,h-216);place(statusBox,x,h-40,w,30);
    int cols[]={170,(w-510),125,120,90};for(int i=0;i<5;++i)ListView_SetColumnWidth(historyList,i,cols[i]);
}
void paint(){
    PAINTSTRUCT ps;HDC dc=BeginPaint(mainWindow,&ps);RECT r;GetClientRect(mainWindow,&r);FillRect(dc,&r,background);
    RECT sidebar{0,0,198,r.bottom};FillRect(dc,&sidebar,fieldBrush);HPEN pen=CreatePen(PS_SOLID,1,line);auto old=SelectObject(dc,pen);MoveToEx(dc,198,0,nullptr);LineTo(dc,198,r.bottom);
    if(page==0){int x=218,w=r.right-x-28,cw=(w-54)/4;for(int i=0;i<4;++i){RECT card{x+i*(cw+18),353,x+i*(cw+18)+cw,447};auto brush=(HBRUSH)SelectObject(dc,background);RoundRect(dc,card.left,card.top,card.right,card.bottom,12,12);SelectObject(dc,brush);}}
    SelectObject(dc,old);DeleteObject(pen);EndPaint(mainWindow,&ps);
}
void drawButton(const DRAWITEMSTRUCT& d){
    bool nav=d.CtlID>=10&&d.CtlID<=12, chosen=nav&&int(d.CtlID)-10==page, primary=d.CtlID==110;
    bool disabled=(d.itemState&ODS_DISABLED)!=0;COLORREF fill=primary?accent:chosen?RGB(37,67,63):panel;
    if(disabled)fill=RGB(25,33,44);if(d.itemState&ODS_SELECTED)fill=RGB(51,102,83);
    FillRect(d.hDC,&d.rcItem,background);HBRUSH b=CreateSolidBrush(fill);HPEN p=CreatePen(PS_SOLID,1,chosen?accent:line);auto ob=SelectObject(d.hDC,b),op=SelectObject(d.hDC,p);
    RoundRect(d.hDC,d.rcItem.left,d.rcItem.top,d.rcItem.right,d.rcItem.bottom,10,10);SelectObject(d.hDC,ob);SelectObject(d.hDC,op);DeleteObject(b);DeleteObject(p);
    SetBkMode(d.hDC,TRANSPARENT);SetTextColor(d.hDC,disabled?RGB(91,110,126):primary?bg:chosen?accent:ink);SelectObject(d.hDC,font);RECT r=d.rcItem;
    auto t=value(d.hwndItem);DrawTextW(d.hDC,t.c_str(),-1,&r,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    if(d.itemState&ODS_FOCUS){InflateRect(&r,-4,-4);DrawFocusRect(d.hDC,&r);}
}
void create(){
    font=CreateFontW(-16,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    smallFont=CreateFontW(-14,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    titleFont=CreateFontW(-30,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    numberFont=CreateFontW(-29,0,0,0,FW_SEMIBOLD,0,0,0,DEFAULT_CHARSET,0,0,CLEARTYPE_QUALITY,0,L"Segoe UI");
    auto brand=label(L"Zzen",1);SendMessageW(brand,WM_SETFONT,(WPARAM)titleFont,TRUE);label(L"GRATIS  /  0.1",2);label(L"Solo Crónica\r\nF8 detiene la sesión",3);
    button(L"Sesión",10);button(L"Diagnóstico",11);button(L"Cómo usar Zzen",12);
    pageTitle=label(L"");SendMessageW(pageTitle,WM_SETFONT,(WPARAM)titleFont,TRUE);pageSubtitle=label(L"");SendMessageW(pageSubtitle,WM_SETFONT,(WPARAM)smallFont,TRUE);
    const wchar_t* captions[]={L"Categoría",L"Rival",L"Dificultad",L"Partidos",L"Aparición del rival",L"Si un rival aparece varias veces,\r\nelige su número en la ruta.",L"PARTIDOS DE ESTA SESIÓN"};
    for(int i=0;i<7;++i)sessionControls.push_back(label(captions[i],301+i));
    routeBox=control(L"COMBOBOX",L"",101,CBS_DROPDOWN|WS_VSCROLL|WS_TABSTOP);teamBox=control(L"COMBOBOX",L"",102,CBS_DROPDOWN|WS_VSCROLL|WS_TABSTOP);
    diffBox=control(L"COMBOBOX",L"",103,CBS_DROPDOWNLIST|WS_TABSTOP);countBox=control(L"EDIT",L"1",104,ES_NUMBER|WS_BORDER|WS_TABSTOP);occBox=control(L"EDIT",L"1",105,ES_NUMBER|WS_BORDER|WS_TABSTOP);
    infiniteBox=control(L"BUTTON",L"Sin límite",106,BS_AUTOCHECKBOX|WS_TABSTOP);SetWindowTheme(infiniteBox,L"",L"");
    for(auto h:{routeBox,teamBox,diffBox,countBox,occBox,infiniteBox})sessionControls.push_back(h);
    for(int i=0;i<6;++i){auto s=difficultyName(i);SendMessageW(diffBox,CB_ADDSTRING,0,(LPARAM)s.c_str());}
    auto& c=store->config;SetWindowTextW(routeBox,c.route.c_str());SetWindowTextW(teamBox,c.team.c_str());SetWindowTextW(countBox,std::to_wstring(c.count).c_str());SetWindowTextW(occBox,std::to_wstring(c.occurrence).c_str());SendMessageW(diffBox,CB_SETCURSEL,c.difficulty,0);SendMessageW(infiniteBox,BM_SETCHECK,c.infinite?BST_CHECKED:BST_UNCHECKED,0);catalogUI();
    for(auto pair:{std::pair{110,L"Iniciar sesión"},std::pair{111,L"Pausar"},std::pair{112,L"Reanudar"},std::pair{113,L"Detener · F8"},std::pair{114,L"Actualizar catálogo"},std::pair{204,L"Corregir resultado"}})sessionControls.push_back(button(pair.second,pair.first));
    const wchar_t* names[]={L"COMPLETADOS",L"VICTORIAS",L"DERROTAS",L"EMPATES / PENDIENTES"};
    for(int i=0;i<4;++i){statNumber[i]=label(L"0",400+i);statLabel[i]=label(names[i],410+i);SendMessageW(statNumber[i],WM_SETFONT,(WPARAM)numberFont,TRUE);SendMessageW(statLabel[i],WM_SETFONT,(WPARAM)smallFont,TRUE);sessionControls.push_back(statNumber[i]);sessionControls.push_back(statLabel[i]);}
    historyList=control(WC_LISTVIEWW,L"",201,LVS_REPORT|LVS_SINGLESEL|WS_TABSTOP);ListView_SetExtendedListViewStyle(historyList,LVS_EX_FULLROWSELECT|LVS_EX_DOUBLEBUFFER);
    ListView_SetBkColor(historyList,panel);ListView_SetTextBkColor(historyList,panel);ListView_SetTextColor(historyList,ink);SetWindowTheme(historyList,L"Explorer",nullptr);
    const wchar_t* cols[]={L"Fecha",L"Rival",L"Dificultad",L"Resultado",L"Marcador"};for(int i=0;i<5;++i){LVCOLUMNW col{};col.mask=LVCF_TEXT|LVCF_WIDTH;col.cx=150;col.pszText=const_cast<wchar_t*>(cols[i]);ListView_InsertColumn(historyList,i,&col);}sessionControls.push_back(historyList);
    details=control(L"EDIT",L"Sin partidos todavía. Abre Crónica en el juego y pulsa Iniciar sesión.",207,ES_MULTILINE|ES_READONLY);sessionControls.push_back(details);
    preview=control(L"ZzenPreview",L"",211);ocrBox=control(L"EDIT",L"Texto reconocido",212,ES_MULTILINE|ES_READONLY|WS_VSCROLL);logBox=control(L"EDIT",L"",213,ES_MULTILINE|ES_READONLY|WS_VSCROLL);debugInfo=label(L"Sin captura",210);
    debugControls={preview,ocrBox,logBox,debugInfo,button(L"Observar juego",115),button(L"Guardar diagnóstico",214)};
    auto help=control(L"EDIT",L"01   PREPARA EL JUEGO\r\n\r\nAbre Crónica con la interfaz en español y controles de teclado y ratón.\r\nMantén el juego visible; cambiar de ventana pausa Zzen.\r\n\r\n02   ELIGE TU SESIÓN\r\n\r\nSelecciona categoría, rival y dificultad. Indica cuántos partidos quieres\r\njugar o marca Sin límite. Aparición distingue nombres repetidos.\r\nActualizar catálogo recorre las ocho pestañas de Crónica.\r\n\r\n03   INICIA Y CONTROLA\r\n\r\nIniciar sesión busca el rival, activa Comandante y repite con Revancha.\r\nF8 detiene. Reanudar continúa una sesión pausada.\r\nLos contadores de Gratis se reinician al empezar otra sesión o cerrar Zzen.\r\n\r\nSI ALGO NO AVANZA\r\n\r\nDiagnóstico muestra la captura, el texto detectado y la última acción.\r\nObservar juego analiza sin enviar entradas. Guardar diagnóstico conserva\r\nuna captura y el registro. Un marcador ilegible queda Pendiente; revísalo\r\nantes de corregir su resultado.\r\n\r\nGratis no identifica ni acumula recompensas. No requiere cuenta ni licencia.\r\nSolo una edición de Zzen puede estar abierta a la vez.",220,ES_MULTILINE|ES_READONLY|WS_VSCROLL);helpControls={help};
    statusBox=label(L"Listo");SendMessageW(statusBox,WM_SETFONT,(WPARAM)smallFont,TRUE);showPage();layout();refresh();
}
LRESULT CALLBACK WindowProc(HWND h,UINT msg,WPARAM w,LPARAM l){
    try{switch(msg){
    case WM_CREATE: mainWindow=h;create();if(!RegisterHotKey(h,1,MOD_NOREPEAT,VK_F8))status(L"F8 está ocupado por otra aplicación; utiliza Detener.");SetTimer(h,1,150,nullptr);return 0;
    case WM_PAINT:paint();return 0;
    case WM_ERASEBKGND:return 1;
    case WM_SIZE:if(routeBox)layout();InvalidateRect(h,nullptr,TRUE);return 0;
    case WM_GETMINMAXINFO:((MINMAXINFO*)l)->ptMinTrackSize={1120,850};return 0;
    case WM_DRAWITEM:if(((DRAWITEMSTRUCT*)l)->CtlType==ODT_BUTTON){drawButton(*(DRAWITEMSTRUCT*)l);return TRUE;}break;
    case WM_CTLCOLORBTN:case WM_CTLCOLORSTATIC:case WM_CTLCOLOREDIT:case WM_CTLCOLORLISTBOX:{HDC dc=(HDC)w;SetTextColor(dc,ink);SetBkColor(dc,panel);if(msg==WM_CTLCOLORSTATIC||msg==WM_CTLCOLORBTN){int id=GetDlgCtrlID((HWND)l);SetBkMode(dc,TRANSPARENT);return (LRESULT)(id>=1&&id<=3?fieldBrush:background);}return (LRESULT)fieldBrush;}
    case WM_NOTIFY:{auto n=(NMHDR*)l;if(n->hwndFrom==historyList&&n->code==LVN_ITEMCHANGED)showRecord();return 0;}
    case WM_COMMAND:{int id=LOWORD(w);
        if(id==101&&HIWORD(w)==CBN_SELCHANGE){int i=int(SendMessageW(routeBox,CB_GETCURSEL,0,0));wchar_t s[512];SendMessageW(routeBox,CB_GETLBTEXT,i,(LPARAM)s);SetWindowTextW(routeBox,s);catalogUI();}
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
    }}catch(...){auto s=exceptionText();MessageBoxW(h,s.c_str(),L"Zzen",MB_ICONINFORMATION);}
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
            mainWindow=CreateWindowW(L"ZzenFree",L"Zzen Gratis · 0.1",WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,1240,900,nullptr,nullptr,instance,nullptr);
            BOOL dark=TRUE;DwmSetWindowAttribute(mainWindow,20,&dark,sizeof dark);ShowWindow(mainWindow,SW_SHOW);
            std::thread thread(worker);MSG msg;
            while(GetMessageW(&msg,nullptr,0,0)>0){if(!IsDialogMessageW(mainWindow,&msg)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
            quitting=true;thread.join();CloseHandle(mutex);
        }
        LocalFree(argv);
    }catch(...){auto s=exceptionText();MessageBoxW(nullptr,s.c_str(),L"Zzen",MB_ICONERROR);code=1;}
    winrt::uninit_apartment();Gdiplus::GdiplusShutdown(token);return code;
}
