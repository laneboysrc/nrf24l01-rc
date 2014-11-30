
__sfr __at (0x80) P0;
__sbit __at (0x80) P0_0;
__sbit __at (0x81) P0_1;
__sbit __at (0x82) P0_2;
__sbit __at (0x83) P0_3;
__sbit __at (0x84) P0_4;
__sbit __at (0x85) P0_5;
__sbit __at (0x86) P0_6;
__sbit __at (0x87) P0_7;

__sfr __at (0x81) SP;
__sfr __at (0x82) DPL;
__sfr __at (0x83) DPH;
__sfr __at (0x84) DPL1;
__sfr __at (0x85) DPH1;
__sfr __at (0x87) PCON;

__sfr16 __at (0x8584) DP1;

__sfr __at (0x88) TCON;
__sbit __at (0x88) TCON_it0;
__sbit __at (0x89) TCON_ie0;
__sbit __at (0x8a) TCON_it1;
__sbit __at (0x8b) TCON_ie1;
__sbit __at (0x8c) TCON_tr0;
__sbit __at (0x8d) TCON_tf0;
__sbit __at (0x8e) TCON_tr1;
__sbit __at (0x8f) TCON_tf1;

__sfr __at (0x89) TMOD;
__sfr __at (0x8a) TL0;
__sfr __at (0x8b) TL1;
__sfr __at (0x8c) TH0;
__sfr __at (0x8d) TH1;
__sfr __at (0x8f) P3CON;

__sfr16 __at (0x8c8a) TIMER0;
__sfr16 __at (0x8d8b) TIMER1;

__sfr __at (0x90) P1;
__sbit __at (0x90) P1_0;
__sbit __at (0x91) P1_1;
__sbit __at (0x92) P1_2;
__sbit __at (0x93) P1_3;
__sbit __at (0x94) P1_4;

__sfr __at (0x92) DPS;
__sfr __at (0x93) P0DIR;
__sfr __at (0x94) P1DIR;
__sfr __at (0x95) P2DIR;
__sfr __at (0x96) P3DIR;
__sfr __at (0x97) P2CON;

__sfr __at (0x98) S0CON;
__sbit __at (0x98) S0CON_ri0;
__sbit __at (0x99) S0CON_ti0;
__sbit __at (0x9a) S0CON_rb80;
__sbit __at (0x9b) S0CON_tb80;
__sbit __at (0x9c) S0CON_ren0;
__sbit __at (0x9d) S0CON_sm20;
__sbit __at (0x9e) S0CON_sm0;
__sbit __at (0x9f) S0CON_sm1;

__sfr __at (0x99) S0BUF;
__sfr __at (0x9e) P0CON;
__sfr __at (0x9f) P1CON;

__sfr __at (0xa0) P2;
__sfr __at (0xa1) PWMDC0;
__sfr __at (0xa2) PWMDC1;
__sfr __at (0xa3) CLKCTRL;
__sfr __at (0xa4) PWRDWN;
__sfr __at (0xa5) WUCON;
__sfr __at (0xa7) MEMCON;

__sfr __at (0xa8) IEN0;
__sbit __at (0xa8) IEN0_ifp;
__sbit __at (0xa9) IEN0_tf0;
__sbit __at (0xaa) IEN0_pwrfail;
__sbit __at (0xab) IEN0_tf1;
__sbit __at (0xac) IEN0_serial;
__sbit __at (0xad) IEN0_tf2;
__sbit __at (0xae) IEN0_6;
__sbit __at (0xaf) IEN0_all;

__sfr __at (0xa9) IP0;
__sfr __at (0xaa) S0RELL;
__sfr __at (0xab) RTC2CPT01;
__sfr __at (0xac) RTC2CPT10;
__sfr __at (0xad) CLKLFCTRL;
__sfr __at (0xae) OPMCON;
__sfr __at (0xaf) WDSW;

__sfr __at (0xb0) P3;
__sfr __at (0xb1) RSTREAS;
__sfr __at (0xb2) PWMCON;
__sfr __at (0xb3) RTC2CON;
__sfr __at (0xb4) RTC2CMP0;
__sfr __at (0xb5) RTC2CMP1;
__sfr __at (0xb6) RTC2CPT00;

__sfr __at (0xb8) IEN1;
__sbit __at (0xb8) IEN1_rfready;
__sbit __at (0xb9) IEN1_rfirq;
__sbit __at (0xba) IEN1_spi_iic;
__sbit __at (0xbb) IEN1_wakeup;
__sbit __at (0xbc) IEN1_misc;
__sbit __at (0xbd) IEN1_tick;
__sbit __at (0xbe) IEN1_6;
__sbit __at (0xbf) IEN1_exf2;

__sfr __at (0xb9) IP1;
__sfr __at (0xba) S0RELH;
__sfr __at (0xbc) SPISCON0;
__sfr __at (0xbe) SPISSTAT;
__sfr __at (0xbf) SPISDAT;

__sfr __at (0xc0) IRCON;
__sbit __at (0xc0) IRCON_rfready;
__sbit __at (0xc1) IRCON_rfiry;
__sbit __at (0xc2) IRCON_spi_iic;
__sbit __at (0xc3) IRCON_wakeup;
__sbit __at (0xc4) IRCON_misc;
__sbit __at (0xc5) IRCON_tick;
__sbit __at (0xc6) IRCON_tf2;
__sbit __at (0xc7) IRCON_exf2;

__sfr __at (0xc1) CCEN;
__sfr __at (0xc2) CCL1;
__sfr __at (0xc3) CCH1;
__sfr __at (0xc4) CCL2;
__sfr __at (0xc5) CCH2;
__sfr __at (0xc6) CCL3;
__sfr __at (0xc7) CCH3;

__sfr __at (0xc8) T2CON;
__sbit __at (0xc8) T2CON_t2i0;
__sbit __at (0xc9) T2CON_t2i1;
__sbit __at (0xca) T2CON_t2cm;
__sbit __at (0xcb) T2CON_t2r0;
__sbit __at (0xcc) T2CON_t2r1;
__sbit __at (0xcd) T2CON_i2fr;
__sbit __at (0xce) T2CON_i3fr;
__sbit __at (0xcf) T2CON_t2ps;

__sfr __at (0xc9) MPAGE;
__sfr __at (0xca) CRCL;
__sfr __at (0xcb) CRCH;
__sfr __at (0xcc) TL2;
__sfr __at (0xcd) TH2;
__sfr __at (0xce) WUOPC1;
__sfr __at (0xcf) WUOPC0;

__sfr16 __at (0xcdcc) TIMER2;

__sfr __at (0xd0) PSW;
__sbit __at (0xd1) PSW_f1;

__sfr __at (0xd1) ADCCON3;
__sfr __at (0xd2) ADCCON2;
__sfr __at (0xd3) ADCCON1;
__sfr __at (0xd4) ADCDATH;
__sfr __at (0xd5) ADCDATL;
__sfr __at (0xd6) RNGCTL;
__sfr __at (0xd7) RNGDAT;

__sfr __at (0xd8) ADCON;
__sbit __at (0xdf) ADCON_bd;

__sfr __at (0xd9) W2SADR;
__sfr __at (0xda) W2DAT;
__sfr __at (0xdb) COMPCON;
__sfr __at (0xdd) CCPDATIA;
__sfr __at (0xde) CCPDATIB;
__sfr __at (0xdf) CCPDATO;


__sfr __at (0xe0) ACC;
__sfr __at (0xe1) W2CON1;
__sfr __at (0xe2) W2CON0;
__sfr __at (0xe4) SPIRCON0;
__sfr __at (0xe5) SPIRCON1;
__sfr __at (0xe6) SPIRSTAT;
__sfr __at (0xe7) SPIRDAT;

__sfr __at (0xe8) RFCON;
__sbit __at (0xe8) RFCON_rfce;
__sbit __at (0xe9) RFCON_rfcsn;
__sbit __at (0xea) RFCON_rfcken;

__sfr __at (0xe9) MD0;
__sfr __at (0xea) MD1;
__sfr __at (0xeb) MD2;
__sfr __at (0xec) MD3;
__sfr __at (0xed) MD4;
__sfr __at (0xee) MD5;
__sfr __at (0xef) ARCON;

__sfr __at (0xf8) FSR;
__sbit __at (0xf8) FSR_0;
__sbit __at (0xf9) FSR_1;
__sbit __at (0xfa) FSR_rdismb;
__sbit __at (0xfb) FSR_infen;
__sbit __at (0xfc) FSR_rdyn;
__sbit __at (0xfd) FSR_wen;
__sbit __at (0xfe) FSR_stp;
__sbit __at (0xff) FSR_endebug;

__sfr __at (0xf9) FPCR;
__sfr __at (0xfa) FCR;
__sfr __at (0xfc) SPIMCON0;
__sfr __at (0xfd) SPIMCON1;
__sfr __at (0xfe) SPIMSTAT;
__sfr __at (0xff) SPIMDAT;

