 int categoryIndex=0,categoryChanges=0,selectedRow=-1;bool tabReady=false,targetOnMap=false;std::wstring category,lastSelection,scanId;std::set<std::wstring> completedCategories;
 std::wstring readCategory(const Reading&r){for(auto&l:r.lines)if(l.y>.21&&l.y<.252&&l.x>.09&&l.x<.4&&l.text.size()>4){auto name=trim(l.text);while(!name.empty()&&!iswalnum(name.back()))name.pop_back();return name;}return L"";}
 int selection(const Image& im){for(int i=0;i<9;i++){auto p=im.at(int(im.width*.31),int(im.height*(311.+68*i)/1080));if(p[0]>140&&p[1]<160&&p[2]<55)return i;}return -1;}
 std::wstring signature(const std::vector<Line>& list){std::wstring s;for(auto&l:list)s+=norm(l.text)+L"|";return s;}
 static std::wstring nameKey(std::wstring s){s=norm(s);std::erase_if(s,[](wchar_t c){return !iswalnum(c);});return s;}
 size_t mergePage(const std::vector<Line>&list){std::vector<std::wstring> names;for(auto&l:list)names.push_back(l.text);size_t overlap=0;for(size_t n=1;n<=std::min(scanned.size(),names.size());n++){size_t errors=0;for(size_t j=0;j<n;j++)errors+=nameKey(scanned[scanned.size()-n+j])!=nameKey(names[j]);if(errors==0||(n>=6&&errors<=2))overlap=n;}size_t offset=scanned.size()-overlap;for(size_t i=overlap;i<names.size();i++)scanned.push_back(names[i]);return offset;} void clearPage(){scrolls=stalePages=topPass=0;lastPage.clear();lastSelection.clear();scanned.clear();}
 // The hovered row is not the scroll position: the mouse may highlight any row
 // while the list is already at its beginning. Verify the scrollbar instead.
 std::optional<bool> listAtTop(const Image& im){
   if(im.width<100||im.height<100)return {};
   int start=-1,bestStart=-1,bestLength=0;
   for(int y=284;y<=885;++y){
     auto p=im.at(int(im.width*909./1920),int(im.height*y/1080.));
     auto edge=im.at(int(im.width*919./1920),int(im.height*y/1080.));
     bool thumb=std::min({p[0],p[1],p[2]})>225&&std::max({p[0],p[1],p[2]})-std::min({p[0],p[1],p[2]})<28&&std::min({edge[0],edge[1],edge[2]})<210;
     if(thumb){if(start<0)start=y;}else if(start>=0){if(y-start>bestLength){bestStart=start;bestLength=y-start;}start=-1;}
   }
   if(start>=0&&886-start>bestLength){bestStart=start;bestLength=886-start;}
   if(bestLength<20)return {};
   return bestStart<=296;
 }
 bool atTop(const Image&im,const Reading&r){
   if(topPass==2)return true;
   auto position=listAtTop(im);auto list=teams(r);
   if(position&&*position&&!list.empty()){
     if(++stalePages>=2){topPass=2;lastPage.clear();stalePages=0;scrolls=0;return true;}
     return false;
   }
   // Short categories have no scrollbar. Confirm an unchanged page after upward input.
   auto backdrop=im.at(int(im.width*.46),int(im.height*.75));
   if(!position&&!list.empty()&&list.size()<9&&backdrop[0]>backdrop[2]*1.2){
     auto sig=signature(list);if(sig==lastPage&&scrolls>0){topPass=2;lastPage.clear();stalePages=0;scrolls=0;return true;}
     lastPage=sig;
   }
   stalePages=0;
   if(position&&*position)return false; // Do not accept an unreadable/empty OCR page.
   if(scrolls>=24)throw std::runtime_error("No se ha localizado el comienzo de la lista.");
   if(action(L"Localizar comienzo de la lista",[]{scroll(120*5);}))++scrolls;
   return false;
 }
 void selectTeam(const Image&im,const Reading&r){if(Clock::now()-lastAction<std::chrono::milliseconds(1600))return;if(!tabReady){int index=-1;{std::lock_guard lock(dataMutex);for(auto&c:store->catalog)if(norm(c.route)==norm(cfg.route)){index=c.tab;break;}}if(index<0){if(norm(cfg.route)==L"inazuma eleven")index=0;else throw std::runtime_error("Primero barre Crónica para localizar esta categoría.");}action(L"Abrir categoría seleccionada",[=]{click((120.+80*index)/1920,193./1080);});tabReady=true;clearPage();return;}if(!atTop(im,r))return; auto list=teams(r);if(list.empty())return;size_t offset=mergePage(list);int occurrence=0;for(size_t i=0;i<scanned.size();i++)if(nameKey(scanned[i])==nameKey(cfg.team)&&++occurrence==cfg.occurrence&&i>=offset&&i-offset<list.size()){if(tap(&list[i-offset],L"Seleccionar "+cfg.team+L" · aparición "+std::to_wstring(cfg.occurrence)))targetOnMap=true;return;}
 auto page=signature(list);if(page==lastPage)++stalePages;else stalePages=0;lastPage=page;if(stalePages>12||++scrolls>200)throw std::runtime_error("Rival no encontrado en esta categoría.");action(L"Buscar rival más abajo",[]{scroll(-120);});}
 void scan(const Image&im,const Reading&r,Screen screen){if(screen==Screen::DropRates||screen==Screen::Difficulty){action(L"Volver al mapa para actualizar catálogo",[]{key(VK_ESCAPE);});return;}if(screen==Screen::RouteMenu){action(L"Cerrar menú de Crónica",[]{key(VK_ESCAPE);});return;}if(screen==Screen::RouteMap){action(L"Abrir Cadena de Crónica",[]{key('V');});return;}if(screen!=Screen::TeamList){status(L"Barrer Crónica: abre su mapa o la lista de equipos");return;}if(Clock::now()-lastAction<std::chrono::milliseconds(1600))return;
 if(!tabReady){action(L"Comenzar por la primera pestaña",[]{click(120./1920,193./1080);});tabReady=true;categoryIndex=0;scanId=stamp(true);clearPage();return;}
 auto cat=readCategory(r);if(cat.empty())throw std::runtime_error("Categoría ilegible. Consulta Debug.");static const wchar_t* categoryNames[]={L"Inazuma Eleven",L"Inazuma Eleven 2",L"Inazuma Eleven 3",L"Inazuma Eleven GO",L"Inazuma Eleven GO 2",L"Inazuma Eleven GO Galaxy",L"Inazuma Eleven: Ares",L"Inazuma Eleven: Orión",L"Inazuma Eleven: Heroes' Victory Road"};category=categoryNames[categoryIndex];cat=category;
 if(!atTop(im,r))return;auto list=teams(r);if(list.empty()){status(L"Esperando equipos de "+cat);return;}auto page=signature(list);int row=selection(im);auto stateKey=page+L"#"+std::to_wstring(row);if(stateKey==lastSelection)++stalePages;else stalePages=0;lastSelection=stateKey;mergePage(list);
 if(page!=lastPage){auto path=store->root/L"evidence"/scanId/(L"categoria-"+std::to_wstring(categoryIndex+1));fs::create_directories(path);im.save(path/(std::to_wstring(scrolls)+L".png"));writeText(path/(std::to_wstring(scrolls)+L".txt"),r.text);lastPage=page;}
 if(stalePages>=3){if(row<0&&stalePages<12){++scrolls;action(L"Verificar final de lista",[]{scroll(-120);});return;}{std::lock_guard lock(dataMutex);std::erase_if(store->catalog,[&](const auto&c){return c.tab==categoryIndex;});std::map<std::wstring,int> counts;for(auto&name:scanned)store->catalog.push_back({cat,name,++counts[norm(name)],categoryIndex});store->saveCatalog();}completedCategories.insert(norm(cat));logText(L"CATÁLOGO · "+cat+L" · "+std::to_wstring(scanned.size())+L" partidos");PostMessageW(mainWindow,WM_APP+3,0,0);if(++categoryIndex>=9){size_t count;{std::lock_guard lock(dataMutex);count=store->catalog.size();}stop(L"Barrido completo · "+std::to_wstring(count)+L" partidos en 9 categorías");return;}action(L"Examinar siguiente categoría de Crónica",[]{key('C');});category.clear();clearPage();return;}
 if(++scrolls>220)throw std::runtime_error("Lista demasiado larga o inestable; se conserva el catálogo anterior.");entered=Clock::now();status(L"Barrer Crónica · "+cat+L" · "+std::to_wstring(scanned.size())+L" partidos");action(L"Leer siguiente posición de la lista",[]{scroll(-120);});}
