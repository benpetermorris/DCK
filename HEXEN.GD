# *****************************************************************************
# DCK game definition file.
# *****************************************************************************
# Don't change the next line.
# I mean that.
# Don't change it.
GAMEDATA
# You had better not have changed it.

[General]
Name; Hexen
# Make sure the following line is correct
Directory; D:\HEXEN
IWAD; HEXEN.WAD
# It's not a good idea to change the next line
Tag; MAP'2
New Map; MAP01
Run; hexen.exe -warp $NUM1
Palette; HEXEN.PAL
Nodes; zennode /nq /rz $FULL $MAP /o $FULL
LargeData; 1

[Texture Files]
# This is a list of .WAD files to search for textures.  The IWAD is
# automatically used, so if you don't have any others .. don't put anything
# here!

# A thing with the name of "Color" is not actually inserted into the thing
# list, but sets all following thing colors to the color specified.
# Please note that the following colors are valid ONLY:
#       Blue Green Cyan Red Magenta Brown Grey DarkGrey
#       LightBlue LightGreen LightCyan LightRed LightMagenta Yellow White

[Things: Player Starts]
Color; Green
Player 1 Start; 1; INRED1
Player 2 Start; 2; INRED2
Player 3 Start; 3; INRED3
Player 4 Start; 4; INRED4
Player 5 Start; 9100; INRED5
Player 6 Start; 9101; INRED6
Player 7 Start; 9102; INRED7
Player 8 Start; 9103; INRED8
Color; LightGreen
Deathmatch Start; 11;
Teleport Landing; 14; TELEA0

[Things: Weapons & Ammo]
Color; Blue
W_Serpent Staff (C2); 10; WCSSA0
W_Timon's Axe (F2); 8010; WFAXA0
W_Frost Shards (M2); 53; WMCSA0
W_Firestorm (C3); 8009; WCFMA0
W_Hammer of Retribution (F3); 123; WFHMA0
W_Arc of Death (M3); 8040; WMLGF0
Color; LightBlue
W_WraithVerge-1 (C4); 18; WCH1A0
W_WraithVerge-2 (C4); 19; WCH2A0
W_WraithVerge-3 (C4); 20; WCH3A0
W_Quietus-1 (F4); 12; WFR1A0
W_Quietus-2 (F4); 13; WFR2A0
W_Quietus-3 (F4); 16; WFR3A0
W_BloodScourge-1 (M4); 21; WMS1A0
W_BloodScourge-2 (M4); 22; WMS2A0
W_BloodScourge-3 (M4); 23; WMS3A0
Color; Grey
Am_Blue Mana; 122; MAN1A0
Am_Green Mana; 124; MAN2A0
Am_Combined Mana; 8004; MAN3O0

[Things: Artifacts & Items]
Color; LightMagenta
A_Disc of Repulsion; 10110; ARTIBLST
A_Krater of Might; 8003; ARTIBMAN
A_Boots of Speed; 8002; ARTISPED
A_Bracers; 8041; ARTIBRAC
A_Icon of the Defender; 84; ARTIINVU
A_Porkelator; 30; ARTIPORK
A_Wings of Wrath; 83; ARTISOAR
A_Resurrection; 10120; ARTIHRAD
A_Flechette; 8000; ARTIPSBG
A_Dark Servant; 86; ARTISUMN
A_Chaos Device; 36; ARTIATLP
A_Torch; 33; ARTITRCH
A_Banishment; 10040; ARTITELO

[Things: Armor & Healing]
Color; White
Ar_Amulet of Warding; 8008; ARM4A0
Ar_Mesh Armor; 8005; ARM1A0
Ar_Platinum Helm; 8007; ARM3A0
Ar_Falcon Shield; 8006; ARM2A0
H_Healing Complete (Urn); 32; ARTISPHL
H_Healing Hefty (Flask); 82; ARTIPTN2
H_Healing Wimpy (Vial); 81; PTN1A0

[Things: Enemies!]
Color; Red
E_Centaur; 107; CENTD1
E_Ettin; 10030; ETTNC1
E_Afrit; 10060; FDMNB8
E_Wendigo; 8020; ICEYA1
E_Green Chaos Serpent; 31; DEMNB2B8
E_Brown Chaos Serpent; 8080; DEM2G2G8
Color; LightRed
E_Dark Bishop; 114; BISHF1
E_Slaughtaur; 115; CENTF1
E_Traductus (Cleric); 10101; CLERA5
E_Death Wyvern; 254; DRAGE2E8
E_Zedek (Fighter); 10100; PLAYA5
E_Heresiarch; 10080; SORCF1
E_Korax; 10200; KORXD1
E_Menelkir (Mage); 10102; MAGEA5
E_Stalker; 121; SSDVC0
E_Stalker Shooter; 120; SSPTK1
E_Reiver; 34; WRTHA1
E_Reiver Buried; 10011; WRT2A0

[Things: Keys]
Color; Magenta
K_Steel Key; 8030; KEY1A0
K_Cave Key; 8031; KEY2A0
K_Axe Key; 8032; KEY3A0
K_Fire Key; 8033; KEY4A0
K_Emerald Key; 8034; KEY5A0
K_Dungeon Key; 8035; KEY6A0
K_Silver Key; 8036; KEY7A0
K_Rusty Key; 8037; KEY8A0
K_Horn Key; 8038; KEY9A0
K_Swamp Key; 8039; KEYAA0
K_Castle Key; 8200; KEYBA0

[Things: PolyObjs, MapSpots, Spawn]
Color; LightCyan
PO_Anchor; 3000;
PO_StartSpot; 3001;
PO_StartSpot_Crush; 3002;
Spawn_Bat; 10225; ABATA1
Spawn_Fog; 10000; FOGLA0
Spawn_Fog_a; 10001; FOGSA0
Spawn_Fog_b; 10002; FOGMA0
Spawn_Fog_c; 10003; FOGLA0
Spawn_Leaf; 113; LEF1G0
Spike_Down; 10090; TSPKA0
Spike_Up; 10091; TSPKA0
X_MapSpot; 9001;
X_MapSpotGravity; 9013;

[Things: Sounds]
Color; Cyan
SS_Creak; 1403;
SS_EarthCrack; 1408;
SS_Heavy; 1401;
SS_Ice; 1407;
SS_Lava; 1405;
SS_Metal; 1402;
SS_Metal2; 1409;
SS_Silent; 1404;
SS_Stone; 1400;
SS_Water; 1406;
SE_Wind; 1410;

[Things: Unclassified]
Color; Grey
Z_ArmorSuit; 8064; SUITA0
Z_Banner; 77; BNR1A0
Z_Barrel; 8100; BARLA0
Z_Bell; 8065; BBLLA0
Z_BlueCandle; 8066; CANDA0
Z_BrassBrazier; 8061; BRTRA0
Z_Bucket; 8103; BCKTA0
Z_Candle; 119; CNDLA0
Z_Cauldron; 8069; CDRND0
Z_Cauldron Unlit; 8070; CDRNA0
Z_Chain32; 8071; CHNSA0
Z_Chain64; 8072; CHNSB0
Z_ChainHeart; 8073; CHNSC0
Z_ChainLargeHook; 8074; CHNSD0
Z_ChainSmallHook; 8075; CHNSE0
Z_ChainSkull; 8077; CHNSG0
Z_ChainSpikeBall; 8076; CHNSF0
Z_Chandelier; 17; CDLRA0
Z_Chandelier Unlit; 8063; CDLRD0
Z_FireBull; 8042; FBULA0
Z_FireBull Unlit; 8043; FBULH0
Z_FireSkull; 8060; FSKLA0
Z_GlitterBridge; 118; TLGLA0
Z_LargeFlame Permanent; 10503; FFLGG0
Z_LargeFlame Timed; 10502; FFLGG0
Z_SmallFlame Permanent; 10501; FFSMA0
Z_SmallFlame Timed; 10500; FFSMA0
Z_TeleportSmoke; 140; TSMKA0
Z_TwinedTorch; 116; TWTRA0
Z_TwinedTorch Unlit; 117; TWTRI0
Z_VasePillar; 103; VASEA0
Z_WallTorch; 54; WLTRA0
Z_WallTorch Unlit; 55; WLTRI0
Z_WingedStatue; 5; STTWA0
ZC_Rock1; 6; RCK1A0
ZC_Rock2; 7; RCK2A0
ZC_Rock3; 9; RCK3A0
ZC_Rock4; 15; RCK4A0
ZC_ShroomLarge; 41; MSH3A0
ZC_ShroomSmall1; 42; MSH4A0
ZC_ShroomSmall2; 44; MSH5A0
ZC_ShroomSmall3; 45; MSH6A0
ZC_StalactiteLarge; 52; SLC1A0
ZC_StalactiteMedium; 56; SLC2A0
ZC_StalactiteSmall; 57; SLC3A0
ZC_StalagmitePillar; 48; SGMPA0
ZC_StalagmiteLarge; 49; SGM1A0
ZC_StalagmiteMedium; 50; SGM2A0
ZC_StalagmiteSmall; 51; SGM3A0
ZF_ShroomBoom; 8104; SHRMA0
ZF_ShroomLarge1; 39; MSH1A0
ZF_ShroomLarge2; 40; MSH2A0
ZF_ShroomSmall1; 46; MSH7A0
ZF_ShroomSmall2; 47; MSH8A0
ZF_Hedge; 8068; XMASA0
ZF_DestructibleTree; 8062; TRDTA0
ZF_Shrub1; 8101; SHB1A0
ZF_Shrub2; 8102; SHB2A0
ZF_StumpBare; 29; STM2A0
ZF_StumpBurned; 28; STM1A0
ZF_TreeDead; 24; TRE1A0
ZF_TreeDestructible; 25; TRE1A0
ZF_TreeGnarled1; 80; TRE6A0
ZF_TreeGnarled2; 87; TRE7A0
ZF_TreeLarge1; 78; TRE4A0
ZF_TreeLarge2; 79; TRE5A0
ZG_BloodPool; 111; BDPLA0
ZG_CorpseHanging; 71; CPS4A0
ZG_CorpseKabob; 61; CPS1A0
ZG_CorpseLynched; 108; CPS4A0
ZG_CorpseNoHeart; 109; CPS5A0
ZG_CorpseSitting; 110; CPS6A0
ZG_CorpseSleeping; 62; CPS2A0
ZG_IronMaiden; 8067; IRONA0
ZG_TombstoneBigCross; 65; TMS3A0
ZG_TombstoneBrianP; 69; TMS7A0
ZG_TombstoneBrianR; 66; TMS4A0
ZG_TombstoneCrossCircle; 67; TMS5A0
ZG_TombstoneRIP; 63; TMS1A0
ZG_TombstoneShane; 64; TMS2A0
ZG_TombstoneSmallCross; 68; TMS6A0
ZI_IceSpikeLarge; 93; ICM1A0
ZI_IceSpikeMedium; 94; ICM2A0
ZI_IceSpikeSmall; 95; ICM3A0
ZI_IcicleLarge; 89; ICT1A0
ZI_IcicleMedium; 90; ICT2A0
ZI_IcicleSmall; 91; ICT3A0
ZM_CandleWeb; 8502; TST3A0
ZM_MeatCleaver; 8509; TST0A0
ZM_GobletSilver; 8508; TST9A0
ZM_GobletSmall; 8507; TST8A0
ZM_GobletSpill; 8505; TST6A0
ZM_GobletTall; 8506; TST7A0
ZM_LgCandle; 8504; TST5A0
ZM_LgStein; 8500; TST1A0
ZM_Pot1; 104; POT1A0
ZM_Pot2; 105; POT2A0
ZM_Pot3; 106; POT3A0
ZM_Rubble1; 100; RBL1A0
ZM_Rubble2; 101; RBL2A0
ZM_Rubble3; 102; RBL3A0
ZM_SmCandle; 8503; TST4A0
ZM_SmStein; 8501; TST2A0
ZP_GargBronzeShort; 8051; GAR9A0
ZP_GargBronzeTall; 8047; GAR5A0
ZP_GargCorrode; 8044; GAR1A0
ZP_GargIceShort; 76; STT5A0
ZP_GargIceTall; 73; STT3A0
ZP_GargLavaBrtShort; 8050; GAR7A0
ZP_GargLavaBrtTall; 8046; GAR3A0
ZP_GargLavaDrkShort; 8049; GAR6A0
ZP_GargLavaDrkTall; 8045; GAR2A0
ZP_GargPortalShort; 74; STT4A0
ZP_GargPortalTall; 72; STT2A0
ZP_GargStlShort; 8052; GAR9A0
ZP_GargStlTall; 8048; GAR5A0
ZS_Log; 88; LOGGA0
ZS_Moss1; 58; MSS1A0
ZS_Moss2; 59; MSS2A0
ZS_Stump1; 37; STM3A0
ZS_Stump2; 38; STM4A0
ZS_Tree1; 27; TRE3A0
ZS_Tree2; 26; TRE2A0
ZS_Vine; 60; SWMVA0
ZW_RockBlack; 99; RKBKA0
ZW_RockBrownLarge; 97; RKBLA0
ZW_RockBrownSmall; 98; RKBSA0

[Things: Puzzle Items]
Color; Yellow
ZZ_Book1; 9007; ABK1A0
ZZ_Book2; 9008; ABK2A0
ZZ_ClericWeapon; 9016; ACWPA0
ZZ_MageWeapon; 9017; AMWPA0
ZZ_FighterWeapon; 9015; AFWPA0
ZZ_Gear1; 9018; AGERA0
ZZ_Gear2; 9019; AGR2A0
ZZ_Gear3; 9020; AGR3A0
ZZ_Gear4; 9021; AGR4A0
ZZ_GemBlue1; 9006; AGMBA0
ZZ_GemBlue2; 9010; AGB2A0
ZZ_GemGreen1; 9005; AGMGA0
ZZ_GemGreen2; 9009; AGG2A0
ZZ_GemPedestal; 9012; GMPDB0
ZZ_GemRed; 9004; AGMRA0
ZZ_GemRedBig; 9003; ABGMA0
ZZ_Yorick's Skull; 9002; ASKUA0
ZZ_Skull2; 9014; ASK2A0
ZZ_Yorick w/o Skull; 9011; STWNA0

[Actions: Normal]
No Action; 0; ; ; ; ;

[Actions: Doors]
Door_Close; 10; tag; speed; ; ;
Door_Open; 11; tag; speed; ; ;
Door_Raise; 12; tag; speed; delay; ;
Door_LockedRaise; 13; tag; speed; delay; $key;

[Actions: Platforms (elevators.)]
Plat_PerpetualRaise; 60; tag; speed; delay; ;
Plat_Stop; 61; tag; ; ; ;
Plat_DownWaitUpStay; 62; tag; speed; delay; ;
Plat_DownByValue; 63; tag; speed; delay; height;
Plat_UpWaitDownStay; 64; tag; speed; delay; ;
Plat_UpByValue; 65; tag; speed; delay; height;

[Actions: Scripts]
ACS_Execute; 80; script; map; s_arg1; s_arg2; s_arg3
ACS_Suspend; 81; script; map; ; ;
ACS_Terminate; 82; script; map; ; ;
ACS_LockedExecute; 83; script; map; s_arg1; s_arg2; $key

[Actions: Floors]
Floor_LowerByValue; 20; tag; speed; height; ;
Floor_LowerToLowest; 21; tag; speed; ; ;
Floor_LowerToNearest; 22; tag; speed; ; ;
Floor_RaiseByValue; 23; tag; speed; height; ;
Floor_RaiseToHighest; 24; tag; speed; ; ;
Floor_RaiseToNearest; 25; tag; speed; ; ;
Floor_RaiseAndCrush; 28; tag; speed; damage; ;
Floor_RaiseByValueTimes8; 35; tag; speed; height; ;
Floor_LowerByValueTimes8; 36; tag; speed; height; ;
Floor_CrushStop; 46; tag; ; ; ;
Floor_LowerInstant; 66; tag; ; height; ;
Floor_RaiseInstant; 67; tag; ; height; ;
Floor_MoveToValueTimes8; 68; tag; speed; height; negative;

[Actions: Ceilings]
Ceiling_LowerByValue; 40; tag; speed; height; ;
Ceiling_RaiseByValue; 41; tag; speed; height; ;
Ceiling_CrushAndRaise; 42; tag; speed; damage; ;
Ceiling_LowerAndCrush; 43; tag; speed; damage; ;
Ceiling_CrushStop; 44; tag; ; ; ;
Ceiling_CrushRaiseAndStay; 45; tag; speed; damage; ;
Ceiling_MoveToValueTimes8; 69; tag; speed; height; negative;

[Actions: Floors & Ceilings]
FloorAndCeiling_LowerByValue; 95; tag; speed; height; ;
FloorAndCeiling_RaiseByValue; 96; tag; speed; height; ;

[Actions: PolyObjects]
Polyobj_StartLine; 1; PolyID; mirror; $sound; ;
Polyobj_RotateLeft; 2; PolyID; speed; $angle; ;
Polyobj_RotateRight; 3; PolyID; speed; $angle; ;
Polyobj_Move; 4; PolyID; speed; $angle; distance;
Polyobj_ExplicitLine; 5; PolyID; order; mirror; $sound;
Polyobj_MoveTimes8; 6; PolyID; speed; $angle; distance;
Polyobj_DoorSwing; 7; PolyID; speed; $angle; delay;
Polyobj_DoorSlide; 8; PolyID; speed; $angle; distance; delay
Polyobj_OR_RotateLeft; 90; PolyID; speed; $angle; ;
Polyobj_OR_RotateRight; 91; PolyID; speed; $angle; ;
Polyobj_OR_Move; 92; PolyID; speed; $angle; distance;
Polyobj_OR_MoveTimes8; 93; PolyID; speed; $angle; distance;

[Actions: Pillars & Stairs]
Pillar_Build; 29; tag; speed; height; ;
Pillar_Open; 30; tag; speed; f_height; c_height;
Pillar_BuildAndCrush; 94; tag; speed; height; damage;
Stairs_BuildDown; 26; tag; speed; height; delay; reset
Stairs_BuildUp; 27; tag; speed; height; delay; reset
Stairs_BuildDownSync; 31; tag; speed; height; reset;
Stairs_BuildUpSync; 32; tag; speed; height; reset;

[Actions: Lights]
Light_RaiseByValue; 110; tag; value; ; ;
Light_LowerByValue; 111; tag; value; ; ;
Light_ChangeToValue; 112; tag; value; ; ;
Light_Fade; 113; tag; value; tics; ;
Light_Glow; 114; tag; upper; lower; tics;
Light_Flicker; 115; tag; upper; lower; ;
Light_Strobe; 116; tag; upper; lower; u-tics; l-tics

[Actions: Miscellaneous]
Line_SetIdentification; 121; LineID; ; ; ;
Scroll_Texture_Left; 100; speed; ; ; ;
Scroll_Texture_Right; 101; speed; ; ; ;
Scroll_Texture_Up; 102; speed; ; ; ;
Scroll_Texture_Down; 103; speed; ; ; ;
UsePuzzleItem; 129; $pzlitem; script; s_arg1; s_arg2; s_arg3
Sector_ChangeSound; 140; tag; $sound; ; ;
Earthquake; 120; intensity; duration; damrad; tremrad; tid

[Actions: Teleporters]
Teleport_NewMap; 74; map; position; ; ;
Teleport_EndGame; 75; ; ; ; ;
Teleport; 70; tid; ; ; ;
Teleport_NoFog; 71; tid; ; ; ;

[Actions: Items]
ThrustItem; 72; $angle; distance; ; ;
DamageItem; 73; damage; ; ; ;
Item_Activate; 130; tid; ; ; ;
Item_Deactivate; 131; tid; ; ; ;
Item_Remove; 132; tid; ; ; ;
Item_Destroy; 133; tid; ; ; ;
Item_Projectile; 134; tid; $spwntype; $angle; speed; vspeed
Item_ProjectileGravity; 136; tid; $spwntype; $angle; speed; vspeed
Item_Spawn; 135; tid; $spwntype; $angle; ;
Item_SpawnNoFog; 137; tid; $spwntype; $angle; ;

[Sectors]
Normal; 0
Sky 2; 200
Light Phased; 1
Light Sequence Start; 2
Light Sequence Special 1; 3
Light Sequence Special 2; 4
Lightning Indoor Dim; 199
Lightning Indoor Bright; 198
Stairs Special 1; 26
Stairs Special 2; 27
Scroll North Slow; 201
Scroll North Medium; 202
Scroll North Fast; 203
Scroll East Slow; 204
Scroll East Medium; 205
Scroll East Fast; 206
Scroll South Slow; 207
Scroll South Medium; 208
Scroll South Fast; 209
Scroll West Slow; 210
Scroll West Medium; 211
Scroll West Fast; 212
Scroll Northwest Slow; 213
Scroll Northwest Medium; 214
Scroll Northwest Fast; 215
Scroll Northeast Slow; 216
Scroll Northeast Medium; 217
Scroll Northeast Fast; 218
Scroll Southwest Slow; 222
Scroll Southwest Medium; 223
Scroll Southwest Fast; 224
Scroll Southeast Slow; 219
Scroll Southeast Medium; 220
Scroll Southeast Fast; 221

[Angles]
North; 90
South; 270
East; 0
West; 180
Northeast; 45
Northwest; 135
Southeast; 315
Southwest; 225

[Popup: Sound]
Heavy; 1
Metal; 2
Creak; 3
Silence; 4
Lava; 5
Water; 6
Ice; 7
Earth; 8
Metal 2; 9

[Popup: Key]
Steel key; 1
Cave key; 2
Axe key; 3
Fire key; 4
Emerald key; 5
Dungeon key; 6
Silver key; 7
Rusty key; 8
Horn key; 9
Swamp key; 10
Castle key; 11

[Popup: Pzlitem]
ZZ_Skull; 0
ZZ_BigGem; 1
ZZ_GemRed; 2
ZZ_GemGreen1; 3
ZZ_GemGreen2; 4
ZZ_GemBlue1; 5
ZZ_GemBlue2; 6
ZZ_Book1; 7
ZZ_Book2; 8
ZZ_Skull2; 9
ZZ_FighterWeapon; 10
ZZ_ClericWeapon; 11
ZZ_MageWeapon; 12
ZZ_Gear; 13
ZZ_Gear2; 14
ZZ_Gear3; 15
ZZ_Gear4; 16

[Popup: Spwntype]
None; 0
A_Banishment; 17
A_Boots of Speed; 13
A_Bracers; 22
A_Chaos Device; 18
A_Dark Servant; 16
A_Disc of Repulsion; 74
A_Flechette; 72
A_Krater of Might; 26
A_Porkelator; 14
A_Torch; 73
A_Wings of Wrath; 15
Am_Blue Mana; 11
Am_Combined Mana; 75
Am_Green Mana; 12
Ar_Amulet of Warding; 71
Ar_Falcon Shield; 69
Ar_Mesh Armor; 68
Ar_Platinum Helm; 70
E_Afrit; 5
E_Centaur; 1
E_Dark Bishop; 19
E_Ettin; 4
E_Green Chaos Serpent; 3
E_Reiver Buried; 9
E_Reiver; 8
E_Slaughtaur; 2
E_Stalker Shooter; 7
E_Stalker; 6
E_Wendigo; 20
H_Healing Complete (Urn); 25
H_Healing Hefty (Flask); 24
H_Healing Wimpy (Vial); 23
P_Arrow; 50
P_Blade; 64
P_Dart; 51
P_Fireball; 10
P_Ice Shard; 65
P_Poison Dart; 52
P_Porkelator Bullet; 40
P_Ripper Ball; 53
Stained Glass-10; 63
Stained Glass-1; 54
Stained Glass-2; 55
Stained Glass-3; 56
Stained Glass-4; 57
Stained Glass-5; 58
Stained Glass-6; 59
Stained Glass-7; 60
Stained Glass-8; 61
Stained Glass-9; 62
W_BloodScourge-1 (M4); 37
W_BloodScourge-2 (M4); 38
W_BloodScourge-3 (M4); 39
W_Frost Shards (M2) ; 36
W_Hammer of Retribution (F3); 28
W_Quietus-1 (F4); 29
W_Quietus-2 (F4); 30
W_Quietus-3 (F4); 31
W_Serpent Staff (C2); 32
W_Timon's Axe (F2); 27
W_WraithVerge-1 (C4); 33
W_WraithVerge-2 (C4); 34
W_WraithVerge-3 (C4); 35
Puzzle-Skull; 76
Puzzle-Big Red Gem; 77
Puzzle-Red Gem; 78
Puzzle-Green Gem 1; 79
Puzzle-Green Gem 2; 80
Puzzle-Blue Gem 1; 81
Puzzle-Blue Gem 2; 82
Puzzle-Book 1; 83
Puzzle-Book 2; 84
Rock-1; 41
Rock-2; 42
Rock-3; 43
Dirt-1; 44
Dirt-2; 45
Dirt-3; 46
Dirt-4; 47
Dirt-5; 48
Dirt-6; 49
Small Flame; 66
Large Flame; 67
K_Steel key; 85
K_Cave key; 86
K_Axe key; 87
K_Fire key; 88
K_Emerald key; 89
K_Dungeon key; 90
K_Silver key; 91
K_Rusty key; 92
K_Horn key; 93
K_Swamp key; 94
Water Dripping; 95
Temporary Small Flame; 96
Permanent Small Flame; 97
Temporary Large Flame; 98
Permanent Large Flame; 99
Green Serpent Guts; 100
Brown Serpent Guts; 101
Ettin Guts; 102
Centaur Guts; 103
Thrust Spike Up; 104
Thrust Spike Down; 105
Flesh Drip 1; 106
Flesh Drip 2; 107
Spark Drip; 108
Glitter Bridge; 21

[Popup: Angle]
North; 64
South; 192
East; 0
West; 128
Northeast; 32
Northwest; 96
Southeast; 224
Southwest; 160

[Actions with TID]
70
71
130
131
132
133
134
135
136
137

[Thing Args]
1; startspot; ; ; ;
2; startspot; ; ; ;
3; startspot; ; ; ;
4; startspot; ; ; ;
10225; spwnfreq; spread; ; duration; turn
10000; fogspeed; spread; spwnfreq; foglife;
10001; fogspeed; ; ; foglife; moving
10002; fogspeed; ; ; foglife; moving
10003; fogspeed; ; ; foglife; moving
104; $spwntype; ; ; ;
105; $spwntype; ; ; ;
106; $spwntype; ; ; ;

[Thing Info]
# Thing information: Type; Radius; Height; AngleClash
# AngleClash: 0 = No angle/clash, 1 = angle, 2 = clash, 3 = angle/clash
# Nonspecified things have radius of 20 and height of 16, and don't draw
# angles or clash.

# enemies
10030; 25; 68; 3
10011; 20; 68; 3
10060; 20; 68; 3
10080; 40; 110; 3
10200; 65; 115; 3
114; 22; 65; 3
115; 20; 64; 3
107; 20; 64; 3
254; 20; 65; 3
34; 20; 55; 3
8020; 22; 75; 3
8080; 32; 64; 3
121; 32; 70; 3
120; 32; 70; 3
31; 32; 64; 3
10100; 16; 64; 3
10101; 16; 64; 3
10102; 16; 64; 3

# players, dm, teleport
1; 20; 56; 3
2; 20; 56; 3
3; 20; 56; 3
4; 20; 56; 3
9100; 20; 56; 3
9101; 20; 56; 3
9102; 20; 56; 3
9103; 20; 56; 3
14; 20; 56; 3
11; 20; 56; 3

# other things
10090; 20; 128; 0
10091; 20; 128; 2
103; 12; 54; 2
104; 10; 32; 2
105; 10; 25; 2
106; 15; 25; 2
108; 11; 95; 2
109; 10; 100; 2
110; 15; 35; 0
116; 10; 64; 2
117; 10; 64; 2
118; 32; 2; 2
122; 8; 8; 0
124; 8; 8; 0
17; 20; 60; 0
24; 10; 96; 2
25; 15; 128; 2
26; 10; 150; 2
27; 10; 120; 2
28; 12; 20; 2
29; 12; 20; 2
48; 8; 138; 2
49; 8; 48; 2
5; 10; 62; 2
50; 6; 40; 2
51; 8; 36; 2
52; 8; 66; 0
56; 6; 50; 0
57; 8; 40; 0
58; 20; 20; 0
59; 20; 24; 0
60; 8; 52; 0
61; 10; 92; 2
63; 10; 46; 2
64; 10; 46; 2
65; 10; 46; 2
66; 10; 52; 2
67; 10; 52; 2
68; 8; 46; 2
69; 8; 46; 2
71; 6; 75; 0
72; 14; 108; 2
73; 14; 108; 2
74; 14; 62; 2
76; 14; 62; 2
77; 8; 120; 2
78; 15; 180; 2
79; 15; 180; 2
80; 22; 100; 2
8004; 8; 8; 0
8030; 8; 20; 0
8031; 8; 20; 0
8032; 8; 20; 0
8033; 8; 20; 0
8034; 8; 20; 0
8035; 8; 20; 0
8036; 8; 20; 0
8037; 8; 20; 0
8038; 8; 20; 0
8039; 8; 20; 0
8042; 20; 80; 2
8043; 20; 80; 2
8044; 14; 108; 2
8045; 14; 108; 2
8046; 14; 108; 2
8047; 14; 108; 2
8048; 14; 108; 2
8049; 14; 62; 2
8050; 14; 62; 2
8051; 14; 62; 2
8052; 14; 62; 2
8060; 5; 10; 0
8061; 6; 35; 2
8062; 15; 180; 2
8063; 20; 60; 0
8064; 16; 72; 2
8065; 56; 120; 2
8067; 12; 60; 2
8068; 11; 130; 2
8069; 12; 26; 2
8070; 12; 26; 2
8071; 4; 32; 0
8072; 4; 64; 0
8073; 4; 32; 0
8074; 4; 32; 0
8075; 4; 32; 0
8076; 4; 32; 0
8077; 4; 32; 0
8100; 15; 32; 2
8101; 8; 24; 2
8102; 16; 40; 2
8103; 8; 72; 2
8104; 6; 20; 2
8200; 8; 20; 0
87; 22; 100; 2
88; 20; 25; 2
89; 8; 66; 0
90; 5; 50; 0
9011; 10; 62; 0
9012; 10; 40; 0
91; 4; 32; 0
92; 4; 8; 0
93; 8; 66; 2
94; 5; 50; 2
95; 4; 32; 2
97; 17; 72; 0
98; 15; 50; 0
99; 20; 40; 0
99; 20; 40; 0
9001; 10; 10; 0
9013; 10; 10; 2

[Actions with Tag]
# a list of all the actions whose arg0 parameter is a sector tag
10
11
12
13
60
61
62
63
64
65
20
21
22
23
24
25
28
35
36
46
66
67
68
40
41
42
43
44
45
69
95
96
29
30
94
26
27
31
32
110
111
112
113
114
115
116
140
