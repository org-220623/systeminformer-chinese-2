/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     jxy-s   2022
 *     dmex    2022-2023
 *
 */

#ifndef PH_KSISUP_H
#define PH_KSISUP_H

VOID
PhShowKsiStatus(
    VOID
    );

VOID
PhInitializeKsi(
    VOID
    );

NTSTATUS
PhCleanupKsi(
    VOID
    );

PPH_STRING
PhGetKsiMessage(
    _In_opt_ NTSTATUS Status,
    _In_ BOOLEAN Force,
    _In_ PCWSTR Format,
    ...
    );

LONG
PhShowKsiMessage2(
    _In_opt_ HWND WindowHandle,
    _In_ ULONG Buttons,
    _In_opt_ PCWSTR Icon,
    _In_opt_ NTSTATUS Status,
    _In_ BOOLEAN Force,
    _In_ PCWSTR Title,
    _In_ PCWSTR Format,
    ...
    );

VOID
PhShowKsiMessageEx(
    _In_opt_ HWND WindowHandle,
    _In_opt_ PCWSTR Icon,
    _In_opt_ NTSTATUS Status,
    _In_ BOOLEAN Force,
    _In_ PCWSTR Title,
    _In_ PCWSTR Format,
    ...
    );

VOID
PhShowKsiMessage(
    _In_opt_ HWND WindowHandle,
    _In_opt_ PCWSTR Icon,
    _In_ PCWSTR Title,
    _In_ PCWSTR Format,
    ...
    );

PWSTR PhKsiStandardHelpText(
    VOID
    );

FORCEINLINE
VOID
PhShowKsiNotConnected(
    _In_opt_ HWND WindowHandle,
    _In_ PWSTR Message
    )
{
    PhShowKsiMessage(
        WindowHandle,
        TD_INFORMATION_ICON,
        L"内核驱动程序未连接",
        L"%s\r\n\r\n"
        L"System Informer 未连接到内核驱动程序，或缺少此功能所需的必要状态。请确保已启用“启用内核模式驱动程序”选项，并且 System Informer 以管理员权限运行。",
        Message
        );
}

FORCEINLINE
PPH_STRING
PhGetKsiNotConnectedString(
    _In_ PWSTR Message
    )
{
    return PhGetKsiMessage(
        0,
        FALSE,
        L"%s\r\n\r\n%s\r\n\r\n%s",
        L"内核驱动程序未连接",
        L"System Informer 未连接到内核驱动程序，或缺少此功能所需的必要状态。请确保已启用“启用内核模式驱动程序”选项，并且 System Informer 以管理员权限运行。",
        Message
        );
}

#endif
