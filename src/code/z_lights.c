#include <ultra64.h>
#include <global.h>

#pragma intrinsic (sqrtf)
extern float fabsf(float);
#pragma intrinsic (fabsf)

void Lights_InitPositionalLight(z_LightInfoPositional* info, s16 posX, s16 posY, s16 posZ, u8 red, u8 green, u8 blue, s16 radius, u32 type) {
    info->type = type;
    info->params.posX = posX;
    info->params.posY = posY;
    info->params.posZ = posZ;
    Lights_SetPositionalLightColorAndRadius(info, red, green, blue, radius);
}

void Lights_InitType0PositionalLight(z_LightInfoPositional* info, s16 posX, s16 posY, s16 posZ, u8 red, u8 green, u8 blue, s16 radius) {
    Lights_InitPositionalLight(info, posX, posY, posZ, red, green, blue, radius, 0);
}

void Lights_InitType2PositionalLight(z_LightInfoPositional* info, s16 posX, s16 posY, s16 posZ, u8 red, u8 green, u8 blue, s16 radius) {
    Lights_InitPositionalLight(info, posX, posY, posZ, red, green, blue, radius, 2);
}

void Lights_SetPositionalLightColorAndRadius(z_LightInfoPositional* info, u8 red, u8 green, u8 blue, s16 radius) {
    info->params.red = red;
    info->params.green = green;
    info->params.blue = blue;
    info->params.radius = radius;
}

void Lights_SetPositionalLightPosition(z_LightInfoPositional* info, s16 posX, s16 posY, s16 posZ) {
    info->params.posX = posX;
    info->params.posY = posY;
    info->params.posZ = posZ;
}

void Lights_InitDirectional(z_LightInfoDirectional* info, s8 dirX, s8 dirY, s8 dirZ, u8 red, u8 green, u8 blue) {
    info->type = 1;
    info->params.dirX = dirX;
    info->params.dirY = dirY;
    info->params.dirZ = dirZ;
    info->params.red = red;
    info->params.green = green;
    info->params.blue = blue;
}

void Lights_MapperInit(z_LightMapper* mapper, u8 red, u8 green, u8 blue) {
    mapper->ambient.l.colc[0] = red;
    mapper->ambient.l.col[0] = red;
    mapper->ambient.l.colc[1] = green;
    mapper->ambient.l.col[1] = green;
    mapper->ambient.l.colc[2] = blue;
    mapper->ambient.l.col[2] = blue;
    mapper->numLights = 0;
}

// TODO regalloc
void Lights_UploadLights(z_LightMapper* mapper, z_GraphicsContext* gCtxt) {
    s32 i;
    Light* l;

    gSPNumLights(gCtxt->polyOpa.append++, mapper->numLights);
    gSPNumLights(gCtxt->polyXlu.append++, mapper->numLights);

    l = &mapper->lights[0];

    for (i = 0; i < mapper->numLights;) {
        gSPLight(gCtxt->polyOpa.append++, l, ++i);
        gSPLight(gCtxt->polyXlu.append++, l++, i);
    }

    gSPLight(gCtxt->polyOpa.append++, &mapper->ambient, ++i);
    gSPLight(gCtxt->polyXlu.append++, &mapper->ambient, i);
}

Light* Lights_MapperGetNextFreeSlot(z_LightMapper* mapper) {
    if (6 < mapper->numLights) {
        return NULL;
    }
    return &mapper->lights[mapper->numLights++];
}

GLOBAL_ASM("./asm/nonmatching/z_lights/Lights_MapPositionalWithReference.asm")

// TODO this uses a float that's stored in .rodata, which we can't handle right now
void Lights_MapPositional(z_LightMapper* mapper, z_LightInfoPositionalParams* params, z_GlobalContext* ctxt) {
    Light* light;
    f32 radiusF = params->radius;
    z_Vector3f posF;
    z_Vector3f adjustedPos;
    u32 pad;
    if (radiusF > 0) {
        posF.x = params->posX;
        posF.y = params->posY;
        posF.z = params->posZ;
        Matrix_MultiplyByVectorXYZ(&ctxt->unk187B0,&posF,&adjustedPos);
        if ((adjustedPos.z > -radiusF) &&
            (600 + radiusF > adjustedPos.z) &&
            (400 > fabsf(adjustedPos.x) - radiusF) &&
            (400 > fabsf(adjustedPos.y) - radiusF)) {
            light = Lights_MapperGetNextFreeSlot(mapper);
            if (light != NULL) {
                radiusF = 4500000.0f / (radiusF * radiusF);
                if (radiusF > 255) {
                    radiusF = 255;
                } else if (20 > radiusF) {
                    radiusF = 20;
                }

                light->lPos.col[0] = params->red;
                light->lPos.colc[0] = light->lPos.col[0];
                light->lPos.col[1] = params->green;
                light->lPos.colc[1] = light->lPos.col[1];
                light->lPos.col[2] = params->blue;
                light->lPos.colc[2] = light->lPos.col[2];
                light->lPos.pos[0] = params->posX;
                light->lPos.pos[1] = params->posY;
                light->lPos.pos[2] = params->posZ;
                light->lPos.pad1 = 0x8;
                light->lPos.pad2 = 0xFF;
                light->lPos.unkE = (s8)radiusF;
            }
        }
    }
}

void Lights_MapDirectional(z_LightMapper* mapper, z_LightInfoDirectionalParams* params, z_GlobalContext* ctxt) {
    Light* light = Lights_MapperGetNextFreeSlot(mapper);

    if (light != NULL) {
        light->l.col[0] = params->red;
        light->l.colc[0] = light->l.col[0];
        light->l.col[1] = params->green;
        light->l.colc[1] = light->l.col[1];
        light->l.col[2] = params->blue;
        light->l.colc[2] = light->l.col[2];
        light->l.dir[0] = params->dirX;
        light->l.dir[1] = params->dirY;
        light->l.dir[2] = params->dirZ;
        light->l.pad1 = 0;
    }
}

void Lights_MapLights(z_LightMapper* mapper, z_Light* lights, z_Vector3f* refPos, z_GlobalContext* ctxt) {
    if (lights != NULL) {
        if ((refPos == NULL) && (mapper->enablePosLights == 1)) {
            do {
                lightPositionalMapFuncs[lights->info->type](mapper, &lights->info->params, ctxt);
                lights = lights->next;
            } while (lights != NULL);
        } else {
            do {
                lightDirectionalMapFuncs[lights->info->type](mapper, &lights->info->params, refPos);
                lights = lights->next;
            } while (lights != NULL);
        }
    }
}

z_Light* Lights_FindFreeSlot(void) {
    z_Light* ret;

    if (0x1f < lightsList.numOccupied) {
        return NULL;
    }

    ret = &lightsList.lights[lightsList.nextFree];
    while (ret->info != NULL) {
        lightsList.nextFree++;
        if (lightsList.nextFree < 0x20) {
            ret++;
        } else {
            lightsList.nextFree = 0;
            ret = &lightsList.lights[0];
        }
    }

    lightsList.numOccupied++;
    return ret;
}

void Lights_Free(z_Light* light) {
    if (light != NULL) {
        lightsList.numOccupied--;
        light->info = NULL;
        lightsList.nextFree = (light - lightsList.lights) / (s32)sizeof(z_Light); //! @bug Due to pointer arithmetic, the division is unnecessary
    }
}

void Lights_Init(z_GlobalContext* ctxt, z_LightingContext* lCtxt) {
    Lights_ClearHead(ctxt, lCtxt);
    Lights_SetAmbientColor(lCtxt, 80, 80, 80);
    func_80102544(lCtxt, 0, 0, 0, 0x3e4, 0x3200);
    _blkclr(&lightsList, sizeof(z_LightsList));
}

void Lights_SetAmbientColor(z_LightingContext* lCtxt, u8 red, u8 green, u8 blue) {
    lCtxt->ambientRed = red;
    lCtxt->ambientGreen = green;
    lCtxt->ambientBlue = blue;
}

void func_80102544(z_LightingContext* lCtxt, u8 a1, u8 a2, u8 a3, s16 sp12, s16 sp16) {
    lCtxt->unk7 = a1;
    lCtxt->unk8 = a2;
    lCtxt->unk9 = a3;
    lCtxt->unkA = sp12;
    lCtxt->unkC = sp16;
}

z_LightMapper* Lights_CreateMapper(z_LightingContext* lCtxt, z_GraphicsContext* gCtxt) {
    return Lights_MapperAllocateAndInit(gCtxt, lCtxt->ambientRed, lCtxt->ambientGreen, lCtxt->ambientBlue);
}

void Lights_ClearHead(z_GlobalContext* ctxt, z_LightingContext* lCtxt) {
    lCtxt->lightsHead = NULL;
}

void Lights_RemoveAll(z_GlobalContext* ctxt, z_LightingContext* lCtxt) {
    while (lCtxt->lightsHead != NULL) {
        Lights_Remove(ctxt, lCtxt, lCtxt->lightsHead);
        lCtxt->lightsHead = lCtxt->lightsHead->next;
    }
}

z_Light* Lights_Insert(z_GlobalContext* ctxt, z_LightingContext* lCtxt, z_LightInfo* info) {
    z_Light* light;

    light = Lights_FindFreeSlot();
    if (light != NULL) {
        light->info = info;
        light->prev = NULL;
        light->next = lCtxt->lightsHead;

        if (lCtxt->lightsHead != NULL) {
            lCtxt->lightsHead->prev = light;
        }

        lCtxt->lightsHead = light;
    }

    return light;
}

void Lights_Remove(z_GlobalContext* ctxt, z_LightingContext* lCtxt, z_Light* light) {
    if (light != NULL) {
        if (light->prev != NULL) {
            light->prev->next = light->next;
        } else {
            lCtxt->lightsHead = light->next;
        }

        if (light->next != NULL) {
            light->next->prev = light->prev;
        }

        Lights_Free(light);
    }
}

// TODO regalloc
z_LightMapper* func_801026E8(z_GraphicsContext* gCtxt, u8 ambientRed, u8 ambientGreen, u8 ambientBlue, u8 numLights, u8 red, u8 green, u8 blue, s8 dirX, s8 dirY, s8 dirZ) {
    z_LightMapper* mapper;
    u32 i;

    // TODO allocation should be a macro
    mapper = (z_LightMapper *)((int)gCtxt->polyOpa.appendEnd - sizeof(z_LightMapper));
    gCtxt->polyOpa.appendEnd = (void*)mapper;

    mapper->ambient.l.col[0] = ambientRed;
    mapper->ambient.l.colc[0] = ambientRed;
    mapper->ambient.l.col[1] = ambientGreen;
    mapper->ambient.l.colc[1] = ambientGreen;
    mapper->ambient.l.col[2] = ambientBlue;
    mapper->ambient.l.colc[2] = ambientBlue;
    mapper->enablePosLights = 0;
    mapper->numLights = numLights;

    for (i = 0; i < numLights; i++) {
        mapper->lights[i].l.colc[0] = red;
        mapper->lights[i].l.col[0] = red;
        mapper->lights[i].l.colc[1] = green;
        mapper->lights[i].l.col[1] = green;
        mapper->lights[i].l.colc[2] = blue;
        mapper->lights[i].l.col[2] = blue;
        mapper->lights[i].l.dir[0] = dirX;
        mapper->lights[i].l.dir[1] = dirY;
        mapper->lights[i].l.dir[2] = dirZ;
    }

    Lights_UploadLights(mapper,gCtxt);

    return mapper;
}

z_LightMapper* Lights_MapperAllocateAndInit(z_GraphicsContext* gCtxt, u8 red, u8 green, u8 blue) {
    z_LightMapper* mapper;

    // TODO allocation should be a macro
    mapper = (z_LightMapper *)((int)gCtxt->polyOpa.appendEnd - sizeof(z_LightMapper));
    gCtxt->polyOpa.appendEnd = (void*)mapper;

    mapper->ambient.l.col[0] = red;
    mapper->ambient.l.colc[0] = red;
    mapper->ambient.l.col[1] = green;
    mapper->ambient.l.colc[1] = green;
    mapper->ambient.l.col[2] = blue;
    mapper->ambient.l.colc[2] = blue;
    mapper->enablePosLights = 0;
    mapper->numLights = 0;

    return mapper;
}

// TODO regalloc
// TODO this uses a float that's stored in .rodata, which we can't handle right now
void func_80102880(z_GlobalContext* ctxt) {
    z_Light* light = ctxt->lightsContext.lightsHead;
    z_LightInfoPositionalParams* params;
    z_Vector3f local_14;
    z_Vector3f local_20;
    f32 local_24;
    f32 fVar4;
    s32 s2;
    u32 pad[2];

    while (light != NULL) {
        if (light->info->type == 2) {
            params = (z_LightInfoPositionalParams*)&light->info->params;
            local_14.x = params->posX;
            local_14.y = params->posY;
            local_14.z = params->posZ;
            func_800B4EDC(ctxt, &local_14, &local_20, &local_24);

            params->unk9 = 0;

            if ((local_20.z > 1) &&
                (fabsf(local_20.x * local_24) < 1) &&
                (fabsf(local_20.y * local_24) < 1)) {
                fVar4 = local_20.z * local_24;
                s2 = (s32)(fVar4 * 16352) + 16352;
                if (s2 < func_80178A94(local_20.x * local_24 * 160 + 160, local_20.y * local_24 * -120 + 120)) {
                    params->unk9 = 1;
                }
            }
        }

        light = light->next;
    }
}

// TODO regalloc
// TODO this uses a float that's stored in .rodata, which we can't handle right now
void func_80102A64(z_GlobalContext *ctxt) {
    Gfx* dl;
    z_LightInfoPositionalParams* params;
    f32 scale;
    z_GraphicsContext* gCtxt;
    z_Light* light = ctxt->lightsContext.lightsHead;

    if (light != NULL) {
        gCtxt = ctxt->commonVars.graphicsContext;
        dl = func_8012C7FC(gCtxt->polyXlu.append);

        gSPSetOtherMode(dl++, G_SETOTHERMODE_H, 4, 4, 0x00000080); //! This doesn't resolve to any of the macros in gdi.h

        gDPSetCombineLERP(dl++, 0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0,
                                0, 0, 0, PRIMITIVE, TEXEL0, 0, PRIMITIVE, 0);

        gSPDisplayList(dl++, &D_04029CB0);

        do {
            if (light->info->type == 2) {
                params = (z_LightInfoPositionalParams*)&light->info->params;
                if (params->unk9 != 0) {
                    scale = (f32)params->radius * (f32)params->radius * 2e-6f;

                    gDPSetPrimColor(dl++, 0, 0, params->red, params->green, params->blue, 50);

                    SysMatrix_InsertTranslation(params->posX, params->posY, params->posZ, 0);
                    SysMatrix_InsertScale(scale,scale,scale,1);

                    gSPMatrix(dl++, SysMatrix_AppendStateToPolyOpaDisp((ctxt->commonVars).graphicsContext), G_MTX_PUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

                    gSPDisplayList(dl++, &D_04029CF0);
                }
            }

            light = light->next;
        } while (light != NULL);

        gCtxt->polyXlu.append = dl;
    }
}
