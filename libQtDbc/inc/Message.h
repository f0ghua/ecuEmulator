#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "Attribute.h"
#include "SignalDbc.h"
#include "SignalGroup.h"

namespace Vector {
namespace DBC {

/**
 * Message (BO)
 */
class VECTOR_DBC_EXPORT Message
{
public:
    Message();

    /** Identifier (with bit 31 set this is extended CAN frame) */
    unsigned int id;

    /** Name */
    QString name;

    /** Size */
    unsigned int size;

    /** Transmitter (empty string if the number of send nodes is zero or more than one) */
    QString transmitter;

    /** Signals (SG) */
    QMap<QString, Signal> m_signals;

    /** Message Transmitters (BO_TX_BU) */
    QStringList transmitters;

    /** Signal Groups (SIG_GROUP) */
    QMap<QString, SignalGroup> signalGroups;

    /** Comment (CM) */
    QString comment;

    /** Attribute Values (BA) */
    QMap<QString, Attribute> attributeValues;

    Signal *multiplexorSignal;

    Signal *findSignalByName(QString name) const;
    bool addSignal(Signal &sig);
};

}
}
