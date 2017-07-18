#include "xcommdefine.h"
#include "xbusmgr.h"
#include "ftworker.h"

#include <pthread.h>
#include <windows.h>

#include <QDebug>
#include <QObject>

#ifdef Q_OS_LINUX
#include <sys/time.h>
#else
typedef struct _EVENT_HANDLE
{
	pthread_cond_t eCondVar;
	pthread_mutex_t eMutex;
	int iVar;
} EVENT_HANDLE;
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
	#define DELTA_EPOCH_IN_MICROSECS	11644473600000000Ui64
#else
	#define DELTA_EPOCH_IN_MICROSECS	11644473600000000ULL
#endif

int gettimeofday(struct timeval *tv, struct timezone *)
{
	FILETIME ft;
	unsigned __int64 tmpres = 0;

	if (NULL != tv) {
		GetSystemTimeAsFileTime(&ft);

		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;

		tmpres /= 10; /*convert into microseconds*/
		/*converting file time to unix epoch*/
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}
	return 0;
}
#endif //Q_OS_LINUX

FtWorker::FtWorker(XBusMgr *mgr) :HAL(mgr, 1)
{
	raw.clear();
	RxBuffer = (char *)calloc(128 * 1024, 1);
}

void FtWorker::PhyOpenDevice(QString tryDev, HANDLE &eh)
{
	FT_STATUS ftStatus;
	DWORD EventDWord, TxBytes, RxBytes, EventMask = FT_EVENT_RXCHAR | FT_EVENT_MODEM_STATUS;

    //printf("%s\n", tryDev.toLatin1().data());
	ftStatus = FT_OpenEx(tryDev.toLatin1().data(), FT_OPEN_BY_SERIAL_NUMBER, &ftHandle);
	if (ftStatus != FT_OK) {
		printf("FT_OpenEx failed (error code %d), %s\n", (int)ftStatus, tryDev.toLatin1().data());
		return;
	}

	ftStatus = FT_SetDivisor(ftHandle, 0);
	if (ftStatus == FT_OK) {
		//printf("setbaund ok\n");
	}
	else {
		printf("setbaund fail\n");
	}

	ftStatus = FT_SetDataCharacteristics(ftHandle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
	if (ftStatus == FT_OK) {
		//printf("set 8n1 ok\n");
	}
	else {
		printf("set 8n1 fail\n");
	}

	ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0x11, 0x13);
	if (ftStatus == FT_OK) {
		//printf("set flow ok\n");
	}
	else {
		printf("set flow fail\n");
	}

	ftStatus = FT_SetDtr(ftHandle);
	if (ftStatus == FT_OK) {
		//printf("set dtr ok\n");
	}
	else {
		printf("set dtr fail\n");
	}

	ftStatus = FT_SetRts(ftHandle);
	if (ftStatus == FT_OK) {
		//printf("set rts ok\n");
	}
	else {
		printf("set rts fail\n");
	}

	ftStatus = FT_SetLatencyTimer(ftHandle, 2);
	if (ftStatus == FT_OK) {
		qDebug() << "set timeouts ok";
	}
	else {
		qDebug() << "set timeouts fail";
	}	

	qDebug() << dev << " open ok";;
    eh = ::CreateEvent(NULL, false, false, reinterpret_cast<LPCWSTR>(""));

	ftStatus = FT_SetEventNotification(ftHandle, EventMask, (PVOID)eh);
	if (ftStatus == FT_OK) {
		//printf("set ev ok\n");
	}
	else {
		printf("set ev fail\n");
		return;
	}

	FT_GetStatus(ftHandle,&RxBytes,&TxBytes,&EventDWord);
	emit updateDeviceConnState(OPEN_SUCC);
}

void FtWorker::PhyCloseDevice()
{
	if(ftHandle != NULL) {
		(void)FT_Close(ftHandle);
		ftHandle = NULL;
		emit updateDeviceConnState(CLOSE_SUCC);
		needToClose = 0;
		printf("ft closed\n");
	}
}

void FtWorker::sigRefreshDevice()
{
	getList = 1;
	qc.wakeAll();
}

void FtWorker::listDevice()
{
	FT_STATUS ftStatus;
	DWORD numDevs = 0;
	int i, retry = 0;

	deviceList.clear();
	ftStatus = FT_CreateDeviceInfoList(&numDevs);
	if (ftStatus != FT_OK) {
		printf("FT_CreateDeviceInfoList failed (error code %d)\n", (int)ftStatus);
		return;
	}

    if (numDevs > 0) {
        FT_DEVICE_LIST_INFO_NODE *devInfo;
        devInfo = (FT_DEVICE_LIST_INFO_NODE *)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
        ftStatus = ::FT_GetDeviceInfoList(devInfo, &numDevs);
        if (ftStatus == FT_OK) {
            for (unsigned int i = 0; i < numDevs; ++i) {
                if (*devInfo[i].SerialNumber != '\0')
                    deviceList << QString(devInfo[i].SerialNumber);
            }
        }
        free(devInfo);
    }
#if 0
	for (i = 0; i < (int)numDevs; i++) {
		char SerialNumber[64];

		memset(SerialNumber, 0, sizeof(SerialNumber));
		ftStatus = FT_ListDevices((PVOID)(uintptr_t) i, SerialNumber,FT_LIST_BY_INDEX|FT_OPEN_BY_SERIAL_NUMBER);
		if(strlen(SerialNumber) == 0 && retry < 8) {
			i--;
			continue;
			retry++;
		}
		else
			retry = 0;

		deviceList << QString(SerialNumber);
	}
#endif
	emit updateDeviceList(deviceList);
}

void FtWorker::sigOpenDevice(QString tryDev)
{
	if(dev != tryDev) {

		if(ftHandle != NULL)
			sigCloseDevice(0);

		dev = tryDev;
		qc.wakeAll();
	}
}

void FtWorker::sigCloseDevice(int)
{
	needToClose = 1;
	dev = "";
}

void FtWorker::run()
{
	HANDLE eh;

	while(shutdown == 0) {
		if(ftHandle == NULL) {
			
			if(dev.size() == 0) {
				m.lock();
				qc.wait(&m);
				m.unlock();
			}
			
			if(getList == 1) {
				getList = 0;
				listDevice();
				continue;
			}
			PhyOpenDevice(dev, eh);
			getv = 1;
		}

        ::WaitForSingleObject(eh, INFINITE);

		if(needToClose == 1)
			PhyCloseDevice();
		else
			dealWithEvent();
	}
}

void FtWorker::dealWithEvent()
{
	DWORD TxBytes, RxBytes, Status, EventDWord;

	FT_GetStatus(ftHandle,&RxBytes, &TxBytes, &EventDWord);

	if (EventDWord & FT_EVENT_MODEM_STATUS) {
		FT_GetModemStatus(ftHandle,&Status);
		if (Status & 0x00000010) { // CTS is high
		}
		else { // CTS is low
		}
		if (Status & 0x00000020) { // DSR is high
		}
		else { // DSR is low
		}
	}
	else if (RxBytes > 0)
		dealWithRx(RxBytes);
}

void FtWorker::dealWithRx(int RxBytes)
{
	DWORD BytesReceived = 0;
	FT_STATUS ftStatus;
	int soFar = 0;

	while(RxBytes > 0) {
		ftStatus = FT_Read(ftHandle, RxBuffer + soFar, RxBytes, &BytesReceived);
		if (ftStatus == FT_OK) {
			RxBytes -= BytesReceived;
			soFar += BytesReceived;
		}
		else {
			// FT_Read Failed
		}
	}

	for (int i = 0; i < soFar; i++)
		procRXChar(RxBuffer[i]);
}

void FtWorker::halWrite(QByteArray &buffer)
{
	DWORD BytesWritten = 0, soFar = 0;
	FT_STATUS ftStatus;
	char tmp[4096];
	
	if(ftHandle == NULL) return;
	if(buffer.size() == 0) return;

	memcpy(tmp, buffer.data(), buffer.size());

	while(soFar != (DWORD) buffer.size()) {
		ftStatus = FT_Write(ftHandle, tmp + soFar, buffer.size() - soFar, &BytesWritten);

		if (ftStatus == FT_OK)
			soFar += BytesWritten;
	}
	FT_SetRts(ftHandle);
}
