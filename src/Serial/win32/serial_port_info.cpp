#include "serial_port_info.h"
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <tchar.h>

std::vector<SerialPortInfo> SerialPortInfo::listPorts()
{
    std::vector<SerialPortInfo> ports;

	HDEVINFO device_info_set = SetupDiGetClassDevs(
		(const GUID *) &GUID_DEVCLASS_PORTS,
		NULL,
		NULL,
		DIGCF_PRESENT);

	unsigned int device_info_set_index = 0;
	SP_DEVINFO_DATA device_info_data;

	device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

	while(SetupDiEnumDeviceInfo(device_info_set, device_info_set_index, &device_info_data))
	{
		device_info_set_index++;

		// Get port name

		HKEY hkey = SetupDiOpenDevRegKey(
			device_info_set,
			&device_info_data,
			DICS_FLAG_GLOBAL,
			0,
			DIREG_DEV,
			KEY_READ);

		TCHAR port_name[MAX_PATH];
		DWORD port_name_length = MAX_PATH;

		LONG return_code = RegQueryValueEx(
					hkey,
					_T("PortName"),
					NULL,
					NULL,
					(LPBYTE)port_name,
					&port_name_length);

		RegCloseKey(hkey);

		if(return_code != EXIT_SUCCESS)
			continue;

		if(port_name_length > 0 && port_name_length <= MAX_PATH)
			port_name[port_name_length-1] = '\0';
		else
			port_name[0] = '\0';

		// Ignore parallel ports

		if(_tcsstr(port_name, _T("LPT")) != NULL)
			continue;

		// Get port friendly name

		TCHAR friendly_name[MAX_PATH];
		DWORD friendly_name_actual_length = 0;

		BOOL got_friendly_name = SetupDiGetDeviceRegistryProperty(
					device_info_set,
					&device_info_data,
					SPDRP_FRIENDLYNAME,
					NULL,
					(PBYTE)friendly_name,
					MAX_PATH,
					&friendly_name_actual_length);

		if(got_friendly_name == TRUE && friendly_name_actual_length > 0)
			friendly_name[friendly_name_actual_length-1] = '\0';
		else
			friendly_name[0] = '\0';

		// Get hardware ID

		TCHAR hardware_id[MAX_PATH];
		DWORD hardware_id_actual_length = 0;

		BOOL got_hardware_id = SetupDiGetDeviceRegistryProperty(
					device_info_set,
					&device_info_data,
					SPDRP_HARDWAREID,
					NULL,
					(PBYTE)hardware_id,
					MAX_PATH,
					&hardware_id_actual_length);

		if(got_hardware_id == TRUE && hardware_id_actual_length > 0)
			hardware_id[hardware_id_actual_length-1] = '\0';
		else
			hardware_id[0] = '\0';

		TCHAR mfc[MAX_PATH];
		DWORD mfc_actual_length = 0;
		BOOL got_mfc = SetupDiGetDeviceRegistryProperty(
					device_info_set,
					&device_info_data,
					SPDRP_MFG,
					NULL,
					(PBYTE)mfc,
					MAX_PATH,
					&mfc_actual_length);
		if(got_mfc == TRUE && mfc_actual_length > 0)
			mfc[mfc_actual_length-1] = '\0';
		else
			mfc[0] = '\0';

		SerialPortInfo port_entry;
		port_entry.port = port_name;
		port_entry.description = friendly_name;
		port_entry.manufacturer = mfc;

        // 提职VID和PID
        const char *p = strstr(hardware_id, "VID_");
        if (p) {
            port_entry.vid = strtoul(p + 4, NULL, 16);
        } else {
            port_entry.vid = 0;
        }
        p = strstr(hardware_id, "PID_");
        if (p) {
            port_entry.pid = strtoul(p + 4, NULL, 16);
        } else {
            port_entry.pid = 0;
        }
		

		ports.push_back(port_entry);
	}

	SetupDiDestroyDeviceInfoList(device_info_set);

	return ports;
}
