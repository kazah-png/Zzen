#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Security.Cryptography.h>
#include <filesystem>
#include <queue>
#include "core.hpp"
namespace zzen {
struct Image{int width=0,height=0;std::vector<unsigned char> pixels;
 Image()=default;Image(int w,int h):width(w),height(h),pixels(size_t(w)*h*4,255){}
 unsigned char* at(int x,int y){return pixels.data()+(size_t(y)*width+x)*4;}
 const unsigned char* at(int x,int y)const{return pixels.data()+(size_t(y)*width+x)*4;}
 Image crop(double x,double y,double w,double h,int targetWidth=0)const{int iw=targetWidth?targetWidth:int(width*w),ih=int(iw*h*height/(w*width));Image out(iw,ih);for(int yy=0;yy<ih;yy++)for(int xx=0;xx<iw;xx++){int sx=std::clamp(int(width*(x+w*xx/iw)),0,width-1),sy=std::clamp(int(height*(y+h*yy/ih)),0,height-1);memcpy(out.at(xx,yy),at(sx,sy),4);}return out;}
 void save(const std::filesystem::path& path)const{Gdiplus::Bitmap b(width,height,width*4,PixelFormat32bppARGB,const_cast<unsigned char*>(pixels.data()));CLSID png{0x557cf406,0x1a04,0x11d3,{0x9a,0x73,0,0,0xf8,0x1e,0xf3,0x2e}};if(b.Save(path.c_str(),&png)!=Gdiplus::Ok)throw std::runtime_error("No se pudo guardar la captura");}
 static Image load(const std::filesystem::path& p){Gdiplus::Bitmap b(p.c_str());if(b.GetLastStatus()!=Gdiplus::Ok)throw std::runtime_error("No se pudo abrir la imagen");Image out(b.GetWidth(),b.GetHeight());Gdiplus::BitmapData data{};Gdiplus::Rect r(0,0,out.width,out.height);b.LockBits(&r,Gdiplus::ImageLockModeRead,PixelFormat32bppARGB,&data);for(int y=0;y<out.height;y++)memcpy(out.at(0,y),(unsigned char*)data.Scan0+y*data.Stride,size_t(out.width)*4);b.UnlockBits(&data);return out;}
 static Image capture(HWND h){RECT r{};GetClientRect(h,&r);POINT p{};ClientToScreen(h,&p);if(r.right<100||r.bottom<100)throw std::runtime_error("Ventana no disponible");Image out(r.right,r.bottom);HDC screen=GetDC(nullptr),dc=CreateCompatibleDC(screen);BITMAPINFO info{};info.bmiHeader={sizeof(BITMAPINFOHEADER),out.width,-out.height,1,32,BI_RGB};void* bits=nullptr;HBITMAP bmp=CreateDIBSection(screen,&info,DIB_RGB_COLORS,&bits,nullptr,0);auto old=SelectObject(dc,bmp);BOOL ok=BitBlt(dc,0,0,out.width,out.height,screen,p.x,p.y,SRCCOPY);if(ok)memcpy(out.pixels.data(),bits,out.pixels.size());SelectObject(dc,old);DeleteObject(bmp);DeleteDC(dc);ReleaseDC(nullptr,screen);if(!ok)throw std::runtime_error("Error de captura");for(size_t i=3;i<out.pixels.size();i+=4)out.pixels[i]=255;return out;}
};
class Vision{
 winrt::Windows::Media::Ocr::OcrEngine engine{nullptr};
public:
 Vision(){engine=winrt::Windows::Media::Ocr::OcrEngine::TryCreateFromUserProfileLanguages();if(!engine)throw std::runtime_error("Activa un idioma de reconocimiento de texto de Windows.");}
 Reading read(const Image& im){using namespace winrt::Windows;auto buffer=Security::Cryptography::CryptographicBuffer::CreateFromByteArray(im.pixels);Graphics::Imaging::SoftwareBitmap bitmap(Graphics::Imaging::BitmapPixelFormat::Bgra8,im.width,im.height,Graphics::Imaging::BitmapAlphaMode::Ignore);bitmap.CopyFromBuffer(buffer);auto result=engine.RecognizeAsync(bitmap).get();Reading r;for(auto l:result.Lines()){Line line;line.text=l.Text();double left=im.width,top=im.height,right=0,bottom=0;for(auto word:l.Words()){auto rect=word.BoundingRect();left=std::min(left,double(rect.X));top=std::min(top,double(rect.Y));right=std::max(right,double(rect.X+rect.Width));bottom=std::max(bottom,double(rect.Y+rect.Height));}line.x=left/im.width;line.y=top/im.height;line.w=(right-left)/im.width;line.h=(bottom-top)/im.height;r.lines.push_back(line);r.text+=line.text+L"\n";}return r;}
 Reading observe(const Image& im){auto r=read(im);
 if(find(r,L"guardar",.68,1,0,.45)){
  auto opponent=read(im.crop(.43,.30,.18,.09,1000));r.text+=L"\n[Chronicle opponent]\n"+opponent.text;
  auto menu=read(im.crop(.09,.43,.28,.42,1150));
  for(auto l:menu.lines){l.x=.09+l.x*.28;l.y=.43+l.y*.42;l.w*=.28;l.h*=.42;r.lines.push_back(l);}r.text+=L"\n[Difficulty menu]\n"+menu.text;
  // Stylized gate titles can be unreadable. Verify the three-row gate layout
  // with text anchors before deriving its two known entries from row spacing.
  auto save=find(r,L"guardar",.68,1,0,.45);auto legend=find(r,L"legendaria",.5,.8,0,.4);auto squad=find(r,L"plantilla de",.38,.52,0,.4);
  if(save&&legend&&squad&&(has(r.text,L"puert")||(has(r.text,L"camino")&&has(r.text,L"desafios")))){
   double end=save->y+save->h/2;double step=(end-(squad->y+squad->h/2))/4;
   if(step>.05&&step<.08&&std::abs(end-step-(legend->y+legend->h/2))<.015){double center=end-step;
    r.lines.insert(r.lines.begin(),{L"BATALLA DE PUERTA+",.14,center-step-.017,.18,.034});
    r.lines.insert(r.lines.begin(),{L"BATALLA DE PUERTA",.14,center-2*step-.017,.18,.034});
   }
  }else if(save&&legend&&squad&&has(r.text,L"inusuales")){
   // Six-difficulty Chronicle layout: verify all anchors before deriving rows.
   double end=save->y+save->h/2;double step=(end-(squad->y+squad->h/2))/7;
   auto unusual=find(r,L"inusuales",.5,.7,0,.4);
   if(step>.05&&step<.08&&std::abs(end-step-(legend->y+legend->h/2))<.02&&unusual&&std::abs(end-4*step-(unusual->y+unusual->h/2))<.02){
    const wchar_t* names[]={L"BATALLA DE CRÓNICA",L"BATALLA DE DESBLOQUEO DE RUTA",L"BATALLA DE OBJETOS INUSUALES",L"BATALLA HEROICA",L"BATALLA HEROICA+",L"BATALLA LEGENDARIA"};
    for(int d=0;d<6;++d)r.lines.insert(r.lines.begin(),{names[d],.14,end-(6-d)*step-.017,.18,.034});
   }
  }

 }
auto bottom=read(im.crop(0,.875,1,.125));r.bottom=bottom.text;for(auto l:bottom.lines){l.y=.875+l.y*.125;l.h*=.125;r.lines.push_back(l);}r.text+=L"\n[Franja inferior]\n"+r.bottom;if(has(r.text,L"elige un equipo al que")){std::erase_if(r.lines,[](const Line& l){return l.x<.48&&l.y>.21&&l.y<.85;});auto list=read(im.crop(.057,.213,.32,.614,1000));for(auto l:list.lines){l.x=.057+l.x*.32;l.y=.213+l.y*.614;l.w*=.32;l.h*=.614;r.lines.push_back(l);}r.text+=L"\n[Lista ampliada]\n"+list.text;}if(classify(r)==Screen::PreMatch){auto rival=read(im.crop(.615,.151,.265,.077,1200));r.text+=L"\n[Rival ampliado]\n"+rival.text;for(auto l:rival.lines){l.x=.615+l.x*.265;l.y=.151+l.y*.077;l.w*=.265;l.h*=.077;r.lines.push_back(l);}}return r;}
 std::optional<int> digit(const Image& im,int side){auto crop=im.crop(side==0?550./1920:1090./1920,140./1080,side==0?280./1920:300./1920,250./1080,side==0?280:300);int w=crop.width,h=crop.height;std::vector<bool> mask(w*h),ink(w*h);for(int y=0;y<h;y++)for(int x=0;x<w;x++){auto c=crop.at(x,y);int b=c[0],g=c[1],red=c[2];mask[y*w+x]=side==0?(b>100&&g>100&&red<g*.8&&red<b*.8):(red>100&&red>g*1.3&&red>b*1.3);}
  int top=h,bottom=0,left=w,right=0;for(int y=0;y<h;y++)for(int x=0;x<w;x++){for(int dy=-2;dy<=2&&!ink[y*w+x];dy++)for(int dx=-2;dx<=2;dx++){int xx=x+dx,yy=y+dy;if(xx>=0&&xx<w&&yy>=0&&yy<h&&mask[yy*w+xx])ink[y*w+x]=true;}if(ink[y*w+x]){top=std::min(top,y);bottom=std::max(bottom,y);left=std::min(left,x);right=std::max(right,x);}}
  if(bottom-top<70||right-left<20)return {};bool zero=false;std::vector<bool> seen(w*h);for(int y=0;y<h;y++)for(int x=0;x<w;x++){int i=y*w+x;if(seen[i]||ink[i])continue;std::queue<int> q;q.push(i);seen[i]=true;int lx=w,rx=0,ty=h,by=0;bool edge=false;std::vector<int> points;while(!q.empty()){int p=q.front();q.pop();points.push_back(p);int px=p%w,py=p/w;lx=std::min(lx,px);rx=std::max(rx,px);ty=std::min(ty,py);by=std::max(by,py);edge|=px==0||py==0||px==w-1||py==h-1;int dx[4]={1,-1,0,0},dy[4]={0,0,1,-1};for(int j=0;j<4;j++){int xx=px+dx[j],yy=py+dy[j];if(xx>=0&&xx<w&&yy>=0&&yy<h&&!seen[yy*w+xx]&&!ink[yy*w+xx]){seen[yy*w+xx]=true;q.push(yy*w+xx);}}}if(!edge){double center=(ty+by)/2.;if(by-ty>(bottom-top)*.55&&rx-lx>(right-left)*.2&&center>top+(bottom-top)*.35&&center<top+(bottom-top)*.65)zero=true;if(points.size()<300)for(int p:points)ink[p]=true;}}
  Image canvas(600,185);Gdiplus::Bitmap b(600,185,2400,PixelFormat32bppARGB,canvas.pixels.data());Gdiplus::Graphics g(&b);Gdiplus::Font font(L"Arial",90,Gdiplus::FontStyleRegular,Gdiplus::UnitPixel);Gdiplus::SolidBrush brush(Gdiplus::Color(255,0,0,0));g.DrawString(L"GOLES",-1,&font,Gdiplus::PointF(20,15),&brush);for(int y=0;y<125;y++)for(int x=0;x<w/2;x++){auto c=canvas.at(390+x,18+y);if(ink[std::min(h-1,y*2)*w+std::min(w-1,x*2)])c[0]=c[1]=c[2]=0;}
  auto text=norm(read(canvas).text);auto pos=text.find(L"goles");if(pos!=text.npos){auto n=trim(text.substr(pos+5));if(n==L"o")return 0;if(n==L"q")return 4;if(!n.empty()&&n.size()<=2&&std::all_of(n.begin(),n.end(),iswdigit))return std::stoi(n);}if(zero)return 0;return {};
 }
 std::optional<std::pair<int,int>> score(const Image& im){auto a=digit(im,0),b=digit(im,1);if(a&&b)return std::pair{*a,*b};return {};}
};
}
