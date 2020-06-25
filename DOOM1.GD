# *****************************************************************************
# DCK game definition file.
# *****************************************************************************
# Don't change the next line.
# I mean that.
# Don't change it.
GAMEDATA
# You had better not have changed it.

[General]
Name; DOOM 1
# Make sure the following line is correct
Directory; D:\DOOM
IWAD; DOOM.WAD
# It's not a good idea to change the next line
Tag; E'1M'1
New Map; E1M1
Run; doom.exe -warp $num1 $num2
Palette; DCK.PAL
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
Player 1 Start; 1; WINUM1
Player 2 Start; 2; WINUM2
Player 3 Start; 3; WINUM3
Player 4 Start; 4; WINUM4
Color; LightGreen
Deathmatch Start; 11; BFE2B0
Teleport exit; 14; BFE2B0

[Things: Enemies]
Color; Red
Former Human Trooper; 3004; POSSA1
Former Human Sergeant; 9; SPOSA1
Imp; 3001; TROOB1
Demon; 3002; SARGA1
Spectre (invisible Demon); 58;
Lost Soul; 3006; SKULA1
Color; LightRed
Baron of Hell; 3003; BOSSE1
Cacodemon; 3005; HEADA1
Spider Boss; 7; SPIDH1
Cyberdemon; 16; CYBRF1

[Things: Keys]
Color; Magenta
Blue Key Card; 5; BKEYA0
Yellow Key Card; 6; YKEYA0
Red Key Card; 13; RKEYA0
Blue Skull Key; 40; BSKUA0
Yellow Skull Key; 39; YSKUA0
Red Skull Key; 38; RSKUA0

[Things: Health and Armor]
Color; White
Medkit (+25); 2012; MEDIA0
Stimpak (+10); 2011; STIMA0
Security Armor (100); 2018; ARM1A0
Combat Armor (200); 2019; ARM2A0
Radiation Suit; 2025; SUITA0

[Things: Items]
Color; LightMagenta
Spiritual Armor (+1); 2015; BON2C0
Health Potion (+1); 2014; BON1A0
Soul Sphere; 2013; SOULA0
Computer Map; 2026; PMAPA0
Blur Artifact; 2024; PINSA0
Berserk Pack; 2023; PSTRA0
Invulnerable Artifact; 2022; PINVA0
Lite-Amp Goggles; 2045; PVISA0

[Things: Weapons]
Color; Blue
Shotgun; 2001; SHOTA0
Chainsaw; 2005; CSAWA0
Chaingun; 2002; MGUNA0
Color; LightBlue
Rocket Launcher; 2003; LAUNA0
Plasma Gun; 2004; PLASA0
BFG9000; 2006; BFUGA0

[Things: Ammo]
Color; Grey
Bullet Clip; 2007; CLIPA0
Box of Bullets; 2048; AMMOA0
4 Shells; 2008; SHELA0
Box of Shells; 2049; SBOXA0
1 Rocket; 2010; ROCKA0
Box of Rockets; 2046; BROKA0
Energy Cell (20); 2047; CELLA0
Energy Pack (100); 17; CELPA0
Backpack; 8; BPAKA0

[Things: Pillars, Barrels, Lights]
Color; Yellow
Barrel; 2035; BEXPA0
Color; Brown
Technical Column; 48; ELECA0
Tall Green Pillar; 30; COL1A0
Tall Red Pillar; 32; COL3A0
Short Green Pillar; 31; COL2A0
Short Red Pillar; 33; COL4A0
Pillar w/Heart; 36; COL5A0
Red Pillar w/Skull; 37; COL6A0
Beacon Lamp; 2028; COLUA0
Candle; 34; CANDA0
Candelabra; 35; CBRAA0
Tall Blue Torch; 44; TBLUA0
Tall Green Torch; 45; TGRNA0
Tall Red Torch; 46; TREDA0
Short Blue Torch; 55; SMBTA0
Short Green Torch; 56; SMGTA0
Short Red Torch; 57; SMRTA0

[Things: Trees, Shrubs]
Color; Brown
Grey Tree; 43; TRE1A0
Brown Stub; 47; SMITA0
Tall Brown Tree; 54; TRE2A0

[Things: Dead Dudes and Roadkill]
Color; Brown
Dead Player (Green); 15; PLAYN0
Dead Trooper; 18; POSSL0
Dead Sergeant; 19; SPOSL0
Dead Imp; 20; TROOM0
Dead Demon; 21; SARGN0
Dead Cacodemon; 22; HEADL0
Guts and Bones; 12; PLAYW0
Guts and Bones; 10; PLAYW0
Pool of Blood; 24; POL5A0

[Things: Impaled Dudes, Hell Decor]
Color; Brown
Pole with Skull; 27; POL4A0
Skewer with Heads; 28; POL2A0
Pile of Skulls; 29; POL3A0
Impaled body; 25; POL1A0
Twitching Impaled Body; 26; POL6A0
Gold Flaming Skulls; 42; FSKUA0
Eye-in-green-thing; 41; CEYEA0

[Things: Hangin' Dudes]
Color; Brown
Swaying Body; 49; GOR1A0
Hanging, Arms Out; 50; GOR2A0
One-legged Body; 51; GOR3A0
Hanging Torso; 52; GOR4A0
Hanging Leg; 53; GOR5A0
Hanging, Arms Out; 59; GOR2A0
One-legged Body; 61; GOR3A0
Hanging Torso; 60; GOR4A0
Hanging Leg; 62; GOR5A0
Swaying Body; 63; GOR1A0

# the following line includes the contents of the file DOOM17.DAT
# in this file.. it contains the line types, sector types, "line
# that don't require tags, etc.
[Include DOOM19.DAT]
