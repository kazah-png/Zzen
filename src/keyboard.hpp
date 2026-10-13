#pragma once
#include <windows.h>
namespace zzen {
inline INPUT keyInput(WORD vk){
 INPUT i{};i.type=INPUT_KEYBOARD;
 auto scan=MapVirtualKeyW(vk,MAPVK_VK_TO_VSC_EX);
 i.ki.wScan=WORD(scan&0xff);
 i.ki.dwFlags=KEYEVENTF_SCANCODE|((scan&0xff00)?KEYEVENTF_EXTENDEDKEY:0);
 if(vk>=VK_PRIOR&&vk<=VK_DOWN){i.ki.wVk=vk;i.ki.dwFlags=KEYEVENTF_EXTENDEDKEY;}
 return i;
}
}
