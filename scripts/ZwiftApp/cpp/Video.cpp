#include "ZwiftApp.h"
void AddVideoMaterial(const char *, const char *) {
    //TODO
}
void ReleaseVideoMaterials() {
    //TODO
}
void UpdateVideoMaterials(void) {
#if 0 //TODO
    __int64 v0; // x21
    __int64 v1; // x19
    int v2; // w22
    __int64 v3; // x8
    int v4; // w9
    unsigned int v5; // w20
    void(__fastcall * v6)(_QWORD, _QWORD, __int64); // x8

    v0 = qword_257BC38;
    if (qword_257BC38)
    {
        while (1)
        {
            v1 = *(_QWORD *)(v0 + 40);
            if (*(_QWORD *)(v1 + 24))
                break;
        LABEL_3:
            v0 = *(_QWORD *)v0;
            if (!v0)
                return;
        }
        v2 = *(_DWORD *)(v1 + 84);
        if (((unsigned int)VideoPlayer::CheckForNewFrame(*(char ***)(v0 + 40)) & 1) == 0)
            goto LABEL_12;
        if ((v2 & 2) == 0)
            VideoPlayer::DrawFrame((VideoPlayer *)v1);
        if (*(_QWORD *)(v1 + 64))
        {
            v3 = *(_QWORD *)(v1 + 24);
            v4 = *(_DWORD *)(v3 + 12);
            if (v4)
            {
                v5 = 2 * (v4 == *(_DWORD *)(v3 + 8));
                if ((v2 & 2) == 0)
                    goto LABEL_14;
            } else
            {
                v5 = 1;
                if ((v2 & 2) == 0)
                {
                LABEL_14:
                    if (v5)
                    {
                        v6 = *(void(__fastcall **)(_QWORD, _QWORD, __int64))(v1 + 64);
                        if (v6)
                            v6(*(_QWORD *)(v1 + 72), v5, *(_QWORD *)(v1 + 24) + 12LL);
                    }
                    goto LABEL_3;
                }
            }
        } else
        {
        LABEL_12:
            v5 = 0;
            if ((v2 & 2) == 0)
                goto LABEL_14;
        }
        VideoPlayer::DrawFrame((VideoPlayer *)v1);
        goto LABEL_14;
    }
#endif
}
void ActivateVideoMaterial(const char *) {
    //TODO
}