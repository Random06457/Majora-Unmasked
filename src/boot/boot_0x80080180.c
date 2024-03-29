#include <ultra64.h>
#include <global.h>

void func_80080180(UNK_TYPE* a0, UNK_TYPE a1, UNK_TYPE a2) {
    while (a1 -= sizeof(u32)) {
        *a0++ = a2;
    }
}

UNK_RET func_8008019C(void) {
    func_80080180(&D_80000500, 0x25800, 0x00010001);
    func_800805E0(0);
    func_8008FE60(&D_80000500);
    func_8008AE70(0);
}

UNK_RET func_800801EC(void) {
    UNK_TYPE sp1C;
    UNK_PTR sp18 = (UNK_PTR)(0x80000000 + osMemSize);
    func_80080150((void*)0x80000400, &D_80000500);
    func_80080150(&D_80025D00, (int*)func_80080060);
    func_80080150(&D_80208EA0, sp18);
}

UNK_RET func_80080250(void) {
    UNK_TYPE* v0;
    UNK_PTR v1;
    UNK_PTR t7;
    UNK_TYPE sp58;
    UNK_TYPE sp54;
    UNK_TYPE sp50;
    UNK_TYPE sp4C;
    UNK_TYPE sp48;
    OSMesgQueue sp30;
    OSMesg sp2C;
    UNK_TYPE sp28;

    osCreateMesgQueue(&sp30, &sp2C, 1);

    v1 = &code_vrom_start;
    t7 = &code_vrom_end;
    v0 = &D_80096B50;
    sp28 = *v0;
    *v0 = 0;

    func_80080C04((DmaRequest*)&sp48, (UNK_FUN_ARG)func_800A5AC0, v1, (u32)t7 - (u32)v1, 0, &sp30, 0);
    func_8008019C();
    func_800801EC();
    osRecvMesg(&sp30, 0, 1);

    D_80096B50 = sp28;

    func_80080150(&D_801E3FA0, &D_80208EA0);
}

UNK_RET func_80080300(UNK_TYPE a0) {
    func_80085320(&D_8009A670, &D_8009A170, &D_8009A670_, 0, 256, &D_800981A0);
    func_80081754(&D_80099EF0, &D_8009A670, 18, 1);
    func_80080D0C();
    func_80080250();
    func_80174BF0(a0);
    func_80080E00();
}

void func_8008038C(void) {
    u32 i;
    osCreateViManager(254);

    D_80096B2C = 66;
    D_80096B30 = 1.0;
    D_80096B34 = 1.0;

    switch (osTvType) {
        case 1:
        D_8009B290 = 2;
        for (i = 0; i < 20; i++) {
            D_800980E0[i] = D_8009B240[i];
        }
        D_80096B34 = D_800981B0;
        break;
        case 2:
        D_8009B290 = 30;
        for (i = 0; i < 20; i++) {
            D_80098010[i] = D_8009B240[i];
        }
        D_80096B34 = D_800981B0;
        break;
        case 0:
        D_8009B290 = 44;
        for (i = 0; i < 20; i++) {
            D_80097FC0[i] = D_8009B240[i];
        }
        break;
    }

    D_80096B20 = 1;
}

void func_80080514(UNK_TYPE a0) {
    func_8008038C();
    osCreatePiManager(150, (OSMesgQueue*)&D_8009B228, (OSMesg*)&D_8009B160, 50);
    func_80085320(&D_8009B140, &D_8009A840, &D_8009B140_, 0, 1024, &D_800981A8);
    osCreateThread((OSThread*)&D_8009A690, 3, (void (*)(void*))func_80080300, (void*)a0, (void*)&D_8009B140, 12);
    osStartThread((OSThread*)&D_8009A690);
    osSetThreadPri(NULL, 0);

    while(1);
}
