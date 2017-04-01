
* OPA350 SPICE Macro-model              3/31/99, Rev. B   by Marek Lis
* Rev. A  12/18/98,  by Marek Lis
* REV. B  23 DEC 1998 NEIL ALBAUGH; REVISED CONNECTION NAMES TO MATCH SYMBOL
* Rev. C  3/31/99,  by Marek Lis: added voltage and current noise (1/f noise is NOT modeled.)
* REV. D  1 APR 1999 NEIL ALBAUGH; REVISED CONNECTION NAMES TO MATCH SYMBOL
*
*
*
*
*
* Copyright 1997 by Burr-Brown Corporation

*

*                non-inverting input

*                |  inverting input

*                |  |  positive supply

*                |  |  |   negative supply

*                |  |  |   |   output

*                |  |  |   |   |

.SUBCKT OPA350   +  -  V+  V-  OUT

* INPUT STAGE

*

i1 V+ 5 400u

m7 550 vswitch 5 5 pix l=2u w=25u m=26

m8 550 550 V- V- nix l=2u w=25u m=26

m9 553a 550 V- V- nix l=2u w=25u m=26

m9c 66 nvsat 553a V- nix l=2u w=25u m=26

Vpvsat V+ vswitch DC 1.8

Vnvsat nvsat V- DC 1.37

iin1 + 98 .5p

iin2 - 98 .5p

d3 5 V+ dx

d4 V- 66 dx

d5 - V+ dx

d6 + V+ dx

d7 V- - dx

d8 V- + dx

rinp 7 7a 500

rinn - 2a 500

m1 33 2a 66 V- nix l=2u w=25u m=13

m2 4 7a 66 V- nix l=2u w=25u m=13

m3 8 2a 5 5 pix l=2u w=25u m=13

m4 9 7a 5 5 pix l=2u w=25u m=13

eos 7 + poly(1) 25 98 0 0

ios - + 0p

r1 V+ 33x 4.833k

r2 V+ 4x 4.833k

r3 8x V- 4.833k

r4 9x V- 4.833k

vr1 33 33x DC 2

vr2 4 4x DC 2

vr3 8x 8 DC 2

vr4 9x 9 DC 2

i1a V+ V- 2464u

*

* GAIN STAGE

*

eref 98 0 poly(2) V+ 0 V- 0 0 0.5 0.5

g1 98 21 poly(2) 4 33 9 8 0 145u 145u

rg 21 98 2.3e6

cc 21 6c 10.6pF

rcc 6c OUT 2.4k

d1 21 22 dx

d2 23 21 dx

v1 V+ 22 1.37

v2 23 V- 1.37


*

* COMMON MODE GAIN STAGE

*

ecm 24 98 poly(2) + 98 - 98 0 0.5 0.5

r5 24 25 1e6

r6 25 98 10k

c1 24 25 0.75p

*

* OUTPUT STAGE

*

isy V+ V- 540.4u

gsy V+ V- poly(1) V+ V- -3.334e-4 6.667e-5

ep V+ 39 poly(1) 98 21 0.78925 1

en 38 V- poly(1) 21 98 0.78925 1

vh OUT 6h DC -2.4e-2

vl 6l OUT DC -2.4e-2

m113 6h 39 V+ V+ pox l=1.5u w=1500u m=12

m114 6l 38 V- V- nox l=1.5u w=1500u m=12

c15 OUT 39 15p

c16 OUT 38 15p

.model dx d(rs=1 cjo=0.1p)

.model nix nmos(vto=0.75 kp=205.5u rd=1 rs=1 rg=1 rb=1 cgso=4e-9

+cgdo=4e-9 cgbo=16.667e-9 cbs=2.34e-13 cbd=2.34e-13)

.model nox nmos(vto=0.75 kp=195u rd=.5 rs=.5 rg=1 rb=1 cgso=66.667e-12

+cgdo=66.667e-12 cgbo=125e-9 cbs=2.34e-13 cbd=2.34e-13)

.model pix pmos(vto=-0.75 kp=205.5u rd=1 rs=1 rg=1 rb=1 cgso=4e-9

+cgdo=4e-9 cgbo=16.667e-9 cbs=2.34e-13 cbd=2.34e-13)

.model pox pmos(vto=-0.75 kp=195u rd=.5 rs=.5 rg=1 rb=1 cgso=66.667e-12

+cgdo=66.667e-12 cgbo=125e-9 cbs=2.34e-13 cbd=2.34e-13)

.ENDS OPA350

