
#include <ntddk.h>
#include <wdf.h>

#include "driver.h"

#define TD_DRIVER_NAME             L"ObRegisterCallbacksDriver"
#define TD_DRIVER_NAME_WITH_EXT    L"ObRegisterCallbacksDriver.sys"

#define TD_NT_DEVICE_NAME          L"\\Device\\ObCallbackTest"
#define TD_DOS_DEVICES_LINK_NAME   L"\\DosDevices\\ObCallbackTest"
#define TD_WIN32_DEVICE_NAME       L"\\\\.\\ObCallbackTest"

#define NAME_SIZE 200

#define TD_IOCTL_PROTECT_NAME_CALLBACK        CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 2), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)
#define TD_IOCTL_UNPROTECT_CALLBACK           CTL_CODE (FILE_DEVICE_UNKNOWN, (0x800 + 3), METHOD_BUFFERED, FILE_SPECIAL_ACCESS)

#define CB_PROCESS_TERMINATE 0x0001
#define CB_THREAD_TERMINATE  0x0001

//#define TD_CALLBACK_REGISTRATION_TAG  '0bCO' // TD_CALLBACK_REGISTRATION structure.
#define TD_CALL_CONTEXT_TAG           '1bCO'

typedef struct _TD_PROTECTNAME_INPUT {
	ULONG Operation;
	WCHAR Name[NAME_SIZE + 1];      // what is the filename to protect - extra wchar for forced NULL
}
TD_PROTECTNAME_INPUT, *PTD_PROTECTNAME_INPUT;

typedef struct _TD_CALLBACK_PARAMETERS {
	ACCESS_MASK AccessBitsToClear;
	ACCESS_MASK AccessBitsToSet;
} TD_CALLBACK_PARAMETERS, *PTD_CALLBACK_PARAMETERS;

typedef struct _TD_CALLBACK_REGISTRATION {
	// Handle returned by ObRegisterCallbacks.
	PVOID RegistrationHandle;

	// If not NULL, filter only requests to open/duplicate handles to this
	// process (or one of its threads).
	PVOID TargetProcess;
	HANDLE TargetProcessId;

	// Currently each TD_CALLBACK_REGISTRATION has at most one process and one
	// thread callback. That is, we can't register more than one callback for
	// the same object type with a single ObRegisterCallbacks call.

	TD_CALLBACK_PARAMETERS ProcessParams;
	TD_CALLBACK_PARAMETERS ThreadParams;

	ULONG RegistrationId;        // Index in the global TdCallbacks array.
} TD_CALLBACK_REGISTRATION, *PTD_CALLBACK_REGISTRATION;

typedef struct _TD_CALL_CONTEXT {
	PTD_CALLBACK_REGISTRATION CallbackRegistration;

	OB_OPERATION Operation;
	PVOID Object;
	POBJECT_TYPE ObjectType;
} TD_CALL_CONTEXT, *PTD_CALL_CONTEXT;

BOOLEAN TdbProtectName = FALSE;
BOOLEAN bCallbacksInstalled = FALSE;

// Here is the protected process
WCHAR   TdwProtectName[NAME_SIZE + 1] = { 0 };
PVOID   TdProtectedTargetProcess = NULL;
HANDLE  TdProtectedTargetProcessId = { 0 };

UNICODE_STRING CBAltitude = { 0 };

PVOID pCBRegistrationHandle = NULL;
OB_CALLBACK_REGISTRATION CBObRegistration = { 0 };
TD_CALLBACK_REGISTRATION CBCallbackRegistration = { 0 };


OB_OPERATION_REGISTRATION CBOperationRegistrations[2] = { { 0 },{ 0 } };




void TdSetCallContext(_Inout_ POB_PRE_OPERATION_INFORMATION PreInfo,_In_ PTD_CALLBACK_REGISTRATION CallbackRegistration) {
	PTD_CALL_CONTEXT CallContext;

	CallContext = (PTD_CALL_CONTEXT)ExAllocatePoolWithTag(
		PagedPool, sizeof(TD_CALL_CONTEXT), TD_CALL_CONTEXT_TAG
	);

	if (CallContext == NULL) {
		return;
	}

	RtlZeroMemory(CallContext, sizeof(TD_CALL_CONTEXT));

	CallContext->CallbackRegistration = CallbackRegistration;
	CallContext->Operation = PreInfo->Operation;
	CallContext->Object = PreInfo->Object;
	CallContext->ObjectType = PreInfo->ObjectType;

	PreInfo->CallContext = CallContext;
}

void TdCheckAndFreeCallContext(_Inout_ POB_POST_OPERATION_INFORMATION PostInfo, _In_ PTD_CALLBACK_REGISTRATION CallbackRegistration) {
	PTD_CALL_CONTEXT CallContext = (PTD_CALL_CONTEXT)PostInfo->CallContext;

	if (CallContext != NULL) {
		NT_ASSERT(CallContext->CallbackRegistration == CallbackRegistration);

		NT_ASSERT(CallContext->Operation == PostInfo->Operation);
		NT_ASSERT(CallContext->Object == PostInfo->Object);
		NT_ASSERT(CallContext->ObjectType == PostInfo->ObjectType);

		ExFreePoolWithTag(CallContext, TD_CALL_CONTEXT_TAG);
	}
}

VOID CBTdPostOperationCallback(_In_ PVOID RegistrationContext, _In_ POB_POST_OPERATION_INFORMATION PostInfo) {
	PTD_CALLBACK_REGISTRATION CallbackRegistration = (PTD_CALLBACK_REGISTRATION)RegistrationContext;

	TdCheckAndFreeCallContext(PostInfo, CallbackRegistration);

	if (PostInfo->ObjectType == *PsProcessType) {
		// Ignore requests for processes other than our target process.

		if (CallbackRegistration->TargetProcess != NULL &&
			CallbackRegistration->TargetProcess != PostInfo->Object
			) {
			return;
		}

		// Also ignore requests that are trying to open/duplicate the current
		// process.

		if (PostInfo->Object == PsGetCurrentProcess()) {
			return;
		}
	}
	else if (PostInfo->ObjectType == *PsThreadType) {
		HANDLE ProcessIdOfTargetThread = PsGetThreadProcessId((PETHREAD)PostInfo->Object);

		// Ignore requests for threads belonging to processes other than our
		// target process.

		if (CallbackRegistration->TargetProcess != NULL &&
			CallbackRegistration->TargetProcessId != ProcessIdOfTargetThread
			) {
			return;
		}

		// Also ignore requests for threads belonging to the current processes.

		if (ProcessIdOfTargetThread == PsGetCurrentProcessId()) {
			return;
		}
	}
	else {
		NT_ASSERT(FALSE);
	}
}

OB_PREOP_CALLBACK_STATUS CBTdPreOperationCallback(_In_ PVOID RegistrationContext, _Inout_ POB_PRE_OPERATION_INFORMATION PreInfo) {
	PTD_CALLBACK_REGISTRATION CallbackRegistration;

	ACCESS_MASK AccessBitsToClear = 0;
	ACCESS_MASK AccessBitsToSet = 0;
	ACCESS_MASK InitialDesiredAccess = 0;
	ACCESS_MASK OriginalDesiredAccess = 0;


	PACCESS_MASK DesiredAccess = NULL;

	LPCWSTR ObjectTypeName = NULL;
	LPCWSTR OperationName = NULL;

	// Not using driver specific values at this time
	CallbackRegistration = (PTD_CALLBACK_REGISTRATION)RegistrationContext;


	NT_ASSERT(PreInfo->CallContext == NULL);

	// Only want to filter attempts to access protected process
	// all other processes are left untouched

	if (PreInfo->ObjectType == *PsProcessType) {
		// Ignore requests for processes other than our target process.

		// if (TdProtectedTargetProcess != NULL &&
		//    TdProtectedTargetProcess != PreInfo->Object)
		if (TdProtectedTargetProcess != PreInfo->Object) {
			goto Exit;
		}

		// Also ignore requests that are trying to open/duplicate the current
		// process.

		if (PreInfo->Object == PsGetCurrentProcess()) {
			DbgPrintEx(
				DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
				"ObCallbackTest: CBTdPreOperationCallback: ignore process open/duplicate from the protected process itself\n");
			goto Exit;
		}

		ObjectTypeName = L"PsProcessType";
		AccessBitsToClear = CB_PROCESS_TERMINATE;
		AccessBitsToSet = 0;
	}
	else if (PreInfo->ObjectType == *PsThreadType) {
		HANDLE ProcessIdOfTargetThread = PsGetThreadProcessId((PETHREAD)PreInfo->Object);

		// Ignore requests for threads belonging to processes other than our
		// target process.

		// if (CallbackRegistration->TargetProcess   != NULL &&
		//     CallbackRegistration->TargetProcessId != ProcessIdOfTargetThread)
		if (TdProtectedTargetProcessId != ProcessIdOfTargetThread) {
			goto Exit;
		}

		// Also ignore requests for threads belonging to the current processes.

		if (ProcessIdOfTargetThread == PsGetCurrentProcessId()) {
			DbgPrintEx(
				DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
				"ObCallbackTest: CBTdPreOperationCallback: ignore thread open/duplicate from the protected process itself\n");
			goto Exit;
		}

		ObjectTypeName = L"PsThreadType";
		AccessBitsToClear = CB_THREAD_TERMINATE;
		AccessBitsToSet = 0;
	}
	else {
		DbgPrintEx(
			DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
			"ObCallbackTest: CBTdPreOperationCallback: unexpected object type\n");
		goto Exit;
	}

	switch (PreInfo->Operation) {
	case OB_OPERATION_HANDLE_CREATE:
		DesiredAccess = &PreInfo->Parameters->CreateHandleInformation.DesiredAccess;
		OriginalDesiredAccess = PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess;

		OperationName = L"OB_OPERATION_HANDLE_CREATE";
		break;

	case OB_OPERATION_HANDLE_DUPLICATE:
		DesiredAccess = &PreInfo->Parameters->DuplicateHandleInformation.DesiredAccess;
		OriginalDesiredAccess = PreInfo->Parameters->DuplicateHandleInformation.OriginalDesiredAccess;

		OperationName = L"OB_OPERATION_HANDLE_DUPLICATE";
		break;

	default:
		NT_ASSERT(FALSE);
		break;
	}

	InitialDesiredAccess = *DesiredAccess;

	// Filter only if request made outside of the kernel
	if (PreInfo->KernelHandle != 1) {
		*DesiredAccess &= ~AccessBitsToClear;
		*DesiredAccess |= AccessBitsToSet;
	}

	// Set call context.

	TdSetCallContext(PreInfo, CallbackRegistration);


	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: CBTdPreOperationCallback: PROTECTED process %p (ID 0x%p)\n",
		TdProtectedTargetProcess,
		(PVOID)TdProtectedTargetProcessId
	);

	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"ObCallbackTest: CBTdPreOperationCallback\n"
		"    Client Id:    %p:%p\n"
		"    Object:       %p\n"
		"    Type:         %ls\n"
		"    Operation:    %ls (KernelHandle=%d)\n"
		"    OriginalDesiredAccess: 0x%x\n"
		"    DesiredAccess (in):    0x%x\n"
		"    DesiredAccess (out):   0x%x\n",
		PsGetCurrentProcessId(),
		PsGetCurrentThreadId(),
		PreInfo->Object,
		ObjectTypeName,
		OperationName,
		PreInfo->KernelHandle,
		OriginalDesiredAccess,
		InitialDesiredAccess,
		*DesiredAccess
	);

Exit:

	return OB_PREOP_SUCCESS;
}

NTSTATUS TdProtectNameCallback(_In_ PTD_PROTECTNAME_INPUT pProtectName) {
	NTSTATUS Status = STATUS_SUCCESS;

	if (!pProtectName) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,"ObCallbackTest: TdProtectNameCallback: name to protect/filter NULL pointer\n");
	}
	else {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,"ObCallbackTest: TdProtectNameCallback: entering name to protect/filter %ls\n", pProtectName->Name);
	}

	// Need to copy out the name and then set the flag to filter
	// This will allow process creation to watch for the process to be created and get the PID
	// and then prevent any other process from opening up that PID to terminate

	memcpy(TdwProtectName, pProtectName->Name, sizeof(TdwProtectName));

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: name copied     %ls\n", TdwProtectName);

	// Need to enable the OB callbacks
	// once the process is matched to a newly created process, the callbacks will protect the process
	if (bCallbacksInstalled == FALSE) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: TdProtectNameCallback: installing callbacks\n");

		// Setup the Ob Registration calls

		CBOperationRegistrations[0].ObjectType = PsProcessType;
		CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_CREATE;
		CBOperationRegistrations[0].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
		CBOperationRegistrations[0].PreOperation = CBTdPreOperationCallback;
		CBOperationRegistrations[0].PostOperation = CBTdPostOperationCallback;

		CBOperationRegistrations[1].ObjectType = PsThreadType;
		CBOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_CREATE;
		CBOperationRegistrations[1].Operations |= OB_OPERATION_HANDLE_DUPLICATE;
		CBOperationRegistrations[1].PreOperation = CBTdPreOperationCallback;
		CBOperationRegistrations[1].PostOperation = CBTdPostOperationCallback;


		RtlInitUnicodeString(&CBAltitude, L"1000");

		CBObRegistration.Version = OB_FLT_REGISTRATION_VERSION;
		CBObRegistration.OperationRegistrationCount = 2;
		CBObRegistration.Altitude = CBAltitude;
		CBObRegistration.RegistrationContext = &CBCallbackRegistration;
		CBObRegistration.OperationRegistration = CBOperationRegistrations;


		Status = ObRegisterCallbacks(
			&CBObRegistration,
			&pCBRegistrationHandle       // save the registration handle to remove callbacks later
		);

		if (!NT_SUCCESS(Status)) {
			DbgPrintEx(
				DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
				"ObCallbackTest: installing OB callbacks failed  status 0x%x\n", Status
			);
			goto Exit;
		}
		bCallbacksInstalled = TRUE;

	}

	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"ObCallbackTest: TdProtectNameCallback: name to protect/filter %ls\n", TdwProtectName
	);

Exit:
	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"ObCallbackTest: TdProtectNameCallback: exiting  status 0x%x\n", Status
	);
	return Status;
}

NTSTATUS TdDeleteProtectNameCallback() {
	NTSTATUS Status = STATUS_SUCCESS;

	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"ObCallbackTest: TdDeleteProtectNameCallback entering\n");


	// if the callbacks are active - remove them
	if (bCallbacksInstalled == TRUE) {
		ObUnRegisterCallbacks(pCBRegistrationHandle);
		pCBRegistrationHandle = NULL;
		bCallbacksInstalled = FALSE;
	}


	DbgPrintEx(
		DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL,
		"ObCallbackTest: TdDeleteProtectNameCallback exiting  - status 0x%x\n", Status
	);

	return Status;
}

VOID TdDeviceUnload(_In_ PDRIVER_OBJECT DriverObject) {
	NTSTATUS Status = STATUS_SUCCESS;
	UNICODE_STRING DosDevicesLinkName = RTL_CONSTANT_STRING(TD_DOS_DEVICES_LINK_NAME);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: TdDeviceUnload\n");

	// remove filtering and remove any OB callbacks
	Status = TdDeleteProtectNameCallback();
	NT_ASSERT(Status == STATUS_SUCCESS);

	Status = IoDeleteSymbolicLink(&DosDevicesLinkName);
	if (Status != STATUS_INSUFFICIENT_RESOURCES) {
		NT_ASSERT(NT_SUCCESS(Status));
	}
	IoDeleteDevice(DriverObject->DeviceObject);
}

NTSTATUS TdControlProtectName(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	NTSTATUS Status = STATUS_SUCCESS;
	PIO_STACK_LOCATION IrpStack = NULL;
	ULONG InputBufferLength = 0;
	PTD_PROTECTNAME_INPUT pProtectNameInput = NULL;

	UNREFERENCED_PARAMETER(DeviceObject);


	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: TdControlProtectName: Entering\n");

	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	InputBufferLength = IrpStack->Parameters.DeviceIoControl.InputBufferLength;

	if (InputBufferLength < sizeof(TD_PROTECTNAME_INPUT)) {
		Status = STATUS_BUFFER_OVERFLOW;
		goto Exit;
	}

	pProtectNameInput = (PTD_PROTECTNAME_INPUT)Irp->AssociatedIrp.SystemBuffer;

	Status = TdProtectNameCallback(pProtectNameInput);
	TdbProtectName = TRUE;

Exit:
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: TD_IOCTL_PROTECTNAME: Status %x\n", Status);

	return Status;
}

NTSTATUS TdControlUnprotect(IN PDEVICE_OBJECT  DeviceObject, IN PIRP  Irp) {
	NTSTATUS Status = STATUS_SUCCESS;

	UNREFERENCED_PARAMETER(DeviceObject);
	UNREFERENCED_PARAMETER(Irp);

	// do not filter requested access
	Status = TdDeleteProtectNameCallback();
	if (Status != STATUS_SUCCESS) {
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: TdDeleteProtectNameCallback:  status 0x%x\n", Status);
	}
	TdbProtectName = FALSE;

	//Exit:
	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: TD_IOCTL_UNPROTECT: exiting - status 0x%x\n", Status);

	return Status;
}

NTSTATUS TdDeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP  Irp) {
	PIO_STACK_LOCATION IrpStack;
	ULONG Ioctl;
	NTSTATUS Status;

	UNREFERENCED_PARAMETER(DeviceObject);

	Status = STATUS_SUCCESS;

	IrpStack = IoGetCurrentIrpStackLocation(Irp);
	Ioctl = IrpStack->Parameters.DeviceIoControl.IoControlCode;

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TdDeviceControl: entering - ioctl code 0x%x\n", Ioctl);

	switch (Ioctl) {
	case TD_IOCTL_PROTECT_NAME_CALLBACK:

		Status = TdControlProtectName(DeviceObject, Irp);
		break;

	case TD_IOCTL_UNPROTECT_CALLBACK:

		Status = TdControlUnprotect(DeviceObject, Irp);
		break;

	default:
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TdDeviceControl: unrecognized ioctl code 0x%x\n", Ioctl);
		break;
	}

	Irp->IoStatus.Status = Status;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "TdDeviceControl leaving - status 0x%x\n", Status);
	return Status;
}

NTSTATUS TdDeviceCreate(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS TdDeviceClose(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS TdDeviceCleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp) {
	UNREFERENCED_PARAMETER(DeviceObject);
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
	NTSTATUS Status;
	UNICODE_STRING NtDeviceName = RTL_CONSTANT_STRING(TD_NT_DEVICE_NAME);
	UNICODE_STRING DosDevicesLinkName = RTL_CONSTANT_STRING(TD_DOS_DEVICES_LINK_NAME);
	PDEVICE_OBJECT Device = NULL;
	BOOLEAN SymLinkCreated = FALSE;
	USHORT CallbackVersion;

	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "ObCallbackTest: DriverEntry: Driver loaded.");

	CallbackVersion = ObGetFilterVersion();

	DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, "ObCallbackTest: DriverEntry: Callback version 0x%hx\n", CallbackVersion);

	// Create our device object.

	Status = IoCreateDevice(
		DriverObject,                 // pointer to driver object
		0,                            // device extension size
		&NtDeviceName,                // device name
		FILE_DEVICE_UNKNOWN,          // device type
		0,                            // device characteristics
		FALSE,                        // not exclusive
		&Device);                     // returned device object pointer

	if (!NT_SUCCESS(Status)) {
		goto Exit;
	}

	NT_ASSERT(Device == DriverObject->DeviceObject);

	DriverObject->MajorFunction[IRP_MJ_CREATE] = TdDeviceCreate;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = TdDeviceClose;
	DriverObject->MajorFunction[IRP_MJ_CLEANUP] = TdDeviceCleanup;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = TdDeviceControl;
	DriverObject->DriverUnload = TdDeviceUnload;

	Status = IoCreateSymbolicLink(&DosDevicesLinkName, &NtDeviceName);

	if (!NT_SUCCESS(Status)) {
		goto Exit;
	}

	SymLinkCreated = TRUE;

Exit:
	if (!NT_SUCCESS(Status)) {
		if (SymLinkCreated == TRUE) {
			IoDeleteSymbolicLink(&DosDevicesLinkName);
		}

		if (Device != NULL) {
			IoDeleteDevice(Device);
		}
	}

	return Status;
}