
//
// Write the inputfile in consecutive blocks onto the drive
// 
// WARNING:
// 		USE AT OWN RISK
// 		NO WARRANTY OF ANY KIND
// 		This will corrupt any data on destination drive !
// 

#include <windows.h>
#include <stdio.h>
#include <winioctl.h>

#define START_BLOCK			1024
#define BLOCKS_PER_FRAME	8

double
GetDriveCapacity(LPWSTR wszPath)
{
	DISK_GEOMETRY pdg = { 0 }; // disk drive geometry structure

    HANDLE hDevice = CreateFileW(wszPath,          // drive to open
			0,                // no access to the drive
			FILE_SHARE_READ | // share mode
			FILE_SHARE_WRITE, 
			NULL,             // default security attributes
			OPEN_EXISTING,    // disposition
			0,                // file attributes
			NULL);            // do not copy file attributes

	if (hDevice != INVALID_HANDLE_VALUE)    // cannot open the drive
	{
		DWORD junk     = 0;                     // discard results
		BOOL bResult = DeviceIoControl(hDevice,	// device to be queried
				IOCTL_DISK_GET_DRIVE_GEOMETRY,	// operation to perform
				NULL, 0,						// no input buffer
				&pdg, sizeof(pdg),				// output buffer
				&junk,							// # bytes returned
				(LPOVERLAPPED) NULL);			// synchronous I/O

		CloseHandle(hDevice);

		if (bResult) 
		{
			ULONGLONG DiskSize = 
				pdg.Cylinders.QuadPart * (ULONG)pdg.TracksPerCylinder * (ULONG)pdg.SectorsPerTrack * (ULONG)pdg.BytesPerSector;

			double gigs = (double) DiskSize / (1024 * 1024 * 1024);

			return gigs;
		} 
	}

	return -1;
}

#define NUM_DRIVE_TYPES		7

wchar_t*
driveTypes[NUM_DRIVE_TYPES] =
{
	L"UNKNOWN    ",
	L"NO_ROOT_DIR",
	L"REMOVABLE  ",
	L"FIXED      ",
	L"REMOTE     ",
	L"CDROM      ",
	L"RAMDISK    "
};

bool
verifyDrive(wchar_t* idestDrive)
{
	DWORD	dwSize = MAX_PATH;
	wchar_t szLogicalDrives[MAX_PATH] = {0};
	DWORD	dwResult = GetLogicalDriveStringsW(dwSize,szLogicalDrives);
	bool	r = false;

	printf("\n");
	printf("Available Drives: \n");
	printf("\n");

	if (dwResult > 0 && dwResult <= MAX_PATH)
	{
		wchar_t* szSingleDrive = szLogicalDrives;
		while(*szSingleDrive)
		{
			int type = GetDriveTypeW(szSingleDrive);

			wchar_t		destDrive[MAX_PATH];
			char		driveLetter = szSingleDrive[0];
			swprintf(destDrive, L"\\\\.\\%c:", driveLetter);

			float gigs = GetDriveCapacity(destDrive);

			if (type >= NUM_DRIVE_TYPES)
				type = 0;

			bool valid = (type == DRIVE_REMOVABLE && (gigs > 0));
			bool match = idestDrive ? (wcscmp(destDrive, idestDrive) == 0) : false;

			printf("%ls  (%ls %5.3g GB%s%s)",
				szSingleDrive,
				driveTypes[type],
				gigs,
				valid ? " AVAILABLE" : "",
				match ? " SELECTED" : "");

			printf("\n");

			if (valid && match)
				r = true;

		
			// get the next drive
			szSingleDrive += wcslen(szSingleDrive) + 1;
		}
	}

	return r;
}

void
DumpBlocks(FILE *input, HANDLE device)
{
	int	block = START_BLOCK;
	int lastSec = -1;

	while (!feof(input))
	{
		BYTE buf[512];
		fread(buf, 1, 512, input);

		int frame = ((block - START_BLOCK) / BLOCKS_PER_FRAME);
		int secs = frame/60;
		int mins = secs/60;
		int hrs = mins/60;

		secs %= 60;
		mins %= 60;

		if (secs != lastSec)
			printf("block %6d .. Frame:%6d  %02d:%02d:%02d \n", block, frame, hrs, mins, secs);
		lastSec = secs;

		SetFilePointer (device, block*512, NULL, FILE_BEGIN);

		DWORD bytesRead;
		if (!WriteFile(device, buf, 512, &bytesRead, NULL))
		{
			fprintf(stderr, "WriteFile Error: %u\n", GetLastError());
			break;
		}

		block += 1;
	}

	printf("\n");
}


int
main(int argc, char ** argv)
{
	wchar_t		destDrive[MAX_PATH];
    HANDLE		device = NULL;

	if (argc != 3)
	{
		fprintf(stderr, "Usage: %s input_file output_drive \n", argv[0]);
		verifyDrive(nullptr);
		return 1;
	}

	FILE	*input = fopen(argv[1], "rb");
	if (!input)
	{
		fprintf(stderr, "Unable to open source input %s \n", argv[1]);
		return 1;
	}

	char driveLetter = toupper(argv[2][0]);
	swprintf(destDrive, L"\\\\.\\%c:", driveLetter);

	if (!verifyDrive(destDrive))
	{
		printf("\n");
		fprintf(stderr, "No valid drive selected. Aborting.\n");
		return 1;
	}

	// final warning

	char c = 0;
	while(1)
	{
		if (c != '\n' && c != '\r')
			printf("WARNING: ALL DATA WILL BE ERASED.   PROCEED? (y/n)\n");

		c = getchar();

		if (c == 'y')
			break;
		else if (c == 'n')
			return 1;
	}


    device = CreateFileW(destDrive,			    // Drive to open
                        GENERIC_WRITE | GENERIC_READ,           // Access mode
                        0, //FILE_SHARE_READ|FILE_SHARE_WRITE,        // Share Mode
                        NULL,                   // Security Descriptor
                        OPEN_EXISTING,          // How to create
                        FILE_ATTRIBUTE_NORMAL, //0,                      // File attributes
                        NULL);                  // Handle to template

    if (device == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error calling CreateFile: %u\n", GetLastError());
        return 1;
    }

	DumpBlocks(input, device);

	fclose(input);
	CloseHandle(device);


    return 0;
}
