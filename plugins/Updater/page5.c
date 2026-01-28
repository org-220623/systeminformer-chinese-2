/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     dmex    2016-2023
 *
 */

#include "updater.h"

HRESULT CALLBACK FinalTaskDialogCallbackProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ LONG_PTR dwRefData
    )
{
    PPH_UPDATER_CONTEXT context = (PPH_UPDATER_CONTEXT)dwRefData;

    switch (uMsg)
    {
    case TDN_NAVIGATED:
        {
#ifndef FORCE_NO_STATUS_TIMER
            if (context->ProgressTimer)
            {
                PhKillTimer(hwndDlg, 9000);
                context->ProgressTimer = FALSE;
            }
#endif
            context->ElevationRequired = !!UpdateCheckDirectoryElevationRequired();

#ifdef FORCE_ELEVATION_CHECK
            context->ElevationRequired = TRUE;
#endif

            if (context->ElevationRequired)
            {
                SendMessage(hwndDlg, TDM_SET_BUTTON_ELEVATION_REQUIRED_STATE, IDYES, TRUE);
            }
        }
        break;
    case TDN_BUTTON_CLICKED:
        {
            INT buttonId = (INT)wParam;

            if (buttonId == IDRETRY)
            {
                ShowCheckForUpdatesDialog(context);
                return S_FALSE;
            }
            else if (buttonId == IDYES)
            {
                if (!NT_SUCCESS(UpdateShellExecute(context, hwndDlg)))
                {
                    return S_FALSE;
                }
            }
        }
        break;
    case TDN_HYPERLINK_CLICKED:
        {
            TaskDialogLinkClicked(context);
            return S_FALSE;
        }
        break;
    }

    return S_OK;
}

VOID ShowUpdateInstallDialog(
    _In_ PPH_UPDATER_CONTEXT Context
    )
{
    TASKDIALOG_BUTTON TaskDialogButtonArray[] =
    {
        { IDYES, L"安装" }
    };
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = PhGetApplicationIcon(FALSE);
    config.cxWidth = 200;
    config.pfCallback = FinalTaskDialogCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.pButtons = TaskDialogButtonArray;
    config.cButtons = RTL_NUMBER_OF(TaskDialogButtonArray);

    config.pszWindowTitle = L"System Informer - 更新程序";
    if (Context->SwitchingChannel)
    {
        switch (Context->Channel)
        {
        case PhReleaseChannel:
            config.pszMainInstruction = L"确认切换到正式版通道?";
            break;
        //case PhPreviewChannel:
        //    config.pszMainInstruction = L"确认切换到预览版通道?";
        //    break;
        case PhCanaryChannel:
            config.pszMainInstruction = L"确认切换到测试版通道?";
            break;
        //case PhDeveloperChannel:
        //    config.pszMainInstruction = L"确认切换到开发者版通道?";
        //    break;
        default:
            config.pszMainInstruction = L"确认切换更新通道?";
            break;
        }

        config.pszContent = L"已成功从更新通道下载并验证安装程序。\r\n\r\n单击“安装”继续。";
    }
    else
    {
        config.pszMainInstruction = L"确认安装更新?";
        config.pszContent = L"更新已成功下载并验证。\r\n\r\n单击“安装”继续。";
    }

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

PPH_STRING UpdaterGetLatestVersionText(
    _In_ PPH_UPDATER_CONTEXT Context
    )
{
    PPH_STRING version;
    PPH_STRING commit;
    ULONG majorVersion;
    ULONG minorVersion;
    ULONG buildVersion;
    ULONG revisionVersion;

    PhGetPhVersionNumbers(&majorVersion, &minorVersion, &buildVersion, &revisionVersion);
    commit = PhGetPhVersionHash();

    if (commit && commit->Length > 4)
    {
        version = PhFormatString(
            L"%lu.%lu.%lu.%lu (%s)",
            majorVersion,
            minorVersion,
            buildVersion,
            revisionVersion,
            PhGetString(commit)
            );
        PhMoveReference(&version, PhFormatString(
            L"%s\r\n\r\n<A HREF=\"changelog.txt\">查看更新日志</A>",
            PhGetStringOrEmpty(version)
            ));
    }
    else
    {
        version = PhFormatString(
            L"System Informer %lu.%lu.%lu.%lu",
            majorVersion,
            minorVersion,
            buildVersion,
            revisionVersion
            );
        PhMoveReference(&version, PhFormatString(
            L"%s\r\n\r\n<A HREF=\"changelog.txt\">查看更新日志</A>",
            PhGetStringOrEmpty(version)
            ));
    }

    if (commit)
    {
        PhDereferenceObject(commit);
    }

    return version;
}

VOID ShowLatestVersionDialog(
    _In_ PPH_UPDATER_CONTEXT Context
    )
{
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED | TDF_ENABLE_HYPERLINKS;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = PhGetApplicationIcon(FALSE);
    config.cxWidth = 200;
    config.pfCallback = FinalTaskDialogCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;

    config.pszWindowTitle = L"System Informer - 更新程序";
    config.pszMainInstruction = L"您使用的软件是最新版本。";
    config.pszContent = PH_AUTO_T(PH_STRING, UpdaterGetLatestVersionText(Context))->Buffer;

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

VOID ShowNewerVersionDialog(
    _In_ PPH_UPDATER_CONTEXT Context
    )
{
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED | TDF_ENABLE_HYPERLINKS;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = PhGetApplicationIcon(FALSE);
    config.cxWidth = 200;
    config.pfCallback = FinalTaskDialogCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;

    config.pszWindowTitle = L"System Informer - 更新程序";
    config.pszMainInstruction = L"您使用的软件是预发布版本。";
    config.pszContent = PH_AUTO_T(PH_STRING, UpdaterGetLatestVersionText(Context))->Buffer;

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}

VOID ShowUpdateFailedDialog(
    _In_ PPH_UPDATER_CONTEXT Context,
    _In_ BOOLEAN HashFailed,
    _In_ BOOLEAN SignatureFailed
    )
{
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    //config.pszMainIcon = MAKEINTRESOURCE(65529);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON | TDCBF_RETRY_BUTTON;
    config.hMainIcon = PhGetApplicationIcon(FALSE);

    config.pszWindowTitle = L"System Informer - 更新程序";
    if (Context->SwitchingChannel)
        config.pszMainInstruction = L"从更新通道下载数据时出现错误。";
    else
        config.pszMainInstruction = L"下载更新时出现错误。";

    if (SignatureFailed)
    {
        if (Context->SwitchingChannel)
            config.pszContent = L"签名检查失败。单击“重试”重新从更新通道下载数据。";
        else
            config.pszContent = L"签名检查失败。单击“重试”重新下载更新。";
    }
    else if (HashFailed)
    {
        if (Context->SwitchingChannel)
            config.pszContent = L"哈希校验失败。单击“重试”重新从更新通道下载数据。";
        else
            config.pszContent = L"哈希校验失败。单击“重试”重新下载更新。";
    }
    else
    {
        if (Context->UpdateStatus)
        {
            PPH_STRING errorMessage;

            if (errorMessage = PhHttpGetErrorMessage(Context->UpdateStatus))
            {
                config.pszContent = PhaFormatString(L"[%lu] %s", Context->UpdateStatus, errorMessage->Buffer)->Buffer;
                PhDereferenceObject(errorMessage);
            }
            else if (errorMessage = PhGetStatusMessage(Context->UpdateStatus, 0))
            {
                config.pszContent = PhaFormatString(L"[%lu] %s", Context->UpdateStatus, errorMessage->Buffer)->Buffer;
                PhDereferenceObject(errorMessage);
            }
            else
            {
                if (Context->SwitchingChannel)
                    config.pszContent = L"单击“重试”重新从更新通道下载数据。";
                else
                    config.pszContent = L"单击“重试”重新下载更新。";
            }
        }
        else
        {
            if (Context->SwitchingChannel)
                config.pszContent = L"单击“重试”重新从更新通道下载数据。";
            else
                config.pszContent = L"单击“重试”重新下载更新。";
        }
    }

    config.cxWidth = 200;
    config.pfCallback = FinalTaskDialogCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;

    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}
