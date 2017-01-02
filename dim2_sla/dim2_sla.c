/*
 *    _____ ________    __________  __  _______
 *   / ___// ____/ /   / ____/ __ )/ / / / ___/
 *   \__ \/ __/ / /   / /_  / __  / / / /\__ \
 *  ___/ / /___/ /___/ __/ / /_/ / /_/ /___/ /
 * /____/_____/_____/_/   /_____/\____//____/
 *
 *  Copyright (c) 2009 Richard Weissteiner richard@seelaus.at
 *  Copyright (c) 2017 Stefan Haller
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 */
/**
* @file   dim2_sla.c
* @author
* @date   Sun Jan 12 21:30:01 2009
*
* @brief  Dimmer Slave  2 channel dimmer mit I2C.
*/

#include "dim2_sla.h"


unsigned char dimm_I2C[2];
unsigned char mk[2];
unsigned char bytenummer;
unsigned int zl_50hz=0;
unsigned int sp=0;
unsigned int m=0;
unsigned char dimmzl=0;     //beim nulldurchgang auf 0 setzen
unsigned char mode=AB;      //modus des dimmer Anschnitt Abschnitt oder PWM
//__bit strom0_fl=0;
unsigned char dimmcompare,cmd,phase;
unsigned char zero;
static __code unsigned char __at (0x1CBC) zeropoint={47};

unsigned char __at (0x00) RAM[0];
__bit locked=0;




void i2c_int(void) __interrupt (6)
{
  if(I2STAT==0x60)      //Slaveadresse
    bytenummer=0;
  if(I2STAT==0x80)      //Datenbyte emfangen
    bytenummer++;
  if(I2STAT==0xA0)      //Stop emfangen
    bytenummer=0;
  if(bytenummer>1&&bytenummer<4)  //erstes byte verwerfen
  if (mode==AB)dimm_I2C[bytenummer-2]= I2DAT; //zweites und drittes byte sind K1 + K2
  else dimm_I2C[bytenummer-2]= 0 - I2DAT;
  SI=0;
}


void nulldurchgang(void)
  {

  if(mode==AB)
    {
    dimmzl=0;
    if(dimm_I2C[0])  //dimmwert gr�sser 0 Dimmer  ein bei abschnitt
      K1OUT=0;       //EIN
    if(dimm_I2C[1])  //dimmwert gr�sser 0 Dimmer  ein
      K2OUT=0;       //EIN
    }
  if(mode==AN)
    {
      dimmzl=0xff;
      K1OUT=1;     //AUS bei anschnitt
      K2OUT=1;
    }
    return;
  }


void ex1_int(void) __interrupt (2)// ERKENNUNG NULLDURCHGANG
{
	unsigned char dc_mess;
	//if(dimmcompare>5 && dimmcompare<25)dimmcompare=17;
//SBUF=dimmcompare;
	dc_mess=dimmcompare-61;// 20
	phase=dc_mess;//	SBUF=dimmcompare;
	if(dc_mess ){
		if (dc_mess>200 && dc_mess<253){
			TH0++;
//			dimmcompare=0;
		}
		else if(dc_mess>=3 && dc_mess<=50){
//			dimmcompare=0;
			TH0--;
		}
	}//ende if(dc_mess...

		//if (dc_mess>=240&& dc_mess<=15)dimmcompare=0;
		//if (dc_mess>=253)dimmcompare=((255-dc_mess)>>1);
		//if (dc_mess<=3)dimmcompare=(dc_mess>>1);



dimmcompare=zero;//61;//20
/*


	//sp=zl_50hz;
//  if(mode==AN&&zl_50hz>450)
//zl_50hz=48;
//P0_1 =! P0_1;
//return;
*/
}

void tim0_int(void) __interrupt (1)
{

	dimmcompare++;
//TF0=0;
/*
	TL0=0x95;	//107 bis �berlauf 29�s
	TH0=0xff;
  if(zl_50hz<5000)
    zl_50hz++;
  if(mode==AN)
    {
      if(P0_2==1 && strom0_fl==0)//flankenauswertung Strommessen
        {
          strom0_fl=1;
          zl_50hz=85;
        }
      if(P0_2==0)
        strom0_fl=0;
    }
  if(zl_50hz==490||zl_50hz==205)      //mit r 2*56K
  //if(zl_50hz==340||zl_50hz==85)         //330,85 mit 68nF
      nulldurchgang();
  */if(mode==AB)
    {
/*      if(dimmzl!=255)
        dimmzl++;
      if(dimmzl>=dimm_I2C[0])
        K1OUT=1;
      if(dimmzl>=dimm_I2C[1])
        K2OUT=1;
*/
      if(dimmcompare<=dimm_I2C[0]&& dimm_I2C[0]!=0)K1OUT=0;
      else K1OUT=1;
      if(dimmcompare<=dimm_I2C[1]&& dimm_I2C[1]!=0)K2OUT=0;
      else K2OUT=1;
    }
  else
   {
      if(dimmcompare<=dimm_I2C[0]|| dimm_I2C[0]==0)K1OUT=1;
      else K1OUT=0;
      if(dimmcompare<=dimm_I2C[1]|| dimm_I2C[1]==0)K2OUT=1;
      else K2OUT=0;
    }
  //P0_1 =! P0_1;
}



void in50hz_init(void)
{
  P1M1 |= (1<<4); //Pin P1.4 input
  P1M2 &= ~(1<<4);
  EX1=1; //Externer interrupt ein
  IT1=1; //fallende flanke
  IP0 = 0x02;
  IP0H = 0x04;
  EA=1;  //globale interruptfreigabe
  return;
}


void main(void)
{

	int i=1;
	zero=zeropoint;
//TRIM-=9;

    i2c_sla_init();
    in50hz_init();
	rs_init(192);
	rs_send_s("\x1B[2J\x1B[0;33;1m--> Selfbus I2C Slave Dimmer V 1.0\n");

  P0M1 &= ~0xc0; //pin 0.6 und 0.7 Ausgang
  P0M2 |= 0xc0;
  K1OUT=0;
  K2OUT=0;

  TMOD=0x02;   // Timer 0 als reload
	TH0=0x70; //f�r 10ms / 256
	TL0=0x70; //f�r 10ms / 256

  //AUXR1&=~0x10;             // toggled whenever Timer0 overflows ausschalten
  ET0=1;                        // Interrupt f�r Timer 0 freigeben
  IP0 |= 0x02;
  TR0=1;                        // Timer 0 starten
  EA=1;

  P0M1 &= ~(1<<1); // P0_1 = EINGANG zum test
  P0M2 &= ~(1<<1);
  P0_1=1;

  P0M1 |= (1<<2); // P0_2 = EINGANG zum test
  P0M2 &= ~(1<<2);
  P0_2=0;

  if(P0_1==0)
    {
      mode=AN;    //anschnittdimmer
      rs_send_s("ANschnitt\n");
    }
  else
      rs_send_s("ABschnitt\n");

  dimm_I2C[0]=0;
  dimm_I2C[1]=0;

  while(1)
    {
      /*if(m<sp-1||m>sp+1)
        {
          rs_send_hex(sp>>8);
          rs_send_hex(sp&0xff);
          rs_send(' ');
          m=sp;
        }*/


	  if(RI){
		  RI=0;
		  cmd=SBUF;
		  rs_send_s("\x1B[2K\x1B[0G");   // clear line
		  if (cmd=='-')zero-=1;
		  if (cmd=='+')zero+=1;
		  if(cmd=='w')
		  {
		      EA=0;
		      START_WRITECYCLE	//cal an 0x1bff schreiben
		      WRITE_BYTE(0x1C,0xBC,zero)
		      STOP_WRITECYCLE
		      EA=1;				//int wieder freigeben
		      rs_send_s("Written to Flash, ");
		  }

		  if(cmd>=0x030&&cmd<=0x39)
		  {
			  dimm_I2C[0]=(cmd-0x30)*20;
			  rs_send_s("Dim Ch1: ");
			  rs_send_dec(dimm_I2C[0]);
		  }

		  rs_send_s("Zero: ");
		  rs_send_dec(zero);
	  }

#ifdef FB_DEBUG
	  DEBUGPOINT;
#endif

      if(dimm_I2C[0]!=mk[0]||dimm_I2C[1]!=mk[1])
         {
          //rs_send_s("\x1B[3K\x1B[0GDim: ");   // clear line
          rs_send_s("\nDim: ");   // clear line
          rs_send_hex(dimm_I2C[0]);
          rs_send(' ');
          rs_send_hex(dimm_I2C[1]);
          mk[0]=dimm_I2C[0];
          mk[1]=dimm_I2C[1];
         }
      else
        for (i=0;i<5000;++i);
    }
}
