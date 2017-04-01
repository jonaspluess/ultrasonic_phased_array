EESchema Schematic File Version 2
LIBS:74xgxx
LIBS:74xx
LIBS:ac-dc
LIBS:actel
LIBS:adc-dac
LIBS:Altera
LIBS:analog_devices
LIBS:analog_switches
LIBS:atmel
LIBS:audio
LIBS:brooktre
LIBS:cmos4000
LIBS:cmos_ieee
LIBS:conn
LIBS:contrib
LIBS:cypress
LIBS:dc-dc
LIBS:device
LIBS:digital-audio
LIBS:diode
LIBS:display
LIBS:dsp
LIBS:elec-unifil
LIBS:ESD_Protection
LIBS:ftdi
LIBS:gennum
LIBS:graphic
LIBS:hc11
LIBS:intel
LIBS:interface
LIBS:ir
LIBS:Lattice
LIBS:linear
LIBS:logo
LIBS:maxim
LIBS:memory
LIBS:microchip
LIBS:microchip_dspic33dsc
LIBS:microchip_pic10mcu
LIBS:microchip_pic12mcu
LIBS:microchip_pic16mcu
LIBS:microchip_pic18mcu
LIBS:microchip_pic32mcu
LIBS:microcontrollers
LIBS:motor_drivers
LIBS:motorola
LIBS:msp430
LIBS:nordicsemi
LIBS:nxp_armmcu
LIBS:onsemi
LIBS:opto
LIBS:Oscillators
LIBS:philips
LIBS:power
LIBS:powerint
LIBS:Power_Management
LIBS:pspice
LIBS:references
LIBS:regul
LIBS:relays
LIBS:rfcom
LIBS:sensors
LIBS:silabs
LIBS:siliconi
LIBS:stm8
LIBS:stm32
LIBS:supertex
LIBS:switches
LIBS:texas
LIBS:transf
LIBS:transistors
LIBS:ttl_ieee
LIBS:valves
LIBS:video
LIBS:Worldsemi
LIBS:Xicor
LIBS:xilinx
LIBS:Zilog
LIBS:Symbols_DCDC-ACDC-Converter_RevC_20Jul2012
LIBS:Symbols_EN60617_13Mar2013
LIBS:Symbols_EN60617-10_HF-Radio_DRAFT_12Sep2013
LIBS:Symbols_ICs-Diskrete_RevD10
LIBS:Symbols_ICs-Opto_RevB_16Sep2013
LIBS:Symbols_Microcontroller_Philips-NXP_RevA_06Oct2013
LIBS:SymbolsSimilarEN60617+oldDIN617-RevE8
LIBS:Symbols_Socket-DIN41612_RevA
LIBS:Symbols_Transformer-Diskrete_RevA
LIBS:pcb-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L TFBS4711 P2
U 1 1 584930B9
P 3200 1850
F 0 "P2" H 3200 2200 50  0000 C CNN
F 1 "TFBS4711" V 3300 1850 50  0000 C CNN
F 2 "lib:TFBS4711" H 3200 1850 50  0001 C CNN
F 3 "" H 3200 1850 50  0000 C CNN
	1    3200 1850
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X06 P1
U 1 1 584930E0
P 2700 1850
F 0 "P1" H 2700 2200 50  0000 C CNN
F 1 "CONN_01X06" V 2800 1850 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x06" H 2700 1850 50  0001 C CNN
F 3 "" H 2700 1850 50  0000 C CNN
	1    2700 1850
	-1   0    0    1   
$EndComp
Wire Wire Line
	2900 1600 3000 1600
Wire Wire Line
	2900 1700 3000 1700
Wire Wire Line
	2900 1800 3000 1800
Wire Wire Line
	2900 1900 3000 1900
Wire Wire Line
	2900 2000 3000 2000
Wire Wire Line
	2900 2100 3000 2100
$Comp
L TFBS4711 P4
U 1 1 58493717
P 4150 1850
F 0 "P4" H 4150 2200 50  0000 C CNN
F 1 "TFBS4711" V 4250 1850 50  0000 C CNN
F 2 "lib:TFBS4711" H 4150 1850 50  0001 C CNN
F 3 "" H 4150 1850 50  0000 C CNN
	1    4150 1850
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X06 P3
U 1 1 5849371D
P 3650 1850
F 0 "P3" H 3650 2200 50  0000 C CNN
F 1 "CONN_01X06" V 3750 1850 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x06" H 3650 1850 50  0001 C CNN
F 3 "" H 3650 1850 50  0000 C CNN
	1    3650 1850
	-1   0    0    1   
$EndComp
Wire Wire Line
	3850 1600 3950 1600
Wire Wire Line
	3850 1700 3950 1700
Wire Wire Line
	3850 1800 3950 1800
Wire Wire Line
	3850 1900 3950 1900
Wire Wire Line
	3850 2000 3950 2000
Wire Wire Line
	3850 2100 3950 2100
$EndSCHEMATC
