//*******************************************************************
#include <stdio.h>
#include "stm32l1xx.h"
#include <math.h>

//-------------------------------------------------------------------
#include "timer.h"
#include "lcd.h"
#include "uart.h"
#include "adc.h"
#include "dac.h"
#include "port.h"
#include "pwm.h"

//******************************************************************
#define PI 3.14156
#define PI2 6.28312
#define DeltaT 0.001

typedef enum {
	DC = 0,
	SIN = 1,
	SAW = 2,
	REC = 3,
}sigType;

typedef struct {
	sigType type;
	int freq;
	int amp;
	float deltaPhi;
}signal;
typedef struct {
	signal sig;
	float phi;
}signalGen;

//*******************************************************************
signalGen sigA = {{0,0,0,0},0};
signalGen sigB = {{0,0,0,0},0};

void idToSig(signalGen* sigGen, char *str){
	signal sig;
	if(sscanf(str,"%d;%d;%d",&(sig.type),&(sig.freq),&(sig.amp)) != 3){
		uartPrintf("Error\r\n");
		return;
	}
	sig.deltaPhi = PI2 * sig.freq * DeltaT;

	TIM2->DIER = TIM_DIER_UDE;
	sigGen->sig = sig;
	TIM2->DIER =TIM_DIER_UIE;



	//signal->counter = 0;
}


void fillStruct(char *str){
	switch(str[0]){
		case 'A': idToSig(&sigA, str+2 ); break;
		case 'B': idToSig(&sigB, str+2 ); break;
	}
}






float dcSig(signalGen* sig){
	return sig->sig.amp;
}

float sinSig(signalGen* sig){
	return sig->sig.amp*sin(sig->phi);
}

float sawSig(signalGen* sig){
	return 0.0;
}

float recSig(signalGen* sig){
	return 0.0;
}

float calculateValue(signalGen* sig){
	sig->phi += sig->sig.deltaPhi;
	if(sig->phi > PI2) sig->phi -= PI2;

	switch(sig->sig.type){
		case DC: return dcSig(sig);
		case SIN: return sinSig(sig);
		case SAW: break;
		case REC: break;
	}
	return 0.0;
}

void timerInterrupt(){
	static const float gain = (float)0xFFF/3000;
	int val = gain*(1500+calculateValue(&sigA)+calculateValue(&sigB));
	if(val > 0xFFF){
		val = 0xFFF;
	}else if(val < 0){
		val = 0;
	}

	dacSet(1, val);
}

int main(void)
{
  uartInit();
  lcdInit();
  dacInit(1);
  timerInit(1e6 * DeltaT, timerInterrupt);
  
  lcdPrintf( 0, 0, 20, __DATE__ "," __TIME__ );
  lcdPrintf( 1, 0, 20, "Hello world1!" );

  while( 1 )
  {
    char *str = uartGetString();

    if( str )
    {
      fillStruct(str);
    }
  }
}





