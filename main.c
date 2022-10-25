//*******************************************************************
#include <stdio.h>
#include "stm32l1xx.h"

//-------------------------------------------------------------------
#include "timer.h"
#include "lcd.h"
#include "uart.h"
#include "adc.h"
#include "dac.h"
#include "port.h"
#include "pwm.h"

//*******************************************************************
unsigned sigA = 1;
unsigned sigB = 2;
unsigned counter = 0;

int main(void)
{
  uartInit();
  lcdInit();
  
  lcdPrintf( 0, 0, 20, __DATE__ "," __TIME__ );
  lcdPrintf( 1, 0, 20, "Hallo Welt!" );

  dacInit(1);

  while( 1 )
  {
	counter++;

    char *str = uartGetString();
      
    if( str )
    {
     //uartPrintf("->%s<-\r\n",str);
    	stringToSig(str);
    }

    //something like this
    //the calculations have to be updated according to the string passed !!!
    //dacSet(1, sigA+sigB);
  }
}


void stringToSig(char *str){
	if(str[0] == 'A'){
		idToSig(sigA, str+2 );
	}
	if(str[0] == 'B'){
		idToSig(sigB, str+2 );
	}
}
void idToSig(int* signal, char *str){
	char param1[128];
	char param2[128];
	char param3[128];

	int i1 =0;
	while(str[0] != ';' && str[0] != '\n' && str[0] != '\0'){
		param1[i1] = str[0];
		str++;
		i1++;
	}
	str++;
	param1[i1] = '\0';

	int i2 = 0;
	while(str[0] != ';' && str[0] != '\n' && str[0] != '\0'){
			param2[i2] = str[0];
			str++;
			i2++;
	}
	str++;
	param2[i2] = '\0';

	int i3 = 0;
	while(str[0] != ';' && str[0] != '\n' && str[0] != '\0'){
			param3[i3] = str[0];
			str++;
			i3++;
	}
	str++;
	param3[i3] = '\0';

	unsigned type = atoi(param1);
	unsigned freq = atoi(param2);
	unsigned amp = atoi(param3);

	switch(type){
	//dc
	case 0:
		signal = dcSig(freq,amp);
		break;
	//sin
	case 1:
		signal = sinSig(freq,amp);
		break;
	//saw
	case 2:
		signal = sawSig(freq,amp);
		break;
	//rec
	case 3:
		signal = recSig(freq,amp);
		break;
	default:
		signal = 0;
	}

	uartPrintf("%u->%d,%d,%d<-\r\n",signal,type,freq,amp);
}





int dcSig(int freq, int amp){
	return amp;
}

int sinSig(int freq, int amp){
	return 20;
}

int sawSig(int freq, int amp){
	return 30;
}

int recSig(int freq, int amp){
	unsigned reduce = counter % (freq*2);
	if(reduce <= freq){
		return amp;
	}else{
		return 0;
	}
}




