#include <Windows.h>
#include <WindowsX.h>
#include <commctrl.h>
#include <stdio.h>
#include "resource.h"
#include "ui.h"
#include "version.h"

struct LoadDialogData
{
    int selectedIndex;
    int imageCount;
    HANDLE file;
};

struct SaveDialogData
{
    bool hasAlphaChannel;
    bool enableDXTCompression;
    bool enableMipMaps;
    bool hasMipMaps;
    SaveDialogOptions options;
};

// Centers a dialog on the parent window
static void CenterDialog(HWND hDlg)
{
    int  nHeight;
    int  nWidth;
    RECT rcDialog;
    RECT rcParent;
    POINT point;

    HWND hParent = GetParent(hDlg);

    if (hParent == nullptr)
    {
        hParent = GetDesktopWindow();
    }
    GetClientRect(hParent, &rcParent);

    GetWindowRect(hDlg, &rcDialog);
    nWidth  = rcDialog.right  - rcDialog.left;
    nHeight = rcDialog.bottom - rcDialog.top;

    point.x = (rcParent.right - rcParent.left) / 2;
    point.y = (rcParent.bottom - rcParent.top) / 2;

    ClientToScreen(hParent, &point);

    point.x -= nWidth / 2;
    point.y -= nHeight / 2;

    SetWindowPos(hDlg, HWND_TOP, point.x, point.y, nWidth, nHeight, SWP_NOSIZE | SWP_NOZORDER);
}

static void InitAboutDialog(HWND dp)
{
    char s[256], format[256];

    GetDlgItemText(dp, ABOUTFORMAT, format, 256);
    sprintf_s(s, format, VERSION_STR);
    SetDlgItemText(dp, ABOUTFORMAT, s);
}

static INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);

    int cmd;
    switch (wMsg)
    {
    case WM_INITDIALOG:
        CenterDialog(hDlg);
        InitAboutDialog(hDlg);
        break;
    case WM_LBUTTONUP:
        EndDialog(hDlg, IDOK);
        break;
    case WM_COMMAND:
        cmd = HIWORD(wParam);

        if (cmd == BN_CLICKED)
        {
            EndDialog(hDlg, IDOK);
        }
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

void DoAbout (AboutRecordPtr about)
{
    PlatformData* platform = reinterpret_cast<PlatformData*>(about->platformData);

    DialogBoxParam(reinterpret_cast<HINSTANCE>(hDllInstance), MAKEINTRESOURCE(IDD_ABOUT), reinterpret_cast<HWND>(platform->hwnd), AboutDlgProc, 0);
}

static void InitLoadDlg(HWND dp, const LoadDialogData* dialogData)
{
    char s[256], format[256];

    GetDlgItemText(dp, IMAGECOUNTLABEL, format, 256);
    sprintf_s(s, format, dialogData->imageCount);
    SetDlgItemText(dp, IMAGECOUNTLABEL, s);

    HWND menu = GetDlgItem(dp, FSHCOMBOITEM);

    for (int i = 0; i < dialogData->imageCount; i++)
    {
        FshBmpEntry entry;
        if (GetFshBmpInfo(dialogData->file, i, &entry) != noErr)
        {
            break;
        }


        const int code = (entry.code & 0x7f);
        const char* bmpformat;

        switch (code)
        {
            case 0x60:
                bmpformat = "DXT1";
            break;
            case 0x61:
                bmpformat = "DXT3";
            break;
            case 0x7d:
                bmpformat = "32-bit ARGB";
            break;
            case 0x7f:
                bmpformat = "24-bit RGB";
            break;
            case 0x7e:
                bmpformat = "16-bit ARGB (1:5:5:5)";
            break;
            case 0x78:
                bmpformat = "16-bit RGB (0:5:6:5)";
            break;
            case 0x6d:
                bmpformat = "16-bit ARGB (4:4:4:4)";
            break;
            default:
                bmpformat = "Unknown";
                break;
        }

        sprintf_s(s, "#%d %ldx%ld %s", (i+1), entry.width, entry.height, bmpformat);

        ComboBox_AddString(menu, s);
    }
    ComboBox_SetCurSel(menu, 0);
}

static bool LoadFshItem(HWND dp, int item, int* selectedIndex)
{
    switch (item)
    {
    case IDOK:
        *selectedIndex = ComboBox_GetCurSel(GetDlgItem(dp, FSHCOMBOITEM));
        return true;
    case IDCANCEL:
        return true;
    }

    return false;
}

static INT_PTR CALLBACK LoadDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    LoadDialogData* dialogData = reinterpret_cast<LoadDialogData*>(lParam);

    if (wMsg == WM_INITDIALOG)
    {
        dialogData = reinterpret_cast<LoadDialogData*>(lParam);

        SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dialogData));
        CenterDialog(hDlg);
        InitLoadDlg(hDlg, dialogData);
    }
    else
    {
        dialogData = reinterpret_cast<LoadDialogData*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));

        int item, cmd;
        switch (wMsg)
        {
        case WM_COMMAND:
            item = LOWORD(wParam);
            cmd = HIWORD(wParam);

            if (cmd == BN_CLICKED && LoadFshItem(hDlg, item, &dialogData->selectedIndex))
            {
                EndDialog(hDlg, item);
            }
            break;

        default:
            return FALSE;
        }
    }

    return TRUE;
}

bool LoadFshDlg(FormatRecordPtr pb, const FshHeader& header, int* selectedIndex)
{
    *selectedIndex = 0;

    PlatformData* platform = reinterpret_cast<PlatformData*>(pb->platformData);
    const HWND hWndParent = reinterpret_cast<HWND>(platform->hwnd);

    LoadDialogData data;
    data.file = reinterpret_cast<HANDLE>(pb->dataFork);
    data.imageCount = header.numBmps;
    data.selectedIndex = 0;

    if (DialogBoxParamA(reinterpret_cast<HINSTANCE>(hDllInstance), MAKEINTRESOURCE(IDD_FSHLOAD), hWndParent, LoadDlgProc, reinterpret_cast<LPARAM>(&data)) == IDOK)
    {
        *selectedIndex = data.selectedIndex;

        return true;
    }

    return false;
}

static void InitSaveDlg(HWND dp, const SaveDialogData* dialogData)
{
    FshBmpType fshType = dialogData->options.fshType;

    if (dialogData->hasAlphaChannel)
    {
        EnableWindow(GetDlgItem(dp, FSHTYPE_32BIT), TRUE);
        EnableWindow(GetDlgItem(dp, FSHTYPE_DXT3), TRUE);
        EnableWindow(GetDlgItem(dp, SIXTEENBITALPHA), TRUE);
        EnableWindow(GetDlgItem(dp, SIXTEENBIT4x4), TRUE);

        CheckDlgButton(dp, FSHTYPE_24BIT, (fshType == TwentyFourBit));
        CheckDlgButton(dp, FSHTYPE_32BIT, (fshType == ThirtyTwoBit));
        CheckDlgButton(dp, FSHTYPE_DXT1, (fshType == DXT1));
        CheckDlgButton(dp, FSHTYPE_DXT3, (fshType == DXT3));
        CheckDlgButton(dp, SIXTEENBIT, (fshType == SixteenBit));
        CheckDlgButton(dp, SIXTEENBITALPHA, (fshType == SixteenBitAlpha));
        CheckDlgButton(dp, SIXTEENBIT4x4, (fshType == SixteenBit4x4));
    }
    else
    {
        EnableWindow(GetDlgItem(dp, FSHTYPE_32BIT), FALSE);
        EnableWindow(GetDlgItem(dp, FSHTYPE_DXT3), FALSE);
        EnableWindow(GetDlgItem(dp, SIXTEENBITALPHA), FALSE);
        EnableWindow(GetDlgItem(dp, SIXTEENBIT4x4), FALSE);

        CheckDlgButton(dp, FSHTYPE_24BIT, (fshType == TwentyFourBit || fshType == ThirtyTwoBit));
        CheckDlgButton(dp, FSHTYPE_32BIT, FALSE);

        CheckDlgButton(dp, FSHTYPE_DXT1, (fshType == DXT1 || fshType == DXT3));
        CheckDlgButton(dp, FSHTYPE_DXT3, FALSE);

        CheckDlgButton(dp, SIXTEENBIT, (fshType == SixteenBit || fshType == SixteenBitAlpha || fshType == SixteenBit4x4));
        CheckDlgButton(dp, SIXTEENBITALPHA, FALSE);
        CheckDlgButton(dp, SIXTEENBIT4x4, FALSE);

        switch (fshType)
        {
        case ThirtyTwoBit:
            fshType = TwentyFourBit;
            break;
        case DXT3:
            fshType = DXT1;
            break;
        case SixteenBitAlpha:
        case SixteenBit4x4:
            fshType = SixteenBit;
            break;
        }
    }

    if (dialogData->enableDXTCompression)
    {
        CheckDlgButton(dp, IDC_FSHWRITE, dialogData->options.fshWriteCompression);
    }
    else
    {
        EnableWindow(GetDlgItem(dp, FSHTYPE_DXT1), FALSE);
        EnableWindow(GetDlgItem(dp, FSHTYPE_DXT3), FALSE);
        EnableWindow(GetDlgItem(dp, IDC_FSHWRITE), FALSE);

        CheckDlgButton(dp, FSHTYPE_DXT1, FALSE);
        CheckDlgButton(dp, FSHTYPE_DXT3, FALSE);
        CheckDlgButton(dp, IDC_FSHWRITE, FALSE);

        if (fshType == DXT1 || !dialogData->hasAlphaChannel)
        {
            CheckDlgButton(dp, FSHTYPE_24BIT, TRUE);
            fshType = TwentyFourBit;
        }
        else
        {
            CheckDlgButton(dp, FSHTYPE_32BIT, TRUE);
            fshType = ThirtyTwoBit;
        }
    }

    if (dialogData->enableMipMaps)
    {
        if (dialogData->hasMipMaps)
        {
            CheckDlgButton(dp, EMBEDMIPMAPS, TRUE);
            EnableWindow(GetDlgItem(dp, EMBEDMIPMAPS), FALSE);
        }
        else
        {
            CheckDlgButton(dp, EMBEDMIPMAPS, FALSE);
            EnableWindow(GetDlgItem(dp, EMBEDMIPMAPS), TRUE);
        }
    }
    else
    {
        CheckDlgButton(dp, EMBEDMIPMAPS, FALSE);
        EnableWindow(GetDlgItem(dp, EMBEDMIPMAPS), FALSE);
    }

    HWND editTxtHwnd = GetDlgItem(dp, ENTRYDIRTXT);
    Edit_LimitText(editTxtHwnd, 4);

    if (dialogData->options.entryDirName[0] != 0)
    {
        char name[5];
        memcpy(name, dialogData->options.entryDirName, 4);
        name[4] = '\0';

        SetWindowTextA(editTxtHwnd, name);
    }
    else
    {
        SetWindowTextA(editTxtHwnd, "FiSH");
    }
}

static bool SaveFshItem(HWND dp, int item, SaveDialogOptions* outputParams)
{
    switch (item)
    {
    case IDOK:
        if (GetWindowTextLengthA(GetDlgItem(dp, ENTRYDIRTXT)) == 4)
        {
            char name[5];
            ZeroMemory(name, 5);
            if (GetWindowTextA(GetDlgItem(dp, ENTRYDIRTXT), name, 5) == 4)
            {
                memcpy(outputParams->entryDirName, name, 4);
            }
        }
        return true;
    case IDCANCEL:
        return true;
    case FSHTYPE_24BIT:
        outputParams->fshType = TwentyFourBit;
        break;
    case FSHTYPE_32BIT:
        outputParams->fshType = ThirtyTwoBit;
        break;
    case FSHTYPE_DXT1:
        outputParams->fshType = DXT1;
        break;
    case FSHTYPE_DXT3:
        outputParams->fshType = DXT3;
        break;
    case SIXTEENBIT:
        outputParams->fshType = SixteenBit;
        break;
    case SIXTEENBITALPHA:
        outputParams->fshType = SixteenBitAlpha;
        break;
    case SIXTEENBIT4x4:
        outputParams->fshType = SixteenBit4x4;
        break;
    case IDC_FSHWRITE:
        outputParams->fshWriteCompression = (Button_GetCheck(GetDlgItem(dp, IDC_FSHWRITE)) == BST_CHECKED);
        break;
    case EMBEDMIPMAPS:
        outputParams->embedMipmaps = (Button_GetCheck(GetDlgItem(dp, EMBEDMIPMAPS)) == BST_CHECKED);
        break;
    }

    CheckDlgButton(dp, FSHTYPE_24BIT, (outputParams->fshType == TwentyFourBit));
    CheckDlgButton(dp, FSHTYPE_32BIT, (outputParams->fshType == ThirtyTwoBit));
    CheckDlgButton(dp, FSHTYPE_DXT1, (outputParams->fshType == DXT1));
    CheckDlgButton(dp, FSHTYPE_DXT3, (outputParams->fshType == DXT3));
    CheckDlgButton(dp, SIXTEENBIT, (outputParams->fshType == SixteenBit));
    CheckDlgButton(dp, SIXTEENBITALPHA, (outputParams->fshType == SixteenBitAlpha));
    CheckDlgButton(dp, SIXTEENBIT4x4, (outputParams->fshType == SixteenBit4x4));

    return false;
}

static INT_PTR CALLBACK SaveDlgProc(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    SaveDialogData* dialogData = nullptr;

    if (wMsg == WM_INITDIALOG)
    {
        dialogData = reinterpret_cast<SaveDialogData*>(lParam);

        SetWindowLongPtr(hDlg, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dialogData));
        CenterDialog(hDlg);
        InitSaveDlg(hDlg, dialogData);
    }
    else
    {
        dialogData = reinterpret_cast<SaveDialogData*>(GetWindowLongPtr(hDlg, GWLP_USERDATA));

        int item, cmd;
        switch (wMsg)
        {
        case WM_COMMAND:
            item = LOWORD(wParam);
            cmd = HIWORD(wParam);

            if (cmd == BN_CLICKED && SaveFshItem(hDlg, item, &dialogData->options))
            {
                EndDialog(hDlg, item);
            }
            break;

        default:
            return FALSE;
        }
    }

    return TRUE;
}

static bool EnableMipMapCheckbox(FormatRecordPtr pb, const int mipCount)
{
    bool enabled = false;

    if (ChannelPortsSuiteAvailable(pb))
    {
        // Images containing mipmaps must divisible by 2.
        if ((pb->imageSize.h & 1) == 0 && (pb->imageSize.v & 1) == 0)
        {
            int numScales = 0;

            if (mipCount > 0)
            {
                numScales = mipCount;
            }
            else
            {
                int width = pb->imageSize.h;
                int height = pb->imageSize.v;

                while (width > 1 && height > 1)
                {
                    numScales++;
                    width >>= 1;
                    height >>= 1;
                }

                // The FSH format supports a maximum of 15 mipmaps.
                if (numScales > 15)
                {
                    numScales = 0;
                }
            }

            if (numScales > 0)
            {
                // The image dimensions must be divisible by the total number of mipmaps.
                if ((pb->imageSize.h % (1 << numScales)) == 0 && (pb->imageSize.v % (1 << numScales)) == 0)
                {
                    enabled = true;
                }
            }
        }
    }

    return enabled;
}

bool SaveFshDlg(FormatRecordPtr pb, const Globals* globals, SaveDialogOptions* outputDialogOptions)
{
    PlatformData* platform = reinterpret_cast<PlatformData*>(pb->platformData);
    const HWND hWndParent = reinterpret_cast<HWND>(platform->hwnd);

    SaveDialogData dialogData;

    dialogData.hasAlphaChannel = pb->planes == 4;
    // DXT compression requires images to be divisible by 4.
    dialogData.enableDXTCompression = ((pb->imageSize.h & 3) == 0 && (pb->imageSize.v & 3) == 0);
    dialogData.enableMipMaps = EnableMipMapCheckbox(pb, globals->mipCount);
    dialogData.hasMipMaps = globals->mipCount > 0;

    dialogData.options.fshType = globals->fshCode;
    memcpy(dialogData.options.entryDirName, globals->entryDir, 4);
    dialogData.options.fshWriteCompression = globals->fshWriteCompression;
    dialogData.options.embedMipmaps = false;

    if (DialogBoxParamA(reinterpret_cast<HINSTANCE>(hDllInstance), MAKEINTRESOURCE(FSHSAVEOPTIONS), hWndParent, SaveDlgProc, reinterpret_cast<LPARAM>(&dialogData)) == IDOK)
    {
        outputDialogOptions->fshType = dialogData.options.fshType;
        memcpy(outputDialogOptions->entryDirName, dialogData.options.entryDirName, 4);
        outputDialogOptions->fshWriteCompression = dialogData.options.fshWriteCompression;
        outputDialogOptions->embedMipmaps = dialogData.options.embedMipmaps;

        return true;
    }

    return false;
}