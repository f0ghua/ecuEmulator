#pragma once

#include <QObject>

#include "platform.h"
#include "vector_dbc_export.h"

#include "Network.h"
#include "Status.h"
#include "DbcHandle.h"

namespace Vector {
namespace DBC {

/**
 * File
 */
class VECTOR_DBC_EXPORT File
{
public:
    File();
    ~File();
    NetworkHandle *netHandle() {return m_pNetHandle;}
    Network *network() {return m_pNetwork;}
    QString filename() {return m_fileName;}

    /**
     * @brief Load database file
     * @param[out] network Network
     * @param[in] filename File name
     * @return Status code
     *
     * Loads database file.
     */
    Status load(const char *fileName);

    /**
     * @brief Load database file
     * @param[out] network Network
     * @param[in] filename File Name
     * @return Status code
     *
     * Loads database file.
     */
    Status load(const QString &fileName);

    void readNodes(QString &line);
    void readEnvironmentVariable(QString &line);
    void readMessageTransmitter(QString &line);
    bool readValueDescriptionSignal(QString &line);
    bool readValueDescriptionEnvironmentVariable(QString &line);
    void readValueDescription(QString &line);
    void readAttributeDefinition(QString &line);
    void readAttributeDefault(QString &line);
    bool readAttributeValueNode(QString &line);
    bool readAttributeValueMessage(QString &line);
    bool readAttributeValueSignal(QString &line);
    bool readAttributeValueEnvironmentVariable(QString &line);
    bool readAttributeValueNetwork(QString &line);
    void readAttributeValue(QString &line);
    void readSignalGroup(QString &line);

private:
    QString m_fileName;
    Network *m_pNetwork;
    NetworkHandle *m_pNetHandle;

};

}
}