#include <winapi/dinput/idirectinputeffect.h>
#include <winapi/wrapper.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <mutex>
#include <vector>
#include <map>

namespace win32 { namespace dinput {
namespace {
constexpr HRESULT invalid = 0x80070057, unsupported = 0x80004001;
const GUID iid = {0xe7e1f7c0,0x88d2,0x11d0,{0x9a,0xd0,0,0xa0,0xc9,0xa0,0x6e,0x35}};
GUID effectID(unsigned kind) { return {0x13541c20u+kind,0x8e33,0x11d0,{0x9a,0xd0,0,0xa0,0xc9,0xa0,0x6e,0x35}}; }
// Only types actually implemented by the rumble translator are advertised.
const unsigned kinds[] = {0,1,2,3,4,5,6,7};
const char* names[] = {"Constant","Ramp","Square","Sine","Triangle","Saw up","Saw down","Spring"};
std::recursive_mutex mutex;
struct Device { DWORD gain=10000; bool enabled=true,paused=false; Uint64 pausedAt=0; float lastLevel=-1; Uint64 lastOutput=0; Gamepad::RumbleDetail lastDetail; };
std::map<Gamepad*,Device> devices;
struct Effect : GenericResource {
    Gamepad* pad; GUID guid; unsigned kind;
    IDirectInputDevice::DIEFFECT params;
    IDirectInputDevice::DIENVELOPE envelope{};
    DWORD axes[2]{}; LONG direction[2]{}; LONG data[12]{};
    bool playing=false,downloaded=false,hasEnvelope=false;
    // Started by the game and not stopped since, see setParameters().
    bool armed=false;
    Uint64 began=0; DWORD iterations=1;
    // Edge state for an effect the device fires itself, see pollTriggers().
    bool triggerDown=false; Uint64 triggerFired=0;
    ~Effect();
    Effect(Gamepad* p,const GUID& g):pad(p),guid(g),kind(g.Data1-0x13541c20u) { std::memset(&params,0,sizeof(params));params.dwSize=56;params.dwGain=10000;params.dwTriggerButton=0xffffffffu; }
};
std::vector<Effect*> effects;
Effect::~Effect(){
    std::lock_guard<std::recursive_mutex> lock(mutex);
    effects.erase(std::remove(effects.begin(),effects.end(),this),effects.end());
}
DWORD dataSize(unsigned kind,DWORD axes) { return kind==0?4:kind==1?8:kind==7?24*axes:16; }
bool running(Effect& e,Uint64 now) {
    if(!e.playing||!e.pad)return false;
    const auto& device=devices[e.pad];
    if(device.paused)now=device.pausedAt;
    Uint64 elapsed=now-e.began;
    if(elapsed<e.params.dwStartDelay)return true;
    if(e.params.dwDuration!=0xffffffffu && e.iterations!=0xffffffffu &&
       elapsed-e.params.dwStartDelay>=Uint64(e.params.dwDuration)*e.iterations)e.playing=false;
    return e.playing;
}
/* A periodic effect's share of a rumble motor.  The game keeps two going for
 * the whole race -- a road rumble that follows speed and surface, an engine hum
 * that follows the revs -- and at full weight they drowned everything else out:
 * the motors hummed from the start line on and a collision barely rose above
 * them.  As texture they stay underneath, strongest off the road and at the rev
 * limiter, and the top of the range is left to the jolts. */
constexpr float texture=0.3f;
/* A jolt -- a finite constant force, which the game plays for collisions,
 * landings and gear changes -- is felt as a hit that dies away.  Held at full
 * strength for its whole duration, up to a second after a crash, it came out
 * as a drone.  The game gives harder hits longer durations, so they also fade
 * for longer. */
float impact(const Effect& e,Uint64 t) {
    const float fade=std::clamp(e.params.dwDuration/2.f,80000.f,400000.f);
    return std::max(0.f,1.f-float(t)/fade);
}
float strength(Effect& e,Uint64 now) {
    /* A condition effect such as the spring is a force against the player's
     * hand, pulling a wheel back toward the centre, and a rumble motor has
     * nothing to pull with.  Played as vibration that grew with the distance
     * from that centre, it buzzed on every turn and every pedal press, through
     * the spring the game keeps running all race. */
    if(e.kind==7||!running(e,now)||now-e.began<e.params.dwStartDelay)return 0;
    Uint64 t=now-e.began-e.params.dwStartDelay;
    const bool finite=e.params.dwDuration&&e.params.dwDuration!=0xffffffffu;
    if(finite)t%=e.params.dwDuration;
    float value=0;
    if(e.kind==0)value=float(e.data[0]);
    else if(e.kind==1) { float f=e.params.dwDuration?float(t)/e.params.dwDuration:0;value=e.data[0]+(float(e.data[1])-e.data[0])*f; }
    else {
        /* Periodic effects report how hard they shake, not where their waveform
         * happens to be right now.  The mixer runs once per frame -- about 33 ms
         * at the frame cap -- while a periodic effect's period is tens of
         * milliseconds, so sampling the wave lands on an arbitrary point of it,
         * near zero as often as near the peak: a road rumble came out as jitter
         * averaging well under its real strength, which is why a steady engine
         * was the only thing that could be felt.  A rumble motor has its own
         * resonance and cannot reproduce the shape anyway; the peak excursion is
         * what it can reproduce, and the envelope below still shapes it. */
        value=(std::abs(float(e.data[1]))+std::abs(float(e.data[0])))*texture;
    }
    float amplitude=std::min(10000.f,std::abs(value));
    if(e.hasEnvelope) {
        float nominal=e.kind==0?std::abs(float(e.data[0])):e.kind==1?std::max(std::abs(float(e.data[0])),std::abs(float(e.data[1]))):std::abs(float(e.data[0]));
        float level=nominal;
        if(e.envelope.dwAttackTime&&t<e.envelope.dwAttackTime)
            level=e.envelope.dwAttackLevel+(nominal-e.envelope.dwAttackLevel)*float(t)/e.envelope.dwAttackTime;
        if(e.params.dwDuration!=0xffffffffu&&e.envelope.dwFadeTime&&e.params.dwDuration-t<e.envelope.dwFadeTime)
            level=e.envelope.dwFadeLevel+(nominal-e.envelope.dwFadeLevel)*float(e.params.dwDuration-t)/e.envelope.dwFadeTime;
        if(nominal>0)amplitude*=level/nominal;
    }
    if(e.kind==0&&finite)amplitude*=impact(e,t);
    return std::clamp(amplitude/10000.f*e.params.dwGain/10000.f,0.f,1.f);
}
#ifdef __ANDROID__
/* DirectInput sums concurrent effects and lets the hardware clip.  Force
 * feedback hardware has several motors and a wide range to clip into; a phone
 * has one LRA whose perceived strength flattens near the top, so a hard clamp
 * made a light road rumble and a head-on collision feel identical -- both summed
 * past 1, both played at maximum.  Below the knee the sum passes through
 * untouched; above it the remaining headroom is approached but never reached, so
 * louder always stays louder and the loudest events still land on the same
 * ceiling the strength slider is calibrated against. */
constexpr float knee=0.5f;
float softLimit(float sum) {
    if(!(sum>knee))return std::max(0.f,sum);
    return knee+(1.f-knee)*(1.f-std::exp(-(sum-knee)/(1.f-knee)));
}
#endif
/* DirectInput lets an effect name a button that fires it, and the device is
 * expected to play it without the game asking again -- so a game that sets one
 * never calls Start() for that effect.  rgbButtons sits at offset 48 in
 * DIJOYSTATE, which is what DIJOFS_BUTTON(n) encodes. */
void pollTriggers(Uint64 now) {
    for(auto* e:effects) {
        const DWORD offset=e->params.dwTriggerButton;
        if(!e->pad||offset==0xffffffffu||offset<48||offset>=48+32){e->triggerDown=false;continue;}
        const bool down=((e->pad->getState().buttons>>(offset-48))&1)!=0;
        const DWORD repeat=e->params.dwTriggerRepeatInterval;
        const bool repeats=repeat!=0&&repeat!=0xffffffffu;
        // Fire on the press, and again while held only if a repeat was asked for.
        if(down&&e->params.cbTypeSpecificParams&&
           (!e->triggerDown||(repeats&&now-e->triggerFired>=repeat))) {
            e->downloaded=true;e->playing=true;e->iterations=1;e->began=now;e->triggerFired=now;
        }
        e->triggerDown=down;
    }
}
// DIPERIODIC.dwPeriod, in microseconds, as a rate.
float hertz(const Effect& e) { return e.data[3]>0?1e6f/float(e.data[3]):0; }
float quantize(float value,float step) { return std::round(value/step)*step; }
/* Each device now drives its own motors -- slot 0 the phone and the first pad,
 * slot 1 the second pad -- so each is mixed and limited on its own.  This used
 * to sum every device into one number and send it to whichever device happened
 * to sort first, which was harmless while there was only ever one, and would
 * have played player two's collisions through player one's phone the moment
 * split screen worked.
 * Beside that level the parts it is made of go out too (Gamepad::RumbleDetail):
 * the jolts, and the game's two textures at full weight with the rate each runs
 * at -- its sine is the engine, its square the road.  A pad's motor cannot
 * vibrate gently, so GameHaptics plays those textures as taps. */
void mix() {
    Uint64 now=SDL_GetTicksNS()/1000;
    for(auto& pair:devices) {
        float level=0,roadPeak=0,enginePeak=0;
        Gamepad::RumbleDetail detail;
        // A silenced or paused device cannot contribute, so do not evaluate its
        // waveforms only to throw the result away.
        if(pair.second.enabled&&!pair.second.paused)
            for(auto* e:effects) {
                if(e->pad!=pair.first)continue;
                const float s=strength(*e,now);
                if(!(s>0))continue;
                level+=s;
                if(e->kind<=1)detail.impact+=s;
                // Any other waveform -- the game plays none -- counts as road.
                else if(e->kind==3){detail.engine+=s/texture;if(s>enginePeak){enginePeak=s;detail.engineHz=hertz(*e);}}
                else {detail.road+=s/texture;if(s>roadPeak){roadPeak=s;detail.roadHz=hertz(*e);}}
            }
        const float gain=pair.second.gain/10000.f;
        level*=gain;
        detail.impact*=gain;detail.road*=gain;detail.engine*=gain;
#ifdef __ANDROID__
        level=softLimit(level);
        detail.impact=softLimit(detail.impact);
#else
        level=std::clamp(level,0.f,1.f);
#endif
        // Quantized, so revs creeping up do not cross JNI on every frame.
        detail.impact=quantize(std::min(1.f,detail.impact),1/255.f);
        detail.road=quantize(std::min(1.f,detail.road),1/255.f);
        detail.engine=quantize(std::min(1.f,detail.engine),1/255.f);
        detail.roadHz=quantize(detail.roadHz,.25f);
        detail.engineHz=quantize(detail.engineHz,.25f);
        auto& d=pair.second;
        if(level!=d.lastLevel||detail!=d.lastDetail||(level>0&&now-d.lastOutput>=40000)){
            pair.first->rumble(level,detail);d.lastLevel=level;d.lastDetail=detail;d.lastOutput=now;
        }
    }
}
}
x86::reg32 IDirectInputEffect::table=0;
IDirectInputEffect::IDirectInputEffect(GenericResource* resource):IUnknown(iid){m_vtable=table;m_resource=resource;}
bool IDirectInputEffect::supported(const GUID& guid){for(unsigned k:kinds)if(guid==effectID(k))return true;return false;}
HRESULT IDirectInputEffect::create(WinApplication* app,x86::CPU& cpu,Gamepad* pad,const GUID* guid,const IDirectInputDevice::DIEFFECT* params,Packed<IUnknown>* out){
    if(!out)return invalid;*out=Packed<IUnknown>();
    if(!pad||!guid||!supported(*guid)||!pad->hasForceFeedback())return unsupported;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto* state=new Effect(pad,*guid);app->allocateResource(state);devices.try_emplace(pad);
    auto address=com::ComAlloc(app);auto* object=new (&app->getMemory<IDirectInputEffect>(address)) IDirectInputEffect(state);
    effects.push_back(state);
    if(params){HRESULT result=object->setParameters(app,cpu,params,0x3ff);if(result){object->release(app,cpu);return result;}}
    *out=Packed<IUnknown>(address);
    // Effects are created and started constantly during a race; this belongs
    // with the rest of the API trace, not in everybody's release log.
    if(WinApplication::traceApi())
        SDL_Log("ForceFeedback CreateEffect kind=%u duration=%u gain=%u",state->kind,state->params.dwDuration,state->params.dwGain);
    return 0;
}
ULONG IDirectInputEffect::release(WinApplication* app,x86::CPU& cpu){
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if(m_refCount==1){auto* e=static_cast<Effect*>(m_resource);app->freeResource(e->getResourceIndex());m_resource=nullptr;mix();}
    return IUnknown::Release(app,cpu);
}
HRESULT IDirectInputEffect::initialize(WinApplication*,x86::CPU&,DWORD,DWORD,const GUID*){return 0;}
HRESULT IDirectInputEffect::effectGuid(WinApplication*,x86::CPU&,GUID* out){if(!out)return invalid;*out=static_cast<Effect*>(m_resource)->guid;return 0;}
HRESULT IDirectInputEffect::setParameters(WinApplication* app,x86::CPU& cpu,const IDirectInputDevice::DIEFFECT* p,DWORD flags){
    if(!p||(p->dwSize!=52&&p->dwSize!=56))return invalid;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    auto& e=*static_cast<Effect*>(m_resource);
    // Validate before mutation; DIEFFECT_DX5 is 52 bytes and has no start delay.
    DWORD axes=(flags&0x20)?p->cAxes:e.params.cAxes;
    if(axes>2 || ((flags&0x20)&&axes&&!p->rgdwAxes) || ((flags&0x40)&&axes&&!p->rglDirection))return invalid;
    if((flags&4)&&p->dwGain>10000)return invalid;
    DWORD bytes=dataSize(e.kind,axes);
    if((flags&0x100)&&(!p->lpvTypeSpecificParams||p->cbTypeSpecificParams!=bytes))return invalid;
    if(flags&1)e.params.dwDuration=p->dwDuration;
    if(flags&2)e.params.dwSamplePeriod=p->dwSamplePeriod;
    if(flags&4)e.params.dwGain=p->dwGain;
    if(flags&8)e.params.dwTriggerButton=p->dwTriggerButton;
    if(flags&0x10)e.params.dwTriggerRepeatInterval=p->dwTriggerRepeatInterval;
    if(flags&0x20){e.params.cAxes=axes;std::memcpy(e.axes,&app->getMemory<DWORD>(p->rgdwAxes),axes*4);}
    if(flags&0x40){e.params.dwFlags=p->dwFlags;std::memcpy(e.direction,&app->getMemory<LONG>(p->rglDirection),axes*4);}
    if(flags&0x80){e.hasEnvelope=p->lpEnvelope!=0;if(e.hasEnvelope)e.envelope=app->getMemory<IDirectInputDevice::DIENVELOPE>(p->lpEnvelope);}
    if(flags&0x100){std::memcpy(e.data,&app->getMemory<LONG>(p->lpvTypeSpecificParams),bytes);e.params.cbTypeSpecificParams=bytes;}
    if((flags&0x200)&&p->dwSize>=56)e.params.dwStartDelay=p->dwStartDelay;
    if(!(flags&0x80000000))e.downloaded=true;
    /* The game starts its jolt effect once and from then on only rewrites it:
     * its own bookkeeping marks an effect playing on Start and clears the mark
     * only on Stop, so every later collision, landing and gear change arrives as
     * new parameters on an effect whose duration ran out long ago.  Following
     * DirectInput to the letter, this played the first jolt and then nothing
     * until the game next stopped its effects.  The update is the only sign of
     * the new jolt, so a finite constant force that was started and not stopped
     * plays again from the top. */
    if(e.kind==0&&e.armed&&e.params.dwDuration&&e.params.dwDuration!=0xffffffffu&&(flags&(1|0x40|0x100)))
        {e.playing=true;e.iterations=1;e.began=SDL_GetTicksNS()/1000;}
    if(flags&0x20000000)return start(app,cpu,1,0);
    mix();return 0;
}
HRESULT IDirectInputEffect::getParameters(WinApplication* app,x86::CPU&,IDirectInputDevice::DIEFFECT* p,DWORD flags){
    if(!p||(p->dwSize!=52&&p->dwSize!=56))return invalid;
    std::lock_guard<std::recursive_mutex> lock(mutex);auto& e=*static_cast<Effect*>(m_resource);
    if(((flags&0x60)&&p->cAxes<e.params.cAxes)||((flags&0x100)&&p->cbTypeSpecificParams<e.params.cbTypeSpecificParams)){
        p->cAxes=e.params.cAxes;p->cbTypeSpecificParams=e.params.cbTypeSpecificParams;return 0x800700ea;
    }
    if(flags&1)p->dwDuration=e.params.dwDuration;
    if(flags&2)p->dwSamplePeriod=e.params.dwSamplePeriod;
    if(flags&4)p->dwGain=e.params.dwGain;
    if(flags&8)p->dwTriggerButton=e.params.dwTriggerButton;
    if(flags&0x10)p->dwTriggerRepeatInterval=e.params.dwTriggerRepeatInterval;
    if(flags&0x20){if(!p->rgdwAxes)return invalid;std::memcpy(&app->getMemory<DWORD>(p->rgdwAxes),e.axes,e.params.cAxes*4);p->cAxes=e.params.cAxes;}
    if(flags&0x40){if(!p->rglDirection)return invalid;std::memcpy(&app->getMemory<LONG>(p->rglDirection),e.direction,e.params.cAxes*4);p->dwFlags=e.params.dwFlags;}
    if((flags&0x80)&&e.hasEnvelope){if(!p->lpEnvelope)return invalid;app->getMemory<IDirectInputDevice::DIENVELOPE>(p->lpEnvelope)=e.envelope;}
    if(flags&0x100){if(!p->lpvTypeSpecificParams)return invalid;std::memcpy(&app->getMemory<LONG>(p->lpvTypeSpecificParams),e.data,e.params.cbTypeSpecificParams);p->cbTypeSpecificParams=e.params.cbTypeSpecificParams;}
    if((flags&0x200)&&p->dwSize>=56)p->dwStartDelay=e.params.dwStartDelay;
    return 0;
}
HRESULT IDirectInputEffect::start(WinApplication*,x86::CPU&,DWORD iterations,DWORD flags){
    std::lock_guard<std::recursive_mutex> lock(mutex);auto& e=*static_cast<Effect*>(m_resource);
    if(!e.pad||!iterations)return invalid;
    /* An effect that names a trigger button is still startable by hand -- the
     * button only adds a second way to fire it.  This used to refuse them
     * outright, which made every such effect silent forever. */
    if(flags&1)for(auto* other:effects)if(other->pad==e.pad)other->playing=other->armed=false;
    if(!e.params.cbTypeSpecificParams)return invalid;
    e.downloaded=true;e.playing=e.armed=true;e.iterations=iterations;e.began=SDL_GetTicksNS()/1000;
    if(WinApplication::traceApi())
        SDL_Log("ForceFeedback Start kind=%u iterations=%u",e.kind,iterations);
    mix();return 0;
}
HRESULT IDirectInputEffect::stop(WinApplication*,x86::CPU&){std::lock_guard<std::recursive_mutex> lock(mutex);auto* e=static_cast<Effect*>(m_resource);e->playing=e->armed=false;mix();return 0;}
HRESULT IDirectInputEffect::status(WinApplication*,x86::CPU&,DWORD* out){if(!out)return invalid;std::lock_guard<std::recursive_mutex> lock(mutex);*out=running(*static_cast<Effect*>(m_resource),SDL_GetTicksNS()/1000)?1:0;return 0;}
HRESULT IDirectInputEffect::download(WinApplication*,x86::CPU&){std::lock_guard<std::recursive_mutex> lock(mutex);static_cast<Effect*>(m_resource)->downloaded=true;return 0;}
HRESULT IDirectInputEffect::unload(WinApplication* app,x86::CPU& cpu){stop(app,cpu);std::lock_guard<std::recursive_mutex> lock(mutex);static_cast<Effect*>(m_resource)->downloaded=false;return 0;}
HRESULT IDirectInputEffect::escape(WinApplication*,x86::CPU&,void*){return unsupported;}
void IDirectInputEffect::update(){
    std::lock_guard<std::recursive_mutex> lock(mutex);
    static Uint64 last=0;auto now=SDL_GetTicks();if(now-last<16)return;last=now;
    pollTriggers(SDL_GetTicksNS()/1000);
    mix();
}
void IDirectInputEffect::command(Gamepad* pad,DWORD flags){
    if(!pad)return;std::lock_guard<std::recursive_mutex> lock(mutex);auto& d=devices[pad];auto now=SDL_GetTicksNS()/1000;
    if(flags==1||flags==2)for(auto* e:effects)if(e->pad==pad){e->playing=e->armed=false;if(flags==1)e->downloaded=false;}
    if(flags==1)d=Device{};
    if(flags==4&&!d.paused){d.paused=true;d.pausedAt=now;}
    if(flags==8&&d.paused){for(auto* e:effects)if(e->pad==pad&&e->playing)e->began+=now-d.pausedAt;d.paused=false;}
    if(flags==16)d.enabled=true;if(flags==32)d.enabled=false;mix();
}
DWORD IDirectInputEffect::state(Gamepad* pad){std::lock_guard<std::recursive_mutex> lock(mutex);auto& d=devices[pad];bool any=false,playing=false;for(auto* e:effects)if(e->pad==pad){any=true;playing|=running(*e,SDL_GetTicksNS()/1000);}return (!any?1u:0u)|(!playing?2u:0u)|(d.paused?4u:0u)|(d.enabled?0x10u:0x20u)|0x40u;}
void IDirectInputEffect::gain(Gamepad* pad,DWORD value){if(!pad)return;std::lock_guard<std::recursive_mutex> lock(mutex);devices[pad].gain=std::min(value,10000u);mix();}
void IDirectInputEffect::detach(Gamepad* pad){std::lock_guard<std::recursive_mutex> lock(mutex);pad->rumble(0);for(auto* e:effects)if(e->pad==pad){e->pad=nullptr;e->playing=false;}devices.erase(pad);}
HRESULT IDirectInputEffect::info(IDirectInputDevice::DIEffectInfo* out,const GUID* guid){
    if(!out||!guid||out->dwSize!=sizeof(*out))return invalid;if(!supported(*guid))return unsupported;
    unsigned k=guid->Data1-0x13541c20u;*out={};out->dwSize=sizeof(*out);out->guid=*guid;out->dwEffType=k==0?1:k==1?2:k==7?4:3;
    out->dwStaticParams=out->dwDynamicParams=0x3e7;std::strcpy(out->tszName,names[k]);return 0;
}
HRESULT IDirectInputEffect::enumerate(WinApplication* app,x86::CPU& cpu,x86::reg32 callback,x86::reg32 user,DWORD filter){
    if(!callback)return invalid;MemMap memory(512);auto* out=&app->getMemory<IDirectInputDevice::DIEffectInfo>(memory.getBlockStart());
    for(unsigned k:kinds){auto guid=effectID(k);out->dwSize=sizeof(*out);info(out,&guid);if((filter&0xff)&&((filter&0xff)!=out->dwEffType))continue;
        cpu.esp-=12;app->getMemory<DWORD>(cpu.esp)=callback;app->getMemory<DWORD>(cpu.esp+4)=memory.getBlockStart();app->getMemory<DWORD>(cpu.esp+8)=user;app->dynamic_call(callback,cpu);if(cpu.terminate||!cpu.eax)break;}
    return 0;
}
x86::reg32 IDirectInputEffect::init(WinApplication* app,x86::reg32 address,x86::reg8* memory){
    table=address;auto* v=reinterpret_cast<x86::reg32*>(memory);v[0]=0x400000;v[1]=0x400001;
#define ENTRY(slot,method) v[slot]=0x400100+slot;app->registerMethod(v[slot],{"IDirectInputEffect::" #method,&Wrapper<decltype(&IDirectInputEffect::method),&IDirectInputEffect::method>::stdcall});
    ENTRY(2,release) ENTRY(3,initialize) ENTRY(4,effectGuid) ENTRY(5,getParameters) ENTRY(6,setParameters)
    ENTRY(7,start) ENTRY(8,stop) ENTRY(9,status) ENTRY(10,download) ENTRY(11,unload) ENTRY(12,escape)
#undef ENTRY
    return 13*4;
}
}}
