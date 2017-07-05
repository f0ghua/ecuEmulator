#include "utils.h"
#include "hal.h"
#include "xbusmgr.h"

#include <QDebug>
#include <QDateTime>

unsigned char HAL::ffFlag = 0;
static const char g_frameEndStr[2] = {(char)0xFF, (char)0x00};
const char *HAL::vHdr= "ICITS Software Version Number:";

HAL::HAL(XBusMgr *mgr, int m) : m_mgr(mgr), mode(m)
{
	raw.clear();
}

void HAL::procRXChar(unsigned char c)
{
	if ((c == 0xFF) && (ffFlag == 0)) { // first 0xFF
		ffFlag = 1;
		return;
	}

	if (ffFlag == 0) {
		raw.append(c);
	}
	else {
		ffFlag = 0;
		if (c == 0xFF) { // 2 0xFF, the first one means escape character
			raw.append(c);
		}
		else if (c == 0x00) { // end of message
			handleFullData(raw);
			raw.clear();
		}
		else { // a new message start
			handleFullData(raw);
			raw.clear();
			raw.append(c);
		}
	}
}

void HAL::handleFullData(const QByteArray &raw)
{
	XBusFrame frame(raw);

	if (XBusFrame::isCommandFrame(raw)) {
#ifndef F_NO_DEBUG
        //qDebug() << tr("[%1], got cmd frame").arg(QDateTime::currentMSecsSinceEpoch());
        //qDebug() << tr("hal read cmd [%1]").arg(Utils::Base::formatByteArray(&raw));
#endif
		emit cmdFrameResponse(raw);
	}
    else if (m_mgr->isRunning()){
#ifndef F_NO_DEBUG
        //qDebug() << tr("hal read data [%1]").arg(Utils::Base::formatByteArray(&raw));
#endif      
        m_mgr->enqueueReceivedFrame(frame);
		framesRapid++;
	}
}

void HAL::sendFrame(const XBusFrame *frame)
{
	QByteArray buffer, ba;

    //if (!pro->getAction()) return;

    ba = XBusFrame::buildHeader(frame->id(), frame->bus());
	buffer.append(ba);

    const QByteArray &payload = frame->payload();
    for (int i = 0; i < payload.count(); i++) {
        buffer.append(payload.at(i));
        if ((quint8)payload.at(i) == 0xFF)
			buffer.append(0xFF);
	}

	buffer.append(QByteArray::fromRawData(g_frameEndStr, sizeof(g_frameEndStr)));

	halWrite(buffer);
}

void HAL::sendRawData(const QByteArray &raw)
{
	QByteArray buffer = XBusFrame::packFrame(raw);

#ifndef F_NO_DEBUG
	//qDebug() << "writing " << buffer.length() << " bytes to serial port";
    //qDebug() << QObject::tr("send frame[%1]: %2").arg(buffer.count()).arg(Utils::Base::formatByteArray(&buffer));
#endif

	halWrite(buffer);
}
