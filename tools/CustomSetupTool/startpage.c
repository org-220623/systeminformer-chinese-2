/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     dmex
 *
 */

#include "setup.h"

VOID SetupShowBrowseDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    PVOID fileDialog;

    if (fileDialog = PhCreateOpenFileDialog())
    {
        PhSetFileDialogOptions(fileDialog, PH_FILEDIALOG_PICKFOLDERS);

        if (PhShowFileDialog(Context->DialogHandle, fileDialog))
        {
            PPH_STRING fileDialogFolderPath;

            fileDialogFolderPath = PhGetFileDialogFileName(fileDialog);
            PhTrimToNullTerminatorString(fileDialogFolderPath);
            PhSwapReference(&Context->SetupInstallPath, fileDialogFolderPath);
        }

        PhFreeFileDialog(fileDialog);
    }

    if (PhIsNullOrEmptyString(Context->SetupInstallPath))
    {
        Context->SetupInstallPath = SetupFindInstallDirectory();
    }

    if (!PhIsNullOrEmptyString(Context->SetupInstallPath))
    {
        if (!PhEndsWithStringRef(&Context->SetupInstallPath->sr, &PhNtPathSeparatorString, TRUE))
        {
            PhSwapReference(&Context->SetupInstallPath, PhConcatStringRef2(&Context->SetupInstallPath->sr, &PhNtPathSeparatorString));
        }
    }
}

_Function_class_(PH_ENUM_DIRECTORY_FILE)
static BOOLEAN CALLBACK SetupCheckDirectoryCallback(
    _In_ HANDLE RootDirectory,
    _In_ PFILE_DIRECTORY_INFORMATION Information,
    _In_ PVOID Context
    )
{
    PH_STRINGREF baseName;

    baseName.Buffer = Information->FileName;
    baseName.Length = Information->FileNameLength;

    if (PhEqualStringRef2(&baseName, L".", TRUE) || PhEqualStringRef2(&baseName, L"..", TRUE))
        return TRUE;

    (*(PULONG)Context) += 1;
    return FALSE;
}

BOOLEAN SetupShowDirectoryWarningPrompt(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    if (PhDoesFileExistWin32(PhGetString(Context->SetupInstallPath)))
    {
        HANDLE directoryHandle;
        ULONG count = 0;

        if (NT_SUCCESS(PhCreateFileWin32(
            &directoryHandle,
            PhGetString(Context->SetupInstallPath),
            FILE_LIST_DIRECTORY | SYNCHRONIZE,
            FILE_ATTRIBUTE_DIRECTORY,
            FILE_SHARE_READ,
            FILE_OPEN,
            FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT
            )))
        {
            PhEnumDirectoryFile(directoryHandle, NULL, SetupCheckDirectoryCallback, &count);
            NtClose(directoryHandle);
        }

        if (count != 0)
        {
            //PhShowMessage2(
            //    Context->DialogHandle,
            //    TDCBF_YES_BUTTON | TDCBF_NO_BUTTON,
            //    TD_WARNING_ICON,
            //    L"警告",
            //    L"The installation directory already contains files and data. Please select a different directory"
            //    L" or click Yes to delete the files and data and continue. Are you sure you want to continue?"
            //    );

            return TRUE;
        }
    }

    return FALSE;
}

VOID ShowErrorPageDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    TASKDIALOGCONFIG config;
    PPH_STRING string;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = Context->IconLargeHandle;
    config.pszWindowTitle = PhApplicationName;
    config.pszMainInstruction = L"安装程序出现错误。";
    config.cxWidth = 200;

    if (string = PhGetStatusMessage(Context->LastStatus, 0))
        config.pszContent = PhaFormatString(L"%s\r\n\r\n点击\"关闭\"退出安装程序", PhGetString(string))->Buffer;
    else
        config.pszContent = L"点击 \"关闭\" 退出安装程序";

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

HRESULT CALLBACK SetupWelcomePageCallbackProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR dwRefData
    )
{
    PPH_SETUP_CONTEXT context = (PPH_SETUP_CONTEXT)dwRefData;

    switch (uMsg)
    {
    case TDN_NAVIGATED:
        {
            PhCenterWindow(hwndDlg, NULL);

            if (!PhGetOwnTokenAttributes().Elevated)
            {
                SendMessage(hwndDlg, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, IDCONTINUE, TRUE);
            }
        }
        break;
    case TDN_BUTTON_CLICKED:
        {
            if ((INT)wParam == IDCONTINUE)
            {
#ifndef FORCE_TEST_UPDATE_LOCAL_INSTALL
                if (PhGetOwnTokenAttributes().Elevated)
                {
                    if (!context->SetupIsLegacyUpdate && SetupLegacySetupInstalled())
                    {
                        SetupShowMessagePromptForLegacyVersion();
                    }

                    ShowConfigPageDialog(context);
                    return S_FALSE;
                }
                else
                {
                    NTSTATUS status;
                    PPH_STRING applicationFileName;
                    PPH_STRING applicationCommandLine;
                    PH_STRINGREF applicationCommandLineStringRef;

                    if (!NT_SUCCESS(status = PhGetProcessCommandLineStringRef(&applicationCommandLineStringRef)))
                    {
                        context->LastStatus = status;
                        return S_FALSE;
                    }
                    if (!(applicationFileName = PhGetApplicationFileNameWin32()))
                    {
                        context->LastStatus = STATUS_NO_MEMORY;
                        return S_FALSE;
                    }
                    applicationCommandLine = PhCreateString2(&applicationCommandLineStringRef);

                    status = PhShellExecuteEx(
                        hwndDlg,
                        PhGetString(applicationFileName),
                        PhGetString(applicationCommandLine),
                        NULL,
                        SW_SHOW,
                        PH_SHELL_EXECUTE_ADMIN,
                        0,
                        &context->SubProcessHandle
                        );

                    PhDereferenceObject(applicationCommandLine);
                    PhDereferenceObject(applicationFileName);

                    if (NT_SUCCESS(status))
                    {
                        ShowWindow(hwndDlg, SW_HIDE);
                    }
                    else
                    {
                        context->LastStatus = status;
                        return S_FALSE;
                    }
                }
#else
                ShowConfigPageDialog(context);
                return S_FALSE;
#endif
            }
        }
        break;
    }

    return S_OK;
}

VOID ShowWelcomePageDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    TASKDIALOG_BUTTON buttonArray[] =
    {
        { IDCONTINUE, L"安装" }
    };
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = Context->IconLargeHandle;
    config.pButtons = buttonArray;
    config.cButtons = ARRAYSIZE(buttonArray);
    config.pfCallback = SetupWelcomePageCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.pszWindowTitle = PhApplicationName;
    config.pszMainInstruction = PhApplicationName;
    config.pszContent = L"一款免费、开源、功能强大、用途广泛的工具，可帮助您监控系统资源、调试软件和检测恶意软件。";
    config.cxWidth = 200;

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

HRESULT CALLBACK SetupCompletePageCallbackProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR dwRefData
    )
{
    PPH_SETUP_CONTEXT context = (PPH_SETUP_CONTEXT)dwRefData;

    switch (uMsg)
    {
    case TDN_NAVIGATED:
        break;
    }

    return S_OK;
}

VOID ShowCompletedPageDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED | TDF_VERIFICATION_FLAG_CHECKED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = Context->IconLargeHandle;
    config.pfCallback = SetupCompletePageCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.pszWindowTitle = PhApplicationName;
    config.pszMainInstruction = PhaConcatStrings2(PhApplicationName, L" 已完成")->Buffer;
    config.pszContent = L"点击 \"关闭\" 退出安装程序";
    config.pszVerificationText = L"启动 System Informer";
    config.cxWidth = 200;

#ifdef FORCE_TEST_UPDATE_LOCAL_INSTALL
    ClearFlag(config.dwFlags, TDF_VERIFICATION_FLAG_CHECKED);
#endif

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

HRESULT CALLBACK SetupConfigPageCallbackProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR dwRefData
    )
{
    PPH_SETUP_CONTEXT context = (PPH_SETUP_CONTEXT)dwRefData;

    switch (uMsg)
    {
    case TDN_NAVIGATED:
        {
            PPH_STRING status;

            status = PhFormatString(
                L"安装目录: \r\n\r\n%s",
                PhGetStringOrEmpty(context->SetupInstallPath)
                );
            SendMessage(hwndDlg, TDM_UPDATE_ELEMENT_TEXT, TDE_CONTENT, (LPARAM)status->Buffer);
            PhDereferenceObject(status);
        }
        break;
    case TDN_BUTTON_CLICKED:
        {
            if ((INT)wParam == IDYES)
            {
                PPH_STRING status;

                SetupShowBrowseDialog(context);

                status = PhFormatString(
                    L"安装目录: \r\n\r\n%s",
                    PhGetStringOrEmpty(context->SetupInstallPath)
                    );
                SendMessage(hwndDlg, TDM_UPDATE_ELEMENT_TEXT, TDE_CONTENT, (LPARAM)status->Buffer);
                PhDereferenceObject(status);

                return S_FALSE;
            }

            if ((INT)wParam == IDOK)
            {
                if (PhIsNullOrEmptyString(context->SetupInstallPath))
                    return S_FALSE;

                //if (SetupShowDirectoryWarningPrompt(context))
                //    return S_FALSE;

                if (SetupShowDirectoryWarningPrompt(context))
                {
                    ShowConfigDirectoryNonEmptyDialog(context);
                    return S_FALSE;
                }

                ShowInstallPageDialog(context);
                return S_FALSE;
            }
        }
        break;
    }

    return S_OK;
}

VOID ShowConfigPageDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    TASKDIALOG_BUTTON buttonConfig[] =
    {
        { IDYES, L"浏览" },
        { IDOK, L"下一步" },
    };
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = Context->IconLargeHandle;
    config.pButtons = buttonConfig;
    config.cButtons = ARRAYSIZE(buttonConfig);
    config.pfCallback = SetupConfigPageCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.cxWidth = 200;
    config.pszWindowTitle = PhApplicationName;
    config.pszMainInstruction = L"安装选项";
    config.pszContent = L"安装目录: \r\n\r\n选择 \"浏览\" 以继续。";

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

HRESULT CALLBACK SetupDirectoryNonEmptyTaskDialogCallbackProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR dwRefData
    )
{
    PPH_SETUP_CONTEXT context = (PPH_SETUP_CONTEXT)dwRefData;

    switch (uMsg)
    {
    case TDN_BUTTON_CLICKED:
        {
            if ((INT)wParam == IDNO)
            {
                ShowInstallPageDialog(context);
                return S_FALSE;
            }

            if ((INT)wParam == IDYES)
            {
                ShowConfigPageDialog(context);
                return S_FALSE;
            }
        }
        break;
    }

    return S_OK;
}

VOID ShowConfigDirectoryNonEmptyDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    TASKDIALOG_BUTTON buttonConfig[] =
    {
        { IDYES, L"更改目录" },
        { IDNO, L"继续" },
    };
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.pszMainIcon = TD_WARNING_ICON;
    config.pButtons = buttonConfig;
    config.cButtons = ARRAYSIZE(buttonConfig);
    config.pfCallback = SetupDirectoryNonEmptyTaskDialogCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.cxWidth = 200;
    config.pszWindowTitle = PhApplicationName;
    config.pszMainInstruction = L"警告";
    config.pszContent = L"选定的安装目录已包含文件和数据。"
        L"如果继续操作，此目录及其中的文件将被删除。\r\n\r\n您要更改目录吗?";

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

HRESULT CALLBACK SetupInstallPageCallbackProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR dwRefData
)
{
    PPH_SETUP_CONTEXT context = (PPH_SETUP_CONTEXT)dwRefData;

    switch (uMsg)
    {
    case TDN_NAVIGATED:
        {
            SendMessage(hwndDlg, TDM_SET_MARQUEE_PROGRESS_BAR, TRUE, 0);
            SendMessage(hwndDlg, TDM_SET_PROGRESS_BAR_MARQUEE, TRUE, 1);

            PhCreateThread2(SetupProgressThread, context);
        }
        break;
    case TDN_BUTTON_CLICKED:
        {
            return S_FALSE;
        }
        break;
    }

    return S_OK;
}

VOID ShowInstallPageDialog(
    _In_ PPH_SETUP_CONTEXT Context
    )
{
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED | TDF_SHOW_MARQUEE_PROGRESS_BAR;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = Context->IconLargeHandle;
    config.pfCallback = SetupInstallPageCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.cxWidth = 200;
    config.pszWindowTitle = PhApplicationName;
    config.pszMainInstruction = L"准备安装...";
    config.pszContent = L" ";

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}
