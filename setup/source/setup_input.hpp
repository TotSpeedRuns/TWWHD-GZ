#pragma once
#include <cstdint>
#include <vpad/input.h>
#include <padscore/kpad.h>
struct SetupInput { uint32_t trigger=0; bool gamepad=false,pro=false; };
static inline uint32_t proSetupTrigger(uint32_t raw) {
 uint32_t out=0;
 if(raw&WPAD_PRO_BUTTON_A)out|=VPAD_BUTTON_A;
 if(raw&WPAD_PRO_BUTTON_B)out|=VPAD_BUTTON_B;
 if(raw&WPAD_PRO_BUTTON_X)out|=VPAD_BUTTON_X;
 if(raw&WPAD_PRO_BUTTON_UP)out|=VPAD_BUTTON_UP;
 if(raw&WPAD_PRO_BUTTON_DOWN)out|=VPAD_BUTTON_DOWN;
 return out;
}
static inline SetupInput readSetupInput() {
 SetupInput input;VPADStatus pad{};VPADReadError error=VPAD_READ_NO_SAMPLES;
 if(VPADRead(VPAD_CHAN_0,&pad,1,&error)>0&&error==VPAD_READ_SUCCESS){input.trigger|=pad.trigger;input.gamepad=true;}
 // GamePad-only startup diagnostic; Pro reads intentionally deferred.
 return input;
}
