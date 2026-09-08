#include <winapi/dinput/idirectinputeffect.h>
#include <SDL3/SDL.h>
#include <cstdio>
#include <stdexcept>
#include <initializer_list>
#include <cstring>
using namespace win32;
using namespace win32::dinput;
static Uint16 amplitude;
static bool SDLCALL rumble(void*,Uint16 low,Uint16){amplitude=low;return true;}
static void check(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct App:WinApplication { App():WinApplication("ff-test",0x400000,{{nullptr,0x400000,4096}}){} };
int main(){
    try{
        check(SDL_Init(SDL_INIT_JOYSTICK),"SDL init");
        SDL_VirtualJoystickDesc v{};SDL_INIT_INTERFACE(&v);v.type=SDL_JOYSTICK_TYPE_GAMEPAD;v.naxes=2;v.nbuttons=2;v.name="Force feedback test";v.Rumble=rumble;
        auto id=SDL_AttachVirtualJoystick(&v);check(id!=0,"virtual joystick");
        {
            App app;com::InitialiseComMemory(&app);Gamepad pad(0);check(pad.hasForceFeedback(),"rumble capability");
            check(!Gamepad::isPolledByGame(),"opening FF device is not an input read");
            Input::poll();check(!Gamepad::isPolledByGame(),"generic polling must not suppress keys");
            { Gamepad outputOnly(999);outputOnly.markInputRead();check(!Gamepad::isPolledByGame(),"output-only device must not suppress keys"); }
            pad.markInputRead();check(Gamepad::isPolledByGame(),"real device state read suppresses duplicate input");
            MemMap memory(8192);auto base=memory.getBlockStart();x86::CPU cpu{};
            auto call=[&](DWORD object,unsigned slot,std::initializer_list<DWORD> args){
                cpu.esp=base+7000;auto sp=cpu.esp;
                app.getMemory<DWORD>(sp)=0;app.getMemory<DWORD>(sp+4)=object;
                unsigned i=2;for(auto a:args)app.getMemory<DWORD>(sp+4*i++)=a;
                DWORD table=app.getMemory<DWORD>(object);DWORD method=app.getMemory<DWORD>(table+4*slot);
                app.dynamic_call(method,cpu);check(cpu.esp==sp+i*4,"stdcall stack balance");return cpu.eax;
            };
            auto* p=&app.getMemory<IDirectInputDevice::DIEFFECT>(base);std::memset(p,0,sizeof(*p));
            p->dwSize=52;p->dwFlags=0x22;p->dwDuration=100000;p->dwGain=10000;p->dwTriggerButton=0xffffffff;
            p->cAxes=1;p->rgdwAxes=Packed<DWORD>(base+128);p->rglDirection=Packed<LONG>(base+132);
            p->cbTypeSpecificParams=4;p->lpvTypeSpecificParams=Packed<void>(base+144);app.getMemory<LONG>(base+144)=5000;
            GUID guid={0x13541c20,0x8e33,0x11d0,{0x9a,0xd0,0,0xa0,0xc9,0xa0,0x6e,0x35}};
            Packed<com::IUnknown> object;
            check(IDirectInputEffect::create(&app,cpu,&pad,&guid,p,&object)==0,"DX5 descriptor creation");
            check(amplitude==0,"creation must not play");
            check(call(object,7,{1,0})==0,"start");check(amplitude>32000&&amplitude<33500,"game magnitude translated");
            SDL_Delay(130);IDirectInputEffect::update();check(amplitude==0,"finite duration expires");
            p->dwDuration=0xffffffff;check(call(object,6,{base,1|0x20000000})==0,"change duration and start");
            check(amplitude>0,"infinite starts");
            IDirectInputEffect::gain(&pad,5000);check(amplitude>16000&&amplitude<17000,"device gain");
            check(call(object,8,{})==0&&amplitude==0,"stop");
            check(call(object,7,{1,0})==0,"restart");
            IDirectInputEffect::command(&pad,4);check(amplitude==0,"pause silences");
            IDirectInputEffect::command(&pad,8);check(amplitude>0,"continue resumes");
            IDirectInputEffect::command(&pad,32);check(amplitude==0,"actuators off");
            IDirectInputEffect::command(&pad,16);check(amplitude>0,"actuators on");
            check(call(object,11,{})==0&&amplitude==0,"unload stops");
            p->dwGain=10001;check(call(object,6,{base,4})!=0,"reject invalid gain");p->dwGain=10000;
            // Deep-copy type-specific data; callers can overwrite their guest buffers.
            app.getMemory<LONG>(base+144)=2000;check(call(object,6,{base,0x100})==0,"update magnitude");app.getMemory<LONG>(base+144)=9999;
            check(call(object,7,{1,0})==0,"start copied data");check(amplitude>6000&&amplitude<7000,"no retained guest pointer");
            check(call(object,2,{})==0&&amplitude==0,"release stops and frees");
            guid.Data1=0x13541c28;check(IDirectInputEffect::create(&app,cpu,&pad,&guid,p,&object)!=0&&DWORD(object)==0,"unsupported effect rejected");
            // All three periodic effects used by this implementation respect zero gain.
            for(DWORD kind:{2u,3u,7u}){
                guid.Data1=0x13541c20+kind;p->dwGain=0;p->cbTypeSpecificParams=kind==7?24:16;
                std::memset(&app.getMemory<LONG>(base+144),0,48);app.getMemory<LONG>(base+144)=10000;
                check(IDirectInputEffect::create(&app,cpu,&pad,&guid,p,&object)==0,"periodic/spring creation");
                check(call(object,7,{1,0})==0&&amplitude==0,"zero gain silent");call(object,2,{});
            }
        }
        SDL_DetachVirtualJoystick(id);SDL_Quit();
        std::puts("PASS: guest COM ABI, DX5 size, create/start/update/stop, duration, gain, pause, actuators, unload/release, deep copy, unsupported types, zero-gain effects");return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"FAIL: %s\n",e.what());return 1;}
}
