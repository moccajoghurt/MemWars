#include "pch.h"
#include "common.h"

void TcPrintUsage()
{
    puts ("Usage:");
    puts ("");
    puts("    ObCallbackTestCtrl.exe -install -name NameofExe -reject NameofExe -uninstall -deprotect [-?]");
    puts("     -install        install driver");
    puts("     -uninstall      uninstall driver");
    puts("     -name NameofExe    protect/filter access to NameofExe");
    puts("     -reject NameofExe    prevents execution of NameofExe");
    puts("     -deprotect      unprotect/unfilter");
}

int _cdecl
wmain (
    _In_ int argc,
    _In_reads_(argc) LPCWSTR argv[]
)
{
    int ExitCode = ERROR_SUCCESS;

    if (argc > 1)
    {
        const wchar_t * arg = argv[1];

        // initialize globals and logging
        if (!TcInitialize()) {
            puts("Initialization failed - program exiting");
            ExitCode = ERROR_FUNCTION_FAILED;
            goto Exit;
        }

        if (0 == wcscmp (arg, L"-install")) {
            TcInstallDriver();

        } else if (0 == wcscmp (arg, L"-uninstall")) {
            TcUninstallDriver();

        } else if ((0 == wcscmp (arg, L"-?")) || (0 == wcscmp (arg, L"-h")) || (0 == wcscmp (arg, L"-help"))) {
            TcPrintUsage();

        } else if (0 == wcscmp (arg, L"-deprotect")) {
            TcRemoveProtection();
            
        } else if (0 == wcscmp (arg, L"-name")) {
            TcProcessName (argc, argv, TDProtectName_Protect);

        } else	{
			puts ("Unknown command!");
			TcPrintUsage();
        }
        
    }
    else
    {
        TcPrintUsage();
    }

Exit:

    if (!TcUnInitialize()) {
        puts("UnInitialization failed");
        ExitCode = ERROR_FUNCTION_FAILED;
    }

    return ExitCode;
}



BOOL TcRemoveProtection ()
{
    BOOL ReturnValue = FALSE;

    LOG_INFO(_T("TcRemoveProtection: Entering"));

    ReturnValue = TcOpenDevice();
    if (ReturnValue != TRUE)
    {
        LOG_INFO_FAILURE (_T("TcOpenDevice failed"));
        goto Exit;
    }

    ReturnValue = TcUnprotectCallback();
    if (ReturnValue != TRUE)
    {
        LOG_INFO_FAILURE (_T("TcUnprotectCallback failed"));
        goto Exit;
    }

Exit:

    ReturnValue = TcCloseDevice();
    if (ReturnValue != TRUE)
    {
        LOG_INFO_FAILURE (_T("TcCloseDevice failed"));
    }
   

    LOG_INFO(_T("TcRemoveProtection: Exiting"));

    return ReturnValue;
}

BOOL TcProcessName(
    _In_ int argc,
    _In_reads_(argc) LPCWSTR argv[],
    _In_ ULONG ulOperation
)
{
    BOOL ReturnValue = FALSE;

    PCWSTR pwProcessName = NULL;

    LOG_INFO(L"TcProcessName: Entering");


    //
    // Parse command line.
    //
    // argv[1] is "-name" so starting from arg #2 that should be the process name to protect
    //

    if (argc < 3) {
        LOG_INFO_FAILURE (L"TcProcessName: Too few parameters");
        LOG_INFO_FAILURE (L"TcProcessName: Usage  -name nameofExe  -reject nameofExe");
        ReturnValue = FALSE;
        goto Exit;
    }

    pwProcessName = argv[2];

    if (!pwProcessName) {
        LOG_INFO_FAILURE (L"TcProcessName: NULL process name to process");
        ReturnValue = FALSE;
        goto Exit;
    }


    LOG_INFO(L"Ready to copy process name");
    LOG_INFO(L"Name to pass to driver   %ls", pwProcessName);


    ReturnValue = TcOpenDevice();
    if (ReturnValue != TRUE)
    {
        LOG_INFO_FAILURE (L"TcProcessName: TcOpenDevice failed");
        goto Exit;
    }


    ReturnValue = TcProcessNameCallback(pwProcessName, ulOperation);
    if (ReturnValue != TRUE)
    {
        LOG_INFO_FAILURE (L"TcProcessName: TcProcessNameCallback failed");
        goto Exit;
    }

Exit:

    ReturnValue = TcCloseDevice();
    if (ReturnValue != TRUE)
    {
        LOG_INFO_FAILURE (L"TcProtectProcess: TcCloseDevice failed");
    }
   

    LOG_INFO(L"TcProtectProcess: Exiting");

    return ReturnValue;
}


BOOL TcInstallDriver ()
{
    BOOL bRC = TRUE;

    LOG_INFO(L"TcInstallDriver: Entering");
    BOOL Result = TcLoadDriver();

    if (Result != TRUE)
    {
        LOG_ERROR (L"TcLoadDriver failed, exiting");
        bRC = FALSE;
        goto Exit;
    }

Exit:

    LOG_INFO(L"TcInstallDriver: Exiting");
    return bRC;
}


BOOL TcUninstallDriver ()
{
    BOOL bRC = TRUE;

    LOG_INFO(L"TcUninstallDriver: Entering");
    BOOL Result = TcUnloadDriver();

    if (Result != TRUE)
    {
        LOG_ERROR (L"TcUnloadDriver failed, exiting");
        bRC = FALSE;
        goto Exit;
    }

Exit:

    LOG_INFO(L"TcUninstallDriver: Exiting");
    return bRC;
}

BOOL bLoggingInitialized = FALSE;

BOOL TcInitialize  ()
{

    BOOL Result = TcInitializeGlobals();
    if (Result != TRUE)
    {
        LOG_ERROR (L"TcInitializeGlobals failed, exiting");
        return FALSE;
    }

    LOG_INFO(L"TcInitialize: Entering");
    return TRUE;

}


BOOL TcUnInitialize()
{
    if (TcCleanupSCM() == FALSE){
        LOG_ERROR (L"TcUnInitialize failed cleanup of SCM");
    }
    return TRUE;
}