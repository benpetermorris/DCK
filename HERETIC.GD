# *****************************************************************************
# DCK game definition file.
# *****************************************************************************
# Don't change the next line.
# I mean that.
# Don't change it.
GAMEDATA
# You had better not have changed it.

[General]
Name; Heretic
# Make sure the following line is correct
Directory; [Undefined]
IWAD; HERETIC.WAD
# It's not a good idea to change the next line
Tag; E'1M'1
New Map; E1M1
Run; heretic.exe -warp $num1 $num2
Palette; HERETIC.PAL
Nodes; zennode /nq /rz $FULL $MAP /o $FULL

[Texture Files]
# This is a list of .WAD files to search for textures.  The IWAD is
# automatically used, so if you don't have any others .. don't put anything
# here!

[Things: Player Starts]

# A thing with the name of "Color" is not actually inserted into the thing
# list, but sets all following thing colors to the color specified.
# Please note that the following colors are valid ONLY:
#       Blue Green Cyan Red Magenta Brown Grey DarkGrey
#       LightBlue LightGreen LightCyan LightRed LightMagenta Yellow White

Color; Green
Player 1 Start; 1; IN1
Player 2 Start; 2; IN2
Player 3 Start; 3; IN3
Player 4 Start; 4; IN4
Color; LightGreen
Deathmatch Start; 11; TELEA0
Teleport Exit; 14; TELEA0
TeleGlitter; 74; TELEA0
TeleGlitter Exit; 52; TELEA0

[Things: Monsters]
Color; Red
Gargoyle; 66; IMPXB1
Golem; 68; MUMME1
Golem Ghost; 69; MUMME1
Undead Warrior; 64; KNIGF1
Sabreclaw; 90; CLNKF1
Color; LightRed
Disciple; 15; WZRDC1
Weredragon; 70; BEASI1
Ironlich; 6; HEADB1
Gargoyle Leader; 5; IMPXE1
Undead Warrior Ghost; 65; KNIGG1
Maulotaur; 9; MNTRV8
Golem Leader; 45; MUMMY1
Golem Leader Ghost; 46; MUMMY1
Ophidian; 92; SNKED1
D'Sparil; 7; SRCRB2
D'Sparil Spot; 56; SOR2I0

[Things: Keys]
Color; Magenta
Blue Key; 79; BKYYA0
Green Key; 73; AKYYA0
Yellow Key; 80; CKYYA0

[Things: Health and Armor]
Color; White
Healing Complete (Urn); 32; ARTISPHL
Healing Hefty; 82; ARTIPTN2
Healing Wimpy; 81; PTN1A0
Shield 1; 85; SHLDA0
Shield 2; 31; SHD2A0

[Things: Weapons]
Color; Blue
DragonClaw; 53; WBLSA0
Crossbow; 2001; WBOWA0
Gauntlets; 2005; WGNTA0
Color; LightBlue
Mace; 2002; WMCEA0
Phoenix Rod; 2003; WPHXA0
Hellstaff; 2004; WSKLA0

[Things: Ammo]
Color; Grey
Energy Orb Hefty; 55; AMB2A0
Energy Orb Wimpy; 54; AMB1A0
Arrows Hefty; 19; AMC2A0
Arrows Wimpy; 18; AMC1A0
Wand Crystal Hefty; 12; AMG2A0
Wand Crystal Wimpy; 10; AMG1A0
Mace Spheres Hefty; 16; AMM2A0
Mace Spheres Wimpy; 13; AMM1A0
Phoenix Orb Hefty; 23; AMP2A0
Phoenix Orb Wimpy; 22; AMP1A0
Hellstaff Rune Hefty; 21; AMS2A0
Hellstaff Rune Wimpy; 20; AMS1A0
Bag Of Holding; 8; BAGHA0

[Things: Artifacts]
Color; LightMagenta
Egg; 30; EGGCA0
Time Bomb; 34; ARTIFBMB
Wings of Wrath; 83; ARTISOAR
Invisibility; 75; ARTIINVS
Invulnerability; 84; ARTIINVU
Map Scroll; 35; SPMPA0
Chaos Device; 36; ARTIATLP
Tome Of Power; 86; ARTIPWBK
Torch; 33; ARTITRCH

[Things: Decorations]
Color; Brown
Barrel; 44; BARLA0
Brown Pillar; 47; BRPLA0
Chandelier; 28; CHDLB0
Fire Brazier; 76; KFR1A0
Hanging Corpse; 51; HCORA0
KeyGizmo Blue; 94; KGZBA0
KeyGizmo Green; 95; KGZGA0
KeyGizmo Yellow; 96; KGZYA0
Moss 1; 48; MOS1A0
Moss 2; 49; MOS2A0
Serpent Torch; 27; SRTCA0
Skull Hang 35; 26; SKH4A0
Skull Hang 45; 25; SKH3A0
Skull Hang 60; 24; SKH2A0
Skull Hang 70; 17; SKH1A0
Small Pillar; 29; SMPLA0
Stalactite Large; 40; STCLA0
Stalactite Small; 39; STCSA0
Stalagmite Large; 38; STGLA0
Stalagmite Small; 37; STGSA0
Volcano; 87; VLCOE0
Wall Torch; 50; WTRHC0

[Things: Pods]
Color; Yellow
Pod; 2035; PPODA0
Pod Generator; 43; PPODK0

[Things: Sounds]
Color; Cyan
A1_Bells; 1205;
A1_Drops; 1202;
A1_FastFootsteps; 1209;
A1_Heartbeat; 1204;
A1_Laughter; 1208;
A1_SlowFootsteps; 1203;
A2_Growl; 1206;
A2_Magic; 1207;
A2_Scream; 1200;
A2_Squish; 1201;
E1_Wind; 42;
E2_Waterfall; 41;

[Actions: Normal]
Normal Line (0); 0

[Actions: Door Open-Close]
DR Door OpenWaitClose (1); 1
S1 Door OpenWaitClose (29); 29
SR Door OpenWaitClose (63); 63
W1 Door OpenWaitClose (4); 4
WR Door OpenWaitClose (90); 90
WR Door OpenWaitClose Turbo (100); 100
DR Door OpenWaitClose Blue (26); 26
DR Door OpenWaitClose Green (28); 28
DR Door OpenWaitClose Yellow (27); 27

[Actions: Door Open-Stay]
DR Door OpenStay (31); 31
SR Door OpenStay (61); 61
S1 Door OpenStay (103); 103
W1 Door OpenStay (2); 2
WR Door OpenStay (86); 86
GR Door OpenStay (46); 46
DR Door OpenStay Blue (32); 32
DR Door OpenStay Green (33); 33
DR Door OpenStay Yellow (34); 34

[Actions: Door Close]
WR Door Close For 30 Seconds (76); 76
W1 Door Close For 30 Seconds (16); 16
SR Door Close (42); 42
S1 Door Close (50); 50
WR Door Close (75); 75
W1 Door Close (3); 3

[Actions: Platforms]
SR Plat DownWaitUpStay (62); 62
S1 Plat DownWaitUpStay (21); 21
W1 Plat DownWaitUpStay (10); 10
WR Plat DownWaitUpStay (88); 88
W1 Plat Start UpWaitDownWait (53); 53
WR Plat Start UpWaitDownWait (87); 87
W1 Plat Stop UpWaitDownWait (54); 54
WR Plat Stop UpWaitDownWait (89); 89

[Actions: Lighting]
SR Lights to 255 (138); 138
SR Lights to 0 (139); 139
WR Lights To Dimmest Near (78); 78
W1 Lights To Dimmest Near (104); 104
W1 Lights to 255 (13); 13
WR Lights to 255 (81); 81
W1 Lights to Brightest Near (12); 12
WR Lights to Brightest Near (80); 80
W1 Lights to 0 (35); 35
WR Lights to 0 (79); 79
W1 Lights Start Strobing (17); 17

[Actions: Teleports and Stairs]
W1 Teleport (39); 39
WR Teleport (97); 97
W1 Teleport Monster Only (125); 125
WR Teleport Monster Only (125); 126
S1 Stairs Build 8 High (7); 7
W1 Stairs Build 8 High (8); 8
S1 Stairs Build 16 Turbo (107); 107

[Actions: Crushers, etc]
W1 Ceiling Crush Start FDmg (6); 6
WR Ceiling Crush Start FDmg (73); 73
W1 Ceiling Crush Start SDmg (25); 25
WR Ceiling Crush Start SDmg (77); 77
S1 Ceiling Crush Start SDmg (49); 49
W1 Ceiling Crush&Stay (44); 44
WR Ceiling Crush&Stay (72); 72
W1 Ceiling Crush Stop (57); 57
WR Ceiling Crush Stop (74); 74
S1 Ceiling Lower To Floor (41); 41
SR Ceiling Lower To Floor (43); 43
SR Floor Raise Crush (65); 65
S1 Floor Raise Crush (55); 55
W1 Floor Raise Crush (56); 56
WR Floor Raise Crush (94); 94

[Actions: Exits & Scroll Effect]
S1 Exit Level (11); 11
W1 Exit Level (52); 52
S1 Exit To 9 (Secret Level) (51); 51
W1 Exit To 9 (Secret Level) (105); 105
Effect Scroll Left (48); 48
Effect Scroll Right (99); 99

[Actions: Floor Lowers]
S1 Floor Change Donut (9); 9
W1 Floor Lower To Near (19); 19
SR Floor Lower To Near (45); 45
S1 Floor Lower To Near (102); 102
WR Floor Lower To Near (83); 83
S1 Floor Lower To Lowest (23); 23
SR Floor Lower To Lowest (60); 60
W1 Floor Lower To Lowest (38); 38
WR Floor Lower To Lowest (82); 82
SR Floor Lower Turbo (70); 70
S1 Floor Lower Turbo (71); 71
W1 Floor Lower Turbo (36); 36
WR Floor Lower Turbo (98); 98
W1 Floor Lower & Change (37); 37
WR Floor Lower & Change (84); 84
W1 Floor Lower Ceiling Raise (40); 40
WR Floor Lower Ceiling Raise (85); 85

[Actions: Floor Raises]
G1 Floor Raise (24); 24
G1 Floor Raise Near & Change (47); 47
SR Floor Raise (64); 64
S1 Floor Raise (101); 101
W1 Floor Raise (5); 5
WR Floor Raise (91); 91
S1 Floor Raise Turbo (131); 131
SR Floor Raise Turbo (132); 132
W1 Floor Raise Turbo (130); 130
WR Floor Raise Turbo (129); 129
S1 Floor Raise To Nearest (18); 18
W1 Floor Raise To Nearest (119); 119
SR Floor Raise To Nearest (69); 69
WR Floor Raise To Nearest (128); 128
S1 Floor Raise 24 (15); 15
SR Floor Raise 24 (66); 66
W1 Floor Raise 24 (58); 58
WR Floor Raise 24 (92); 92
S1 Floor Raise 32 (14); 14
SR Floor Raise 32 (67); 67
W1 Floor Raise 24 & Change (59); 59
WR Floor Raise 24 & Change (93); 93
S1 Floor Raise Near & Change (20); 20
W1 Floor Raise Near & Change (22); 22
SR Floor Raise Near & Change (68); 68
WR Floor Raise Near & Change (95); 95
W1 Floor Raise To Short Tex (30); 30
WR Floor Raise To Short Tex (96); 96

[Sectors]
Normal; 0
Ceiling_Crush&Raise; 6
Damage_LavaFlow; 4
Damage_LavaHefty; 16
Damage_LavaWimpy; 5
Damage_Sludge; 7
Door_Close_in_30; 10
Door_Raise_in_5_Mins; 14
Friction_Low; 15
Light_Fast_Strobe; 2
Light_Flickering; 1
Light_Glowing; 8
Light_Slow_Strobe; 3
Light_SynchStrobeFast; 13
Light_SynchStrobeSlow; 12
# Scroll specials scroll the floor texture and push the player.
Scroll_EastFast; 23
Scroll_EastNormal; 22
Scroll_EastSlow; 21
Scroll_NorthFast; 28
Scroll_NorthNormal; 27
Scroll_NorthSlow; 26
Scroll_SouthFast; 33
Scroll_SouthNormal; 32
Scroll_SouthSlow; 31
Scroll_WestFast; 38
Scroll_WestNormal; 37
Scroll_WestSlow; 36
SecretArea; 9
# Wind specials push objects that are flagged to be affected by wind.
# Here's a (not sure if it's complete) list:  players, pods, ethereal
# bow PL1&PL2 side shots, hellstaff PL1, undead warrior green axes,
# weredragon fireballs, ophidian wimpy shots, ironlich fire column,
# and gargoyle leader fireballs.
Wind_EastNormal; 41
Wind_EastStrong; 42
Wind_EastWeak; 40
Wind_NorthNormal; 44
Wind_NorthStrong; 45
Wind_NorthWeak; 43
Wind_SouthNormal; 47
Wind_SouthStrong; 48
Wind_SouthWeak; 46
Wind_WestNormal; 50
Wind_WestStrong; 51
Wind_WestWeak; 49

[Angles]
North; 90
South; 270
East; 0
West; 180
Northeast; 45
Northwest; 135
Southeast; 315
Southwest; 225

[Thing Info]
# players, dm, teleport.. etc
1; 20; 56; 1
2; 20; 56; 1
3; 20; 56; 1
4; 20; 56; 1
14; 20; 56; 1
11; 20; 56; 1

# monsters
66; 16; 56; 1
68; 30; 56; 1
69; 30; 56; 1
64; 20; 56; 1
90; 24; 56; 1
15; 20; 56; 1
70; 32; 56; 1
6; 32; 56; 1
5; 16; 56; 1
65; 20; 56; 1
9; 40; 56; 1
45; 30; 56; 1
46; 30; 56; 1
92; 24; 56; 1
7; 40; 56; 1
56; 20; 56; 1

#other stuff
79; 19; 16; 0
73; 19; 16; 0
80; 19; 16; 0
32; 19; 16; 0
82; 19; 16; 0
81; 19; 16; 0
85; 19; 16; 0
31; 19; 16; 0
53; 19; 16; 0
2001; 19; 16; 0
2005; 19; 16; 0
2002; 19; 16; 0
2003; 19; 16; 0
2004; 19; 16; 0
55; 19; 16; 0
54; 19; 16; 0
19; 19; 16; 0
18; 19; 16; 0
12; 19; 16; 0
10; 19; 16; 0
16; 19; 16; 0
13; 19; 16; 0
23; 19; 16; 0
22; 19; 16; 0
21; 19; 16; 0
20; 19; 16; 0
8; 19; 16; 0
30; 19; 16; 0
34; 19; 16; 0
83; 19; 16; 0
75; 19; 16; 0
84; 19; 16; 0
35; 19; 16; 0
36; 19; 16; 0
86; 19; 16; 0
33; 19; 16; 0
48; 19; 16; 0
49; 19; 16; 0
50; 19; 16; 0
43; 19; 16; 0
1205; 19; 16; 0
1202; 19; 16; 0
1209; 19; 16; 0
1204; 19; 16; 0
1208; 19; 16; 0
1203; 19; 16; 0
1206; 19; 16; 0
1207; 19; 16; 0
1200; 19; 16; 0
1201; 19; 16; 0
42; 19; 16; 0
41; 19; 16; 0
26; 19; 16; 0
25; 19; 16; 0

[Actions without Tag]
# A list of all the line types that don't require tags.
1
99
26
28
27
117
31
32
33
34
118
48
124
11
52
51
