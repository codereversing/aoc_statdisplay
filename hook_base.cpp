#pragma comment(lib, "detours.lib")

#include <Windows.h>
#include <stdio.h>
#include "detours.h"
#include "item_set.h"

#ifndef __cplusplus
extern "C" {
#endif

typedef void (__stdcall *presources_changed)(short int usage_type, float queued, int unused);
int detour(void **function_to_detour, void *replacement, bool attach);

presources_changed resources_changed = (presources_changed)(0x00555440);
item_set *base_pointers = NULL;
item_set *temp_pointer = NULL;

__declspec(naked) void resources_changed_hook(short int res_type, float usage_type, int unused) {
	__asm {
		pushad
		mov eax, temp_pointer
		mov dword ptr[eax], ecx         //temp_pointer->base_pointer points to calling class
    }
    temp_pointer->player_name = (char*)(*(temp_pointer->base_pointer + (0x98 / sizeof(DWORD_PTR))));
    temp_pointer->player_stat = (player_stats*)(*(temp_pointer->base_pointer + (0xA8 / sizeof(DWORD_PTR))));
    if(insert(&base_pointers, temp_pointer) == true)
        temp_pointer = (item_set*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(item_set));
    __asm {
		popad
		jmp resources_changed
	}
}

LRESULT __declspec(dllexport) CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam) {
    if(code < 0 || GetAsyncKeyState(wParam) == -32767)
		return CallNextHookEx(NULL, code, wParam, lParam);
    if(code == HC_ACTION) {
        if(wParam == VK_F5)
            detour(&(PVOID&)resources_changed, resources_changed_hook, true);
        else if(wParam == VK_F6)
            detour(&(PVOID&)resources_changed, resources_changed_hook, false);
        else if(wParam == VK_F7)
            print(&base_pointers);
        else if(wParam == VK_F8)
            clear(&base_pointers);
    }
    return CallNextHookEx(NULL, code, wParam, lParam);
}

int detour(void **function_to_detour, void *replacement, bool attach) {
    int ret = NO_ERROR;
    if((ret = DetourTransactionBegin()) != NO_ERROR)
        return ret;
    if((ret = DetourUpdateThread(GetCurrentThread())) != NO_ERROR) 
        return ret;
    attach == true ? (ret = DetourAttach(function_to_detour, replacement)) :
        (ret = DetourDetach(function_to_detour, replacement));
    if(ret != NO_ERROR)
        return ret;
    return DetourTransactionCommit();
}

int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID reserved) {
	if(fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hModule);
        if(AllocConsole()) {
            freopen("CONOUT$", "w", stdout);
            SetConsoleTitle(L"Statistics spy");
            SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
            temp_pointer = (item_set*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(item_set));
        }
	}
	return TRUE;
}

#ifndef __cplusplus
}
#endif