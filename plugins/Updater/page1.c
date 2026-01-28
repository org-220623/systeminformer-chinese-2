/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     dmex    2016-2019
 *
 */

#include "updater.h"

HRESULT CALLBACK CheckForUpdatesCallbackProc(
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
        PhSetEvent(&InitializedEvent);
        break;
    case TDN_BUTTON_CLICKED:
        {
            if ((INT)wParam == IDOK)
            {
                ShowCheckingForUpdatesDialog(context);
                return S_FALSE;
            }
        }
        break;
    case TDN_RADIO_BUTTON_CLICKED:
        {
            PH_RELEASE_CHANNEL channel;

            switch ((INT)wParam)
            {
            default:
            case IDOK:
                channel = PhReleaseChannel;
                break;
            case IDRETRY:
                channel = PhCanaryChannel;
                break;
            }

            if (PhGetPhReleaseChannel() != channel)
            {
                context->Channel = channel;
                context->SwitchingChannel = TRUE;
                PhSetIntegerSetting(SETTING_RELEASE_CHANNEL, channel);
            }
        }
        break;
    }

    return S_OK;
}

VOID ShowCheckForUpdatesDialog(
    _In_ PPH_UPDATER_CONTEXT Context
    )
{
    static TASKDIALOG_BUTTON UpdateTaskDialogButtonArray[] =
    {
        { IDOK, L"检查" }
    };
    //static TASKDIALOG_BUTTON SwitchTaskDialogButtonArray[] =
    //{
    //    { IDOK, L"是" }
    //};
    static TASKDIALOG_BUTTON checkForUpdatesRadioButtons[] =
    {
        { IDOK, L"稳定版\n - 推荐" },
        { IDRETRY, L"测试版\n - 预览" },
        //{ IDIGNORE, L"稳定版\n - 推荐" },
        //{ IDCONTINUE, L"测试版\n - 预览" },
    };
    TASKDIALOGCONFIG config;

    memset(&config, 0, sizeof(TASKDIALOGCONFIG));
    config.cbSize = sizeof(TASKDIALOGCONFIG);
    config.dwFlags = TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION | TDF_CAN_BE_MINIMIZED | TDF_ENABLE_HYPERLINKS | TDF_EXPAND_FOOTER_AREA;
    config.dwCommonButtons = TDCBF_CLOSE_BUTTON;
    config.hMainIcon = PhGetApplicationIcon(FALSE);
    config.pRadioButtons = checkForUpdatesRadioButtons;
    config.cRadioButtons = RTL_NUMBER_OF(checkForUpdatesRadioButtons);
    config.pfCallback = CheckForUpdatesCallbackProc;
    config.lpCallbackData = (LONG_PTR)Context;
    config.cxWidth = 200;

    config.pszWindowTitle = L"System Informer - 更新程序";

    switch (Context->Channel)
    {
    default:
    case PhReleaseChannel:
        config.nDefaultRadioButton = IDOK;
        break;
    case PhCanaryChannel:
        config.nDefaultRadioButton = IDRETRY;
        break;
    }

    //if (Context->SwitchingChannel)
    //{
    //    config.pButtons = SwitchTaskDialogButtonArray;
    //    config.cButtons = RTL_NUMBER_OF(SwitchTaskDialogButtonArray);
    //
    //    switch (Context->Channel)
    //    {
    //    case PhReleaseChannel:
    //        config.pszMainInstruction = L"切换到 System Informer 正式版通道?";
    //        break;
    //    //case PhPreviewChannel:
    //    //    config.pszMainInstruction = L"切换到 System Informer 预览版通道?";
    //    //    break;
    //    case PhCanaryChannel:
    //        config.pszMainInstruction = L"切换到 System Informer 测试版通道?";
    //        break;
    //    //case PhDeveloperChannel:
    //    //    config.pszMainInstruction = L"切换到 System Informer 开发者版通道?";
    //    //    break;
    //    default:
    //        config.pszMainInstruction = L"切换 System Informer 更新通道?";
    //        break;
    //    }
    //
    //    //if (Context->Channel < PhGetPhReleaseChannel())
    //    //{
    //    //    config.pszContent = L"降级通道可能会导致软件不稳定。\r\n\r\n单击“是”继续。\r\n";
    //    //}
    //    //else
    //    {
    //        config.pszContent = L"单击“是”继续。";
    //    }
    //}
    //else
    {
        config.pButtons = UpdateTaskDialogButtonArray;
        config.cButtons = RTL_NUMBER_OF(UpdateTaskDialogButtonArray);
        config.pszMainInstruction = L"检查是否有更新的 System Informer 版本?";
        config.pszContent = L"单击“检查”继续。";
    }


    PhTaskDialogNavigatePage(Context->DialogHandle, &config);
}
