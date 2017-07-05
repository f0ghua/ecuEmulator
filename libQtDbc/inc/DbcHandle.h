#ifndef DBCHANDLE_H
#define DBCHANDLE_H

#include "vector_dbc_export.h"

#include "Network.h"

namespace Vector {
namespace DBC {

class VECTOR_DBC_EXPORT NetworkHandle
{
public:
    Network *getNetwork() {return &network;}

    QString buildAttributeString(const Attribute &attribute);

    void dumpMessages();
    void dumpMessagesTransmitters();
    void dumpSignals();
    void dumpSignalsReceivers();
    void dumpMappedSignalsbyMsgId(int id);
    void dumpSignalValueDecriptionByName(QString name);
    void dumpAttributeDefinitions();
    void dumpAttributeDefaults();
    void dumpNetworkAttributes();
    void dumpNodesAttributes();
    void dumpMessagesAttributes();
    void dumpSignalsAttributes();
    void dumpMessagesSignalGroups();
    void dumpRawBits(QByteArray &canData);
    void dumpMessageSampleData();
    void dumpMessageSampleData_2();
    void dumpMessageSampleData_3();

private:
    Network network;
};

class QLIBDBCSHARED_EXPORT DBCHandle
{
public:

private:

};

}
}

#endif // DBCHANDLE_H
