#include <Windows.h> 
#include <process.h> 
#include <stdio.h> 
#include <iostream> 
 
 
 
// BY PTKFIR 
 
#define CharacterName   ( (char*)( *pdwUserData + 0x0000 ) )    
#define BodyModel       (( char*) ( *pdwUserData + 0x0020 ) )    
#define HeadModel       ( (char*)( *pdwUserData + 0x0060 ) )     
/////    
// 0x00A0 (160) --> 0x00B8 (184) = 0x0018 (24) Bytes    
/////    
#define CharacterLevel  ( *(int*)( *pdwUserData + 0x00B8 ) )    
#define Strength        ( *(int*)( *pdwUserData + 0x00BC ) )    
#define Spirit          ( *(int*)( *pdwUserData + 0x00C0 ) )    
#define Talent          ( *(int*)( *pdwUserData + 0x00C4 ) )    
#define Agility         ( *(int*)( *pdwUserData + 0x00C8 ) )    
#define Health_         ( *(int*)( *pdwUserData + 0x00CC ) )    
#define Accuracy        ( *(int*)( *pdwUserData + 0x00D0 ) )    
#define AttackRating    ( *(int*)( *pdwUserData + 0x00D4 ) )    
#define MinDamage       ( *(int*)( *pdwUserData + 0x00D8 ) )    
#define MaxDamage       ( *(int*)( *pdwUserData + 0x00DC ) )    
#define AttackSpeed     ( *(int*)( *pdwUserData + 0x00E0 ) )    
#define ShootingRange   ( *(int*)( *pdwUserData + 0x00E4 ) )    
#define Critical        ( *(int*)( *pdwUserData + 0x00E8 ) )    
#define Defence         ( *(int*)( *pdwUserData + 0x00EC ) )    
#define Block           ( *(int*)( *pdwUserData + 0x00F0 ) )    
#define Absorbtion      ( *(float*)( *pdwUserData + 0x00F4 ) )    
#define MoveSpeed       ( *(float*)( *pdwUserData + 0x00F8 ) )    
#define Sight           ( *(int*)( *pdwUserData + 0x00FC ) )    
#define CurrentWeight   ( *(short*)( *pdwUserData + 0x0100 ) )    
#define TotalWeight     ( *(short*)( *pdwUserData + 0x0102 ) )    
#define ElementalDef    ( (short*)( *pdwUserData + 0x0104 ) ) // short[8]    
#define ElementalAtk    ( (short*)( *pdwUserData + 0x0114 ) ) // short[8]    
#define XorHP_1         ( *(short*)( *pdwUserData + 0x0124 ) )    
#define XorHP_2         ( *pXorHP )     
#define CurrentHP       ( XorHP_1 ^ XorHP_2 )    
#define TotalHP         ( *(short*)( *pdwUserData + 0x0126 ) )    
#define CurrentMP       ( *(short*)( *pdwUserData + 0x0128 ) )    
#define TotalMP         ( *(short*)( *pdwUserData + 0x012A ) )     
#define CurrentSTM      ( *(short*)( *pdwUserData + 0x012C ) )    
#define TotalSTM        ( *(short*)( *pdwUserData + 0x012E ) )    
#define HPRegen         ( *(float*)( *pdwUserData + 0x0130 ) )    
#define MPRegen         ( *(float*)( *pdwUserData + 0x0134 ) )    
#define STMRegen        ( *(float*)( *pdwUserData + 0x0138 ) )    
#define STMRegen2       ( *(DWORD*)( 0x30CC7B84 ) )    
#define XorExp_1        ( *(DWORD*)( *pdwUserData + 0x013C ) )    
#define XorExp_2        ( *pXorExp )    
#define CurrentExp      ( XorExp_1 ^ XorExp_2 )    
#define NextExp         ( *(DWORD*)( *pdwUserData + 0x0140 ) )    
#define CurrentGold     ( *(int*)( *pdwUserData + 0x0144 ) )    
#define Target          ( *(DWORD*)( 0x3340F78 ) )    
#define TalentAdd       1000000    
/////////Memory patching///////    
PBYTE pbPatchXtrap = (PBYTE)0x0055488E;//JMP SHORT    
///////////////////////////////    
//Code Caves//    
DWORD STMA = 0;//return address    
/////    
// 0x0148 (328) --> 0x0174 (372) = 0x002C (44) Bytes    
/////    
#define CharacterRank   ( *(int*)( *pdwUserData + 0x0174 ) )    
bool SetVars = true;    
/////////////////////////////////////////////////    
// Typedefs for the client functions used:    
typedef void (*t_ChatPrint)     ( char * );    
typedef void (*t_MsgPrint)      ( char * );    
typedef void (*t_ChatPrint)     ( char * );    
typedef int  (*t_ExpUp)         ( long );    
typedef void (*t_Restore)       ( DWORD );    
typedef void (*t_ValidChar)     ( );    
typedef int  (*t_Teleport)      ( int );    
typedef int  (*t_HairStyle)     ( int );    
typedef int  (*t_UseSkill)      ( char *,DWORD );    
/////////////////////////////////////////////////    
    
/////////////////////////////////////////////////    
// These addresses change with each version of PT    
// so they need to be found using byte signatures    
PDWORD  pdwUserData         = *(PDWORD*)0x004B9C01;    
PBYTE   pbSkillData         = *(PBYTE*)0x004A366A;    
    
long  * pXorExp             = *(long **)0x0043DA0B;     
short * pXorHP              = *(short**)0x004B60AE;    
short * pCalcHP             = *(short**)0x00401378;    
    
t_MsgPrint  MsgPrint        = (t_MsgPrint)0x0048E5D0;    
t_ChatPrint ChatPrint       = (t_ChatPrint)0x00551CE0;    
//t_ExpUp     ExpUp           = (t_ExpUp)0x00443E10;    
//t_UseSkill  UseSkill        = (t_UseSkill)0x0051CA80;    
t_ValidChar ValidateChar    = NULL;     
//t_Teleport  Teleport        = (t_Teleport)0x00430740;    
//t_HairStyle ChangeHairstyle = (t_HairStyle)0x004437C0;    
DWORD       ValidateFns[8]  = { NULL };    
/////////////////////////////////////////////////    
 
 
// ChatPrintf - Prints formatted strings to the    
//              chat screen    
 
void ChatPrintf( const char *format, ... )    
{    
    va_list ap;    
    char    szBuffer[512] = "";    
    
    va_start( ap, format );    
    _vsnprintf_s( szBuffer, 512, 511, format, ap );    
    va_end( ap );    
    
    ChatPrint( szBuffer );    
}    
 
 
void Validate()    
{    
 
           DWORD dwAddress = 0x0055D0E5;    
ValidateFns[0] = (DWORD)( *(long *)( dwAddress + 28 ) + (long)( dwAddress + 32 ) );    
ValidateFns[1] = (DWORD)( *(long *)( dwAddress + 33 ) + (long)( dwAddress + 37 ) );    
ValidateFns[2] = (DWORD)( *(long *)( dwAddress + 38 ) + (long)( dwAddress + 42 ) );    
ValidateFns[3] = (DWORD)( *(long *)( dwAddress + 43 ) + (long)( dwAddress + 47 ) );    
//ValidateFns[4] = (DWORD)( *(long *)( dwAddress + 48 ) + (long)( dwAddress + 52 ) );    
//ValidateFns[5] = (DWORD)( *(long *)( dwAddress + 55 ) + (long)( dwAddress + 59 ) );    
//ValidateFns[6] = (DWORD)( *(long *)( dwAddress + 63 ) + (long)( dwAddress + 67 ) );    
//ValidateFns[7] = (DWORD)( *(long *)( dwAddress + 68 ) + (long)( dwAddress + 72 ) );    
 
 
    ( ( void (*)( ) )       ValidateFns[0] )(   );    
    ( ( void (*)( ) )       ValidateFns[1] )(   );    
    ( ( void (*)( ) )       ValidateFns[2] )(   );    
    ( ( void (*)( ) )       ValidateFns[3] )(   );    
    ( ( void (*)( ) )       ValidateFns[4] )(   );//Bad!    
 //   ( ( void (*)( int ) )   ValidateFns[5] )( 4 );    
//    ( ( void (*)( ) )       ValidateFns[6] )(   );    
 //   ( ( void (*)( ) )       ValidateFns[7] )(   );    
}    
 
 
bool var1= false; 
bool var2= false; 
bool var3= false; 
bool var4= false; 
bool var5= false; 
 
void MainST(void*) 
{ 
	while(250) 
	{ 
 
		if(GetAsyncKeyState(VK_SHIFT)) 
		{ 
			if(GetAsyncKeyState(VK_F1) & 1) 
			{ 
				if (var1 == false) { 
			var1=true; 
	 
				} 
			else 
			{ 
			var1=false; 
			} 
			} 
 
			if(GetAsyncKeyState(VK_F2) & 1) 
			{ 
			if (var2 == false) { 
			var2=true; 
	//		ChatPrintf("Range Ativ"); 
			} 
			else 
			{ 
			var2=false; 
			} 
			} 
 
			if(GetAsyncKeyState(VK_F3) & 1) 
			{ 
			if (var3 == false) { 
			var3=true; 
			} 
			else 
			{ 
			var3=false; 
			} 
			} 
 
			if(GetAsyncKeyState(VK_F4) & 1) 
			{ 
			if (var4 == false) { 
			var4=true; 
			} 
			else 
			{ 
			var4=false; 
			} 
			} 
 
			if(GetAsyncKeyState(VK_F5) & 1) 
			{ 
			if (var5 == false) { 
			var5=true; 
			} 
			else 
			{ 
			var5=false; 
			} 
			} 
 
			if (var1 == true) 
			{ 
AttackSpeed = 8; 
Validate(); 
			} 
 
			if (var2=true) 
			{ 
ShootingRange = 999; 
Validate(); 
//*(PDWORD)(*(PDWORD)(pdwUserData) + 0x13C) = 0x999999; 
 
			} 
 
			if (var3=true) 
			{ 
//.... 
			} 
 
			if (var4=true) 
			{ 
//.... 
			} 
 
			if (var5=true) 
			{ 
//.... 
			} 
		} 
	} 
 
} 
 
 
 
 
int WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) 
{ 
	switch(dwReason) 
	{ 
		case DLL_PROCESS_ATTACH: //Init Code here 
_beginthread( MainST, 0, NULL); 
		break; 
		case DLL_THREAD_ATTACH: //Thread-specific init code here 
 
		break; 
		case DLL_THREAD_DETACH: //Thread-specific cleanup code here 
 
		break; 
		case DLL_PROCESS_DETACH: //Cleanup code here 
			//End HackThread 
			//hThread->Terminate(); 
 
			//End KeyCatcherThread 
			 
		break; 
	} 
	return true; 
}