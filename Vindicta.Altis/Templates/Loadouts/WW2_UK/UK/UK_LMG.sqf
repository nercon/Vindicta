removeAllWeapons this;
removeAllItems this;
removeAllAssignedItems this;
removeUniform this;
removeVest this;
removeBackpack this;
removeHeadgear this;
removeGoggles this;

_RandomHeadgear = selectRandom ["H_LIB_UK_Helmet_Mk2", "H_LIB_UK_Helmet_Mk2_Bowed"];
this addHeadgear _RandomHeadgear;
this forceAddUniform "U_LIB_UK_P37_LanceCorporal";
this addVest "V_LIB_UK_P37_Heavy";
_RandomBackpack = selectRandom ["B_LIB_UK_HSack_Cape", "B_LIB_UK_HSack", "B_LIB_UK_HSack_Tea"];
this addBackpack _RandomBackpack;

this addWeapon "fow_w_bren";
this addPrimaryWeaponItem "LIB_30Rnd_770x56";


this addItemToUniform "FirstAidKit";
for "_i" from 1 to 4 do {this addItemToVest "LIB_30Rnd_770x56";};
for "_i" from 1 to 2 do {this addItemToVest "LIB_MillsBomb";};

this linkItem "ItemMap";
this linkItem "ItemCompass";
this linkItem "ItemWatch";
