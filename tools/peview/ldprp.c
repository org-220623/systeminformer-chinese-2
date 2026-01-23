/*
 * Copyright (c) 2022 Winsider Seminars & Solutions, Inc.  All rights reserved.
 *
 * This file is part of System Informer.
 *
 * Authors:
 *
 *     wj32    2010-2011
 *     dmex    2017-2022
 *
 */

#include <peview.h>

typedef struct _PV_PE_LOADCONFIG_CONTEXT
{
    HWND WindowHandle;
    HWND ListViewHandle;
    PH_LAYOUT_MANAGER LayoutManager;
    PPV_PROPPAGECONTEXT PropSheetContext;
} PV_PE_LOADCONFIG_CONTEXT, *PPV_PE_LOADCONFIG_CONTEXT;

#define ADD_VALUE(Name, Value) \
{ \
    INT lvItemIndex; \
    lvItemIndex = PhAddListViewItem(lvHandle, MAXINT, Name, NULL); \
    PhSetListViewSubItem(lvHandle, lvItemIndex, 1, Value); \
}

PPH_STRING PvpGetPeGuardFlagsText(
    _In_ ULONG GuardFlags
    )
{
    PH_STRING_BUILDER stringBuilder;
    WCHAR pointer[PH_PTR_STR_LEN_1];

    if (GuardFlags == 0)
        return PhCreateString(L"0x0");

    PhInitializeStringBuilder(&stringBuilder, 10);

    if (GuardFlags & IMAGE_GUARD_CF_INSTRUMENTED)
        PhAppendStringBuilder2(&stringBuilder, L"已检测, ");
    if (GuardFlags & IMAGE_GUARD_CFW_INSTRUMENTED)
        PhAppendStringBuilder2(&stringBuilder, L"已检测 (写入), ");
    if (GuardFlags & IMAGE_GUARD_CF_FUNCTION_TABLE_PRESENT)
        PhAppendStringBuilder2(&stringBuilder, L"函数表, ");
    if (GuardFlags & IMAGE_GUARD_SECURITY_COOKIE_UNUSED)
        PhAppendStringBuilder2(&stringBuilder, L"未使用的安全 Cookie, ");
    if (GuardFlags & IMAGE_GUARD_PROTECT_DELAYLOAD_IAT)
        PhAppendStringBuilder2(&stringBuilder, L"延迟加载已保护 IAT, ");
    if (GuardFlags & IMAGE_GUARD_DELAYLOAD_IAT_IN_ITS_OWN_SECTION)
        PhAppendStringBuilder2(&stringBuilder, L"延迟加载私有节区, ");
    if (GuardFlags & IMAGE_GUARD_CF_ENABLE_EXPORT_SUPPRESSION)
        PhAppendStringBuilder2(&stringBuilder, L"导出抑制, ");
    if (GuardFlags & IMAGE_GUARD_CF_EXPORT_SUPPRESSION_INFO_PRESENT)
        PhAppendStringBuilder2(&stringBuilder, L"导出信息抑制, ");
    if (GuardFlags & IMAGE_GUARD_CF_LONGJUMP_TABLE_PRESENT)
        PhAppendStringBuilder2(&stringBuilder, L"Longjump 表, ");
    if (GuardFlags & IMAGE_GUARD_RETPOLINE_PRESENT)
        PhAppendStringBuilder2(&stringBuilder, L"存在 Retpoline, ");
    if (GuardFlags & IMAGE_GUARD_EH_CONTINUATION_TABLE_PRESENT_V1 || GuardFlags & IMAGE_GUARD_EH_CONTINUATION_TABLE_PRESENT_V2)
        PhAppendStringBuilder2(&stringBuilder, L"异常处理延续表, ");
    if (GuardFlags & IMAGE_GUARD_XFG_ENABLED)
        PhAppendStringBuilder2(&stringBuilder, L"XFG, ");
    if (GuardFlags & IMAGE_GUARD_CASTGUARD_PRESENT)
        PhAppendStringBuilder2(&stringBuilder, L"类型转换防护, ");
    if (GuardFlags & IMAGE_GUARD_MEMCPY_PRESENT)
        PhAppendStringBuilder2(&stringBuilder, L"安全 memcpy, ");

    if (PhEndsWithString2(stringBuilder.String, L", ", FALSE))
        PhRemoveEndStringBuilder(&stringBuilder, 2);

    PhPrintPointer(pointer, UlongToPtr(GuardFlags));
    PhAppendFormatStringBuilder(&stringBuilder, L" (%s)", pointer);

    return PhFinalStringBuilderString(&stringBuilder);
}

PPH_STRING PvpGetPeDependentLoadFlagsText(
    _In_ ULONG DependentLoadFlags
    )
{
    PH_STRING_BUILDER stringBuilder;
    WCHAR pointer[PH_PTR_STR_LEN_1];

    if (DependentLoadFlags == 0)
        return PhCreateString(L"0x0");

    PhInitializeStringBuilder(&stringBuilder, 10);

    if (DependentLoadFlags & DONT_RESOLVE_DLL_REFERENCES)
        PhAppendStringBuilder2(&stringBuilder, L"忽略 DLL 引用, ");
    if (DependentLoadFlags & LOAD_LIBRARY_AS_DATAFILE)
        PhAppendStringBuilder2(&stringBuilder, L"数据文件, ");
    if (DependentLoadFlags & 0x00000004) // LOAD_PACKAGED_LIBRARY
        PhAppendStringBuilder2(&stringBuilder, L"已打包的库, ");
    if (DependentLoadFlags & LOAD_WITH_ALTERED_SEARCH_PATH)
        PhAppendStringBuilder2(&stringBuilder, L"已更改搜索路径, ");
    if (DependentLoadFlags & LOAD_IGNORE_CODE_AUTHZ_LEVEL)
        PhAppendStringBuilder2(&stringBuilder, L"忽略 AuthZ 级别, ");
    if (DependentLoadFlags & LOAD_LIBRARY_AS_IMAGE_RESOURCE)
        PhAppendStringBuilder2(&stringBuilder, L"映像资源, ");
    if (DependentLoadFlags & LOAD_LIBRARY_AS_DATAFILE_EXCLUSIVE)
        PhAppendStringBuilder2(&stringBuilder, L"数据文件 (独占), ");
    if (DependentLoadFlags & LOAD_LIBRARY_REQUIRE_SIGNED_TARGET)
        PhAppendStringBuilder2(&stringBuilder, L"需要签名目标, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR)
        PhAppendStringBuilder2(&stringBuilder, L"搜索 DLL 加载目录, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SEARCH_APPLICATION_DIR)
        PhAppendStringBuilder2(&stringBuilder, L"搜索应用程序目录, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SEARCH_USER_DIRS)
        PhAppendStringBuilder2(&stringBuilder, L"搜索用户目录, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SEARCH_SYSTEM32)
        PhAppendStringBuilder2(&stringBuilder, L"搜索 System32 目录, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SEARCH_DEFAULT_DIRS)
        PhAppendStringBuilder2(&stringBuilder, L"搜索默认目录, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SAFE_CURRENT_DIRS)
        PhAppendStringBuilder2(&stringBuilder, L"搜索当前安全目录, ");
    if (DependentLoadFlags & LOAD_LIBRARY_SEARCH_SYSTEM32_NO_FORWARDER)
        PhAppendStringBuilder2(&stringBuilder, L"搜索 system32 (无转发器), ");
    if (DependentLoadFlags & LOAD_LIBRARY_OS_INTEGRITY_CONTINUITY)
        PhAppendStringBuilder2(&stringBuilder, L"OS 完整性持续性, ");

    if (PhEndsWithString2(stringBuilder.String, L", ", FALSE))
        PhRemoveEndStringBuilder(&stringBuilder, 2);

    PhPrintPointer(pointer, UlongToPtr(DependentLoadFlags));
    PhAppendFormatStringBuilder(&stringBuilder, L" (%s)", pointer);

    return PhFinalStringBuilderString(&stringBuilder);
}

PPH_STRING PvpGetPeEnclaveImportsText(
    _In_ PVOID EnclaveConfig
    )
{
    PH_STRING_BUILDER stringBuilder;
    ULONG i;

    PhInitializeStringBuilder(&stringBuilder, 10);

    if (PvMappedImage.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        PIMAGE_ENCLAVE_CONFIG32 enclaveConfig32 = EnclaveConfig;
        PIMAGE_ENCLAVE_IMPORT enclaveImports;

        enclaveImports = PhMappedImageRvaToVa(
            &PvMappedImage,
            enclaveConfig32->ImportList,
            NULL
            );

        for (i = 0; i < enclaveConfig32->NumberOfImports; i++)
        {
            PCSTR importName;

            if (!enclaveImports || enclaveImports->ImportName == USHRT_MAX)
                break;

            if (importName = PhMappedImageRvaToVa(
                &PvMappedImage,
                enclaveImports->ImportName,
                NULL
                ))
            {
                PhAppendFormatStringBuilder(&stringBuilder, L"%hs, ", importName);
            }

            enclaveImports++;
        }
    }
    else
    {
        PIMAGE_ENCLAVE_CONFIG64 enclaveConfig64 = EnclaveConfig;
        PIMAGE_ENCLAVE_IMPORT enclaveImports;

        enclaveImports = PhMappedImageRvaToVa(
            &PvMappedImage,
            enclaveConfig64->ImportList,
            NULL
            );

        for (i = 0; i < enclaveConfig64->NumberOfImports; i++)
        {
            PCSTR importName;

            if (!enclaveImports || enclaveImports->ImportName == USHRT_MAX)
                break;

            if (importName = PhMappedImageRvaToVa(
                &PvMappedImage,
                enclaveImports->ImportName,
                NULL
                ))
            {
                PhAppendFormatStringBuilder(&stringBuilder, L"%hs, ", importName);
            }

            enclaveImports++;
        }
    }

    if (PhEndsWithString2(stringBuilder.String, L", ", FALSE))
        PhRemoveEndStringBuilder(&stringBuilder, 2);

    return PhFinalStringBuilderString(&stringBuilder);
}

VOID PvpAddPeEnclaveConfig(
    _In_ PVOID ImageConfig,
    _In_ HWND lvHandle
    )
{
    if (PvMappedImage.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
    {
        PIMAGE_LOAD_CONFIG_DIRECTORY32 imageConfig32 = ImageConfig;
        PIMAGE_ENCLAVE_CONFIG32 enclaveConfig;

        if (!RTL_CONTAINS_FIELD(imageConfig32, imageConfig32->Size, EnclaveConfigurationPointer))
            return;

        enclaveConfig = PhMappedImageVaToVa(
            &PvMappedImage,
            (ULONG)imageConfig32->EnclaveConfigurationPointer,
            NULL
            );

        if (enclaveConfig)
        {
            ADD_VALUE(L"安全 PolicyFlags", PhaFormatUInt64(enclaveConfig->PolicyFlags, TRUE)->Buffer);
            ADD_VALUE(L"安全 FamilyID", PH_AUTO_T(PH_STRING, PhFormatGuid((PGUID)enclaveConfig->FamilyID))->Buffer);
            ADD_VALUE(L"安全 ImageID", PH_AUTO_T(PH_STRING, PhFormatGuid((PGUID)enclaveConfig->ImageID))->Buffer);
            ADD_VALUE(L"安全 ImageVersion", PhaFormatUInt64(enclaveConfig->ImageVersion, TRUE)->Buffer);
            ADD_VALUE(L"安全 SecurityVersion", PhaFormatUInt64(enclaveConfig->SecurityVersion, TRUE)->Buffer);
            ADD_VALUE(L"安全 EnclaveSize", PhaFormatUInt64(enclaveConfig->EnclaveSize, TRUE)->Buffer);
            ADD_VALUE(L"安全 NumberOfThreads", PhaFormatUInt64(enclaveConfig->NumberOfThreads, TRUE)->Buffer);
            ADD_VALUE(L"安全 EnclaveFlags", PhaFormatUInt64(enclaveConfig->EnclaveFlags, TRUE)->Buffer);
            ADD_VALUE(L"安全 NumberOfImports", PhaFormatUInt64(enclaveConfig->NumberOfImports, TRUE)->Buffer);
            ADD_VALUE(L"安全 Imports", PH_AUTO_T(PH_STRING, PvpGetPeEnclaveImportsText(enclaveConfig))->Buffer);
        }
    }
    else
    {
        PIMAGE_LOAD_CONFIG_DIRECTORY64 imageConfig64 = ImageConfig;
        PIMAGE_ENCLAVE_CONFIG64 enclaveConfig;

        if (!RTL_CONTAINS_FIELD(imageConfig64, imageConfig64->Size, EnclaveConfigurationPointer))
            return;

        enclaveConfig = PhMappedImageVaToVa(
            &PvMappedImage,
            (ULONG)imageConfig64->EnclaveConfigurationPointer,
            NULL
            );

        if (enclaveConfig)
        {
            ADD_VALUE(L"安全 PolicyFlags", PhaFormatUInt64(enclaveConfig->PolicyFlags, TRUE)->Buffer);
            ADD_VALUE(L"安全 FamilyID", PH_AUTO_T(PH_STRING, PhFormatGuid((PGUID)enclaveConfig->FamilyID))->Buffer);
            ADD_VALUE(L"安全 ImageID", PH_AUTO_T(PH_STRING, PhFormatGuid((PGUID)enclaveConfig->ImageID))->Buffer);
            ADD_VALUE(L"安全 ImageVersion", PhaFormatUInt64(enclaveConfig->ImageVersion, TRUE)->Buffer);
            ADD_VALUE(L"安全 SecurityVersion", PhaFormatUInt64(enclaveConfig->SecurityVersion, TRUE)->Buffer);
            ADD_VALUE(L"安全 EnclaveSize", PhaFormatUInt64(enclaveConfig->EnclaveSize, TRUE)->Buffer);
            ADD_VALUE(L"安全 NumberOfThreads", PhaFormatUInt64(enclaveConfig->NumberOfThreads, TRUE)->Buffer);
            ADD_VALUE(L"安全 EnclaveFlags", PhaFormatUInt64(enclaveConfig->EnclaveFlags, TRUE)->Buffer);
            ADD_VALUE(L"安全 NumberOfImports", PhaFormatUInt64(enclaveConfig->NumberOfImports, TRUE)->Buffer);
            ADD_VALUE(L"安全 Imports", PH_AUTO_T(PH_STRING, PvpGetPeEnclaveImportsText(enclaveConfig))->Buffer);
        }
    }
}

INT_PTR CALLBACK PvPeLoadConfigDlgProc(
    _In_ HWND hwndDlg,
    _In_ UINT uMsg,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam
    )
{
    PPV_PE_LOADCONFIG_CONTEXT context;

    if (uMsg == WM_INITDIALOG)
    {
        context = PhAllocateZero(sizeof(PV_PE_LOADCONFIG_CONTEXT));
        PhSetWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT, context);

        if (lParam)
        {
            LPPROPSHEETPAGE propSheetPage = (LPPROPSHEETPAGE)lParam;
            context->PropSheetContext = (PPV_PROPPAGECONTEXT)propSheetPage->lParam;
        }
    }
    else
    {
        context = PhGetWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT);
    }

    if (!context)
        return FALSE;

    switch (uMsg)
    {
    case WM_INITDIALOG:
        {
            PIMAGE_LOAD_CONFIG_DIRECTORY32 config32;
            PIMAGE_LOAD_CONFIG_DIRECTORY64 config64;
            HWND lvHandle;

            context->WindowHandle = hwndDlg;
            context->ListViewHandle = lvHandle = GetDlgItem(hwndDlg, IDC_LIST);

            PhSetListViewStyle(context->ListViewHandle, TRUE, TRUE);
            PhSetControlTheme(context->ListViewHandle, L"explorer");
            PhAddListViewColumn(context->ListViewHandle, 0, 0, 0, LVCFMT_LEFT, 220, L"名称");
            PhAddListViewColumn(context->ListViewHandle, 1, 1, 1, LVCFMT_LEFT, 170, L"值");
            PhSetExtendedListView(context->ListViewHandle);
            PhLoadListViewColumnsFromSetting(L"ImageLoadCfgListViewColumns", context->ListViewHandle);
            PvConfigTreeBorders(context->ListViewHandle);

            PhInitializeLayoutManager(&context->LayoutManager, hwndDlg);
            PhAddLayoutItem(&context->LayoutManager, context->ListViewHandle, NULL, PH_ANCHOR_ALL);

            #define ADD_VALUES(Type, Config) \
            { \
                LARGE_INTEGER time; \
                SYSTEMTIME systemTime; \
                \
                PhSecondsSince1970ToTime((Config)->TimeDateStamp, &time); \
                PhLargeIntegerToLocalSystemTime(&systemTime, &time); \
                \
                ADD_VALUE(L"时间戳", (Config)->TimeDateStamp ? PhaFormatDateTime(&systemTime)->Buffer : L"0"); \
                ADD_VALUE(L"版本", PhaFormatString(L"%u.%u", (Config)->MajorVersion, (Config)->MinorVersion)->Buffer); \
                ADD_VALUE(L"需清除的全局标志", PhaFormatString(L"0x%x", (Config)->GlobalFlagsClear)->Buffer); \
                ADD_VALUE(L"需设置的全局标志", PhaFormatString(L"0x%x", (Config)->GlobalFlagsSet)->Buffer); \
                ADD_VALUE(L"临界区默认超时", PhaFormatUInt64((Config)->CriticalSectionDefaultTimeout, TRUE)->Buffer); \
                ADD_VALUE(L"取消提交空闲块阈值", PhaFormatUInt64((Config)->DeCommitFreeBlockThreshold, TRUE)->Buffer); \
                ADD_VALUE(L"取消提交总空闲阈值", PhaFormatUInt64((Config)->DeCommitTotalFreeThreshold, TRUE)->Buffer); \
                ADD_VALUE(L"锁前缀表", PhaFormatString(L"0x%x", (Config)->LockPrefixTable)->Buffer); \
                ADD_VALUE(L"最大分配大小", PhaFormatString(L"0x%Ix", (Config)->MaximumAllocationSize)->Buffer); \
                ADD_VALUE(L"虚拟内存阈值", PhaFormatString(L"0x%Ix", (Config)->VirtualMemoryThreshold)->Buffer); \
                ADD_VALUE(L"进程堆标志", PhaFormatString(L"0x%Ix", (Config)->ProcessHeapFlags)->Buffer); \
                ADD_VALUE(L"进程处理器相关性掩码", PhaFormatString(L"0x%Ix", (Config)->ProcessAffinityMask)->Buffer); \
                ADD_VALUE(L"CSD 版本", PhaFormatString(L"%u", (Config)->CSDVersion)->Buffer); \
                ADD_VALUE(L"依赖加载标志", PH_AUTO_T(PH_STRING, PvpGetPeDependentLoadFlagsText((Config)->DependentLoadFlags))->Buffer); \
                ADD_VALUE(L"编辑列表", PhaFormatString(L"0x%Ix", (Config)->EditList)->Buffer); \
                ADD_VALUE(L"安全 Cookie", PhaFormatString(L"0x%Ix", (Config)->SecurityCookie)->Buffer); \
                ADD_VALUE(L"SEH 处理程序表", PhaFormatString(L"0x%Ix", (Config)->SEHandlerTable)->Buffer); \
                ADD_VALUE(L"SEH 处理程序计数", PhaFormatUInt64((Config)->SEHandlerCount, TRUE)->Buffer); \
                \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, GuardCFCheckFunctionPointer)) \
                { \
                    ADD_VALUE(L"控制流防护标志", PH_AUTO_T(PH_STRING, PvpGetPeGuardFlagsText((Config)->GuardFlags))->Buffer); \
                    ADD_VALUE(L"CFG 检查函数指针", PhaFormatString(L"0x%Ix", (Config)->GuardCFCheckFunctionPointer)->Buffer); \
                    ADD_VALUE(L"CFG 分发函数指针", PhaFormatString(L"0x%Ix", (Config)->GuardCFDispatchFunctionPointer)->Buffer); \
                    ADD_VALUE(L"CFG 函数表", PhaFormatString(L"0x%Ix", (Config)->GuardCFFunctionTable)->Buffer); \
                    ADD_VALUE(L"CFG 函数表计数", PhaFormatUInt64((Config)->GuardCFFunctionCount, TRUE)->Buffer); \
                    if (RTL_CONTAINS_FIELD((Config), (Config)->Size, GuardAddressTakenIatEntryTable)) \
                    { \
                        ADD_VALUE(L"CFG IAT 条目表", PhaFormatString(L"0x%Ix", (Config)->GuardAddressTakenIatEntryTable)->Buffer); \
                        ADD_VALUE(L"CFG IAT 条目计数", PhaFormatUInt64((Config)->GuardAddressTakenIatEntryCount, TRUE)->Buffer); \
                    } \
                    if (RTL_CONTAINS_FIELD((Config), (Config)->Size, GuardLongJumpTargetTable)) \
                    { \
                        ADD_VALUE(L"CFG LongJump 表", PhaFormatString(L"0x%Ix", (Config)->GuardLongJumpTargetTable)->Buffer); \
                        ADD_VALUE(L"CFG LongJump 表条目计数", PhaFormatUInt64((Config)->GuardLongJumpTargetCount, TRUE)->Buffer); \
                    } \
                } \
                \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, CodeIntegrity)) \
                { \
                    ADD_VALUE(L"CI 标志", PhaFormatString(L"0x%x", (Config)->CodeIntegrity.Flags)->Buffer); \
                    ADD_VALUE(L"CI 目录", PhaFormatString(L"0x%Ix", (Config)->CodeIntegrity.Catalog)->Buffer); \
                    ADD_VALUE(L"CI 目录偏移", PhaFormatString(L"0x%Ix", (Config)->CodeIntegrity.CatalogOffset)->Buffer); \
                } \
                \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, DynamicValueRelocTable)) \
                { \
                    ADD_VALUE(L"动态值重定位表", PhaFormatString(L"0x%Ix", (Config)->DynamicValueRelocTable)->Buffer); \
                    ADD_VALUE(L"混合元数据指针", PhaFormatString(L"0x%Ix", (Config)->CHPEMetadataPointer)->Buffer); \
                    ADD_VALUE(L"GuardRF 错误处理例程", PhaFormatString(L"0x%Ix", (Config)->GuardRFFailureRoutine)->Buffer); \
                    ADD_VALUE(L"GuardRF 错误处理例程指针", PhaFormatString(L"0x%Ix", (Config)->GuardRFFailureRoutineFunctionPointer)->Buffer); \
                } \
                \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, DynamicValueRelocTableOffset)) \
                { \
                    ADD_VALUE(L"动态值重定位表偏移", PhaFormatString(L"0x%Ix", (Config)->DynamicValueRelocTableOffset)->Buffer); \
                    ADD_VALUE(L"动态值重定位节区", PhaFormatString(L"%u", (Config)->DynamicValueRelocTableSection)->Buffer); \
                    ADD_VALUE(L"GuardRF 验证堆栈指针", PhaFormatString(L"0x%Ix", (Config)->GuardRFVerifyStackPointerFunctionPointer)->Buffer); \
                    ADD_VALUE(L"热修补表偏移", PhaFormatString(L"0x%Ix", (Config)->HotPatchTableOffset)->Buffer); \
                    ADD_VALUE(L"Enclave 配置指针", PhaFormatString(L"0x%Ix", (Config)->EnclaveConfigurationPointer)->Buffer); \
                } \
                \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, VolatileMetadataPointer)) \
                { \
                    ADD_VALUE(L"易失性元数据指针", PhaFormatString(L"0x%Ix", (Config)->VolatileMetadataPointer)->Buffer); \
                } \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, GuardEHContinuationTable)) \
                { \
                    ADD_VALUE(L"防护异常处理延续表", PhaFormatString(L"0x%Ix", (Config)->GuardEHContinuationTable)->Buffer); \
                    ADD_VALUE(L"防护异常处理延续表条目计数", PhaFormatUInt64((Config)->GuardEHContinuationCount, TRUE)->Buffer); \
                } \
            }

            #define ADD_VALUES2(Type, Config) \
            { \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, GuardXFGCheckFunctionPointer)) \
                { \
                    ADD_VALUE(L"XFG 检查函数指针", PhaFormatString(L"0x%Ix", (Config)->GuardXFGCheckFunctionPointer)->Buffer); \
                    ADD_VALUE(L"XFG 分发函数指针", PhaFormatString(L"0x%Ix", (Config)->GuardXFGDispatchFunctionPointer)->Buffer); \
                    ADD_VALUE(L"XFG 表分发函数指针", PhaFormatString(L"0x%Ix", (Config)->GuardXFGTableDispatchFunctionPointer)->Buffer); \
                } \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, CastGuardOsDeterminedFailureMode)) \
                { \
                    ADD_VALUE(L"类型转换防护错误模式", PhaFormatString(L"0x%Ix", (Config)->CastGuardOsDeterminedFailureMode)->Buffer); \
                } \
            }

            #define ADD_VALUES3(Type, Config) \
            { \
                if (RTL_CONTAINS_FIELD((Config), (Config)->Size, GuardMemcpyFunctionPointer)) \
                { \
                    ADD_VALUE(L"安全 memcpy 函数指针", PhaFormatString(L"0x%Ix", (Config)->GuardMemcpyFunctionPointer)->Buffer); \
                } \
            }

            if (PvMappedImage.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
            {
                if (NT_SUCCESS(PhGetMappedImageLoadConfig32(&PvMappedImage, &config32)))
                {
                    ADD_VALUES(IMAGE_LOAD_CONFIG_DIRECTORY32, config32);
                #if defined(NTDDI_WIN10_CO) && (NTDDI_VERSION >= NTDDI_WIN10_CO)
                    ADD_VALUES2(IMAGE_LOAD_CONFIG_DIRECTORY32, config32);
                #endif
                #if defined(NTDDI_WIN10_NI) && (NTDDI_VERSION >= NTDDI_WIN10_NI)
                    ADD_VALUES3(IMAGE_LOAD_CONFIG_DIRECTORY32, config32);
                #endif
                    PvpAddPeEnclaveConfig(config32, context->ListViewHandle);
                }
            }
            else
            {
                if (NT_SUCCESS(PhGetMappedImageLoadConfig64(&PvMappedImage, &config64)))
                {
                    ADD_VALUES(IMAGE_LOAD_CONFIG_DIRECTORY64, config64);
                #if defined(NTDDI_WIN10_CO) && (NTDDI_VERSION >= NTDDI_WIN10_CO)
                    ADD_VALUES2(IMAGE_LOAD_CONFIG_DIRECTORY64, config64);
                #endif
                #if defined(NTDDI_WIN10_NI) && (NTDDI_VERSION >= NTDDI_WIN10_NI)
                    ADD_VALUES3(IMAGE_LOAD_CONFIG_DIRECTORY64, config64);
                #endif
                    PvpAddPeEnclaveConfig(config64, context->ListViewHandle);
                }
            }

            PhInitializeWindowTheme(hwndDlg, PhEnableThemeSupport);
        }
        break;
    case WM_DESTROY:
        {
            PhSaveListViewColumnsToSetting(L"ImageLoadCfgListViewColumns", context->ListViewHandle);
            PhRemoveWindowContext(hwndDlg, PH_WINDOW_CONTEXT_DEFAULT);
            PhFree(context);
        }
        break;
    case WM_SHOWWINDOW:
        {
            if (context->PropSheetContext && !context->PropSheetContext->LayoutInitialized)
            {
                PvAddPropPageLayoutItem(hwndDlg, hwndDlg, PH_PROP_PAGE_TAB_CONTROL_PARENT, PH_ANCHOR_ALL);
                PvDoPropPageLayout(hwndDlg);

                context->PropSheetContext->LayoutInitialized = TRUE;
            }
        }
        break;
    case WM_SIZE:
        {
            PhLayoutManagerLayout(&context->LayoutManager);
        }
        break;
    case WM_NOTIFY:
        {
            PvHandleListViewNotifyForCopy(lParam, context->ListViewHandle);
        }
        break;
    case WM_CONTEXTMENU:
        {
            PvHandleListViewCommandCopy(hwndDlg, lParam, wParam, context->ListViewHandle);
        }
        break;
    case WM_CTLCOLORBTN:
    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORLISTBOX:
        {
            SetBkMode((HDC)wParam, TRANSPARENT);
            SetTextColor((HDC)wParam, RGB(0, 0, 0));
            SetDCBrushColor((HDC)wParam, RGB(255, 255, 255));
            return (INT_PTR)PhGetStockBrush(DC_BRUSH);
        }
        break;
    }

    return FALSE;
}
