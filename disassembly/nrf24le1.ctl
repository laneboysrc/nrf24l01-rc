
;f 88 TCON
k 88 TCON_it0
k 89 TCON_ie0
k 8a TCON_it1
k 8b TCON_ie1
k 8c TCON_tr0
k 8d TCON_tf0
k 8e TCON_tr1
k 8f TCON_tf1

f 93 P0DIR
f 94 P1DIR

f 98 S0CON
k 98 S0CON_ri0
k 99 S0CON_ti0
k 9a S0CON_rb80
k 9b S0CON_tb80
k 9c S0CON_ren0
k 9d S0CON_sm20
k 9e S0CON_sm0
k 9f S0CON_sm1

f 99 S0BUF
f 9e P0CON
f 9f P1CON

f a1 PWMDC0
f a2 PWMDC1
f a3 CLKCTRL
f a4 PWRDWN
f a5 WUCON
f a7 MEMCON

f a8 IEN0
k a8 IEN0_ifp
k a9 IEN0_tf0
k aa IEN0_pwrfail
k ab IEN0_tf1
k ac IEN0_serial
k ad IEN0_tf2
k ae IEN0_6
k af IEN0_all

f a9 IP0
f aa S0RELL
f ab RTC2CPT01
f ac RTC2CPT10
f ad CLKLFCTRL
f ae OPMCON
f af WDSW

f b1 RSTREAS
f b2 PWMCON
f b3 RTC2CON
f b4 RTC2CMP0
f b5 RTC2CMP1
f b6 RTC2CPT00

f b8 IEN1
k b8 IEN1_rfready
k b9 IEN1_rfirq
k ba IEN1_spi_iic
k bb IEN1_wakeup
k bc IEN1_misc
k bd IEN1_tick
k be IEN1_6
k bf IEN1_exf2

f b9 IP1
f ba S0RELH
f bc SPISCON0
f be SPISSTAT
f bf SPISDAT

f c0 IRCON
k c0 IRCON_rfready
k c1 IRCON_rfiry
k c2 IRCON_spi_iic
k c3 IRCON_wakeup
k c4 IRCON_misc
k c5 IRCON_tick
k c6 IRCON_tf2
k c7 IRCON_exf2

f c1 CCEN
f c2 CCL1
f c3 CCH1
f c4 CCL2
f c5 CCH2
f c6 CCL3
f c7 CCH3

;f c8 T2CON
k c8 T2CON_t2i0
k c9 T2CON_t2i1
k ca T2CON_t2cm
k cb T2CON_t2r0
k cc T2CON_t2r1
k cd T2CON_i2fr
k ce T2CON_i3fr
k cf T2CON_t2ps

f c9 MPAGE
f ca CRCL
f cb CRCH
    f ce WUOPC1
f cf WUOPC0

;f d0 PSW
k d1 PSW_f1

f d1 ADCCON3
f d2 ADCCON2
f d3 ADCCON1
f d4 ADCDATH
f d5 ADCDATL
f d6 RNGCTL
f d7 RNGDAT

;f d8 ADCON
k df ADCON_bd

f d9 W2SADR
f da W2DAT
f db COMPCON
f dd CCPDATIA
f de CCPDATIB
f df CCPDATO

f e1 W2CON1
f e2 W2CON0
f e4 SPIRCON0
f e5 SPIRCON1
f e6 SPIRSTAT
f e7 SPIRDAT

f e8 RFCON
k e8 RFCON_rfce
k e9 RFCON_rfcsn
k ea RFCON_rfcken

f e9 MD0
f ea MD1
f eb MD2
f ec MD3
f ed MD4
f ee MD5
f ef ARCON

f f8 FSR
k f8 FSR_0
k f9 FSR_1
k fa FSR_rdismb
k fb FSR_infen
k fc FSR_rdyn
k fd FSR_wen
k fe FSR_stp
k ff FSR_endebug

f f9 FPCR
f fa FCR
f fc SPIMCON0
f fd SPIMCON1
f fe SPIMSTAT
f ff SPIMDAT

