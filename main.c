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
#define EULER 0.577215664901

//Signal
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


//Filter
typedef enum {
	OFF = 0,
	TP = 1,
	HP = 2,
	BS = 3
}filterType;

typedef struct {
	float a0;
	float a1;
	float a2;
	float b1;
	float b2;
}filterKoef2Pol;

typedef struct {
	filterType type;
	float freq;
	float bwidth;
	filterKoef2Pol koef;
	float x1;
	float x2;
	float y1;
	float y2;
}filter;

//*******************************************************************
signalGen sigA = {{0,0,0,0},0};
signalGen sigB = {{0,0,0,0},0};
filter fil = {0,0.0,0.0,{0.0,0.0,0.0,0.0,0.0},0.0,0.0,0.0,0.0};

void idToSig(signalGen* sigGen, char *str){
	signal sig;
	if(sscanf(str,"%d;%d;%d",&(sig.type),&(sig.freq),&(sig.amp)) != 3){
		uartPrintf("Error\r\n");
		return;
	}
	sig.deltaPhi = PI2 * sig.freq * DeltaT;

	lockTimer();
	sigGen->sig = sig;
	unlockTimer();

	//signal->counter = 0;
}

float calculateFilterValue(filter f,float x0){
	float y = f.koef.a0 * x0 + f.koef.a1 * f.x1 + f.koef.a2 * f.x2 + f.koef.b1 * f.y1 + f.koef.b2 * f.y2;
	f.x2 = f.x1;
	f.x1 = x0;
	f.y2 = f.y1;
	f.y1 = y;
	return y;
}

void idToFil(filter* f, char *str){
	filterKoef2Pol fk2 = {0.0,0.0,0.0,0.0,0.0};
	if(sscanf(str,"%d;%f;%f",&(f->type),&(f->freq),&(f->bwidth)) != 3){
		uartPrintf("Error\r\n");
		return;
	}
	float f0 = 0.0;
	float r = 0.0;
	float k = 0.0;
	switch(f->type){
		case OFF: break;
		case TP:
			f0 = sqrt(PI2*f->freq*DeltaT);
			r = pow(EULER,-1*f0);
			k = (1-(2*r) * cos(f0) + pow(r,2)) / 4;
			fk2.a0 = k;
			fk2.a1 = 2*k;
			fk2.a2 = k;
			fk2.b1 = 2*r*cos(f0);
			fk2.b2 = -1*pow(r,2);

			lockTimer();
			f->koef = fk2;
			unlockTimer();

			return;
		case HP: break;
		case BS: break;
	}
}


void fillStruct(char *str){
	switch(str[0]){
		case 'A': idToSig(&sigA, str+2 ); break;
		case 'B': idToSig(&sigB, str+2 ); break;
		case 'F': idToFil(&fil, str+2); break;
	}
}






float dcSig(signalGen* sig){
	return sig->sig.amp;
}

float sinSig(signalGen* sig){
	return sig->sig.amp*sin(sig->phi);
}

float sawSig(signalGen* sig){
	return (sig->sig.amp*(sig->phi/PI2));
}

float recSig(signalGen* sig){
	if(sig->phi <= PI){
		return sig->sig.amp;
	}
	return 0.0;
}

float calculateValue(signalGen* sig){
	sig->phi += sig->sig.deltaPhi;
	if(sig->phi > PI2) sig->phi -= PI2;

	switch(sig->sig.type){
		case DC: return dcSig(sig);
		case SIN: return sinSig(sig);
		case SAW: return sawSig(sig);
		case REC: return recSig(sig);
	}
	return 0.0;
}

void timerInterrupt(){
	static const float gain = (float)0xFFF/3000;
	int valDac1 = gain*(1500+calculateValue(&sigA)+calculateValue(&sigB));
	if(valDac1 > 0xFFF){
		valDac1 = 0xFFF;
	}else if(valDac1 < 0){
		valDac1 = 0;
	}
	dacSet(1, valDac1);

	//unsigned valAdc1 = adcGet(1);


	//unsigned valDac2 = valAdc1;
	//uartPrintf("%d",valAdc1);
	//dacSet(2, valDac2);
}

int main(void)
{
  uartInit();
  lcdInit();
  dacInit(1);
  dacInit(2);
  adcInit(1);
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





