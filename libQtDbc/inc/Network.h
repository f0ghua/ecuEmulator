#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "Attribute.h"
#include "AttributeDefinition.h"
//#include "AttributeRelation.h"
#include "BitTiming.h"
#include "EnvironmentVariable.h"
#include "Message.h"
#include "Node.h"
//#include "SignalType.h"
#include "Status.h"
#include "ValueDescriptions.h"
#include "ValueTable.h"

namespace Vector {
namespace DBC {

/**
 * Network
 */
class VECTOR_DBC_EXPORT Network
{
public:
    Network();

    QString name;

    /** Version (VERSION) */
    QString version;

    /** New Symbols (NS) */
    QStringList newSymbols;

    /** Bit Timing (BS) */
//    BitTiming bitTiming;

    /** Nodes (BU) */
    QMap<QString, Node> nodes;

    /** Value Tables (VAL_TABLE) */
//    QMap<QString, ValueTable> valueTables;

    /** Messages (BO) */
    QMap<unsigned int, Message> messages;

#ifdef SUPPORT_NETWORK_SIGNAL
    /** ordered Signals(name maybe duplicate), real object stored in message */
    QMultiMap<QString, Signal *> pSignals;
#endif
    /* Message Transmitters (BO_TX_BU) */
    // moved to Message (BO)

    /** Environment Variables (EV) */
    QMap<QString, EnvironmentVariable> environmentVariables;

    /* Environment Variables Data (ENVVAR_DATA) */
    // moved to Environment Variables (EV)

    /** Signal Types (SGTYPE, obsolete) */
//    QMap<QString, SignalType> signalTypes;

    /** Comments (CM) */
//    QString comment; // for network
    // moved to Node (BU) for nodes
    // moved to Message (BO) for messages
    // moved to Signal (SG) for signals
    // moved to Environment Variable (EV) for environment variables

    /**
     * Attribute Definitions (BA_DEF) and
     * Attribute Definitions for Relations (BA_DEF_REL)
     */
    QMap<QString, AttributeDefinition> attributeDefinitions;

    /* Sigtype Attr List (?, obsolete) */

    /**
     * Attribute Defaults (BA_DEF_DEF) and
     * Attribute Defaults for Relations (BA_DEF_DEF_REL)
     */
    QMap<QString, Attribute> attributeDefaults;

    /** Attribute Values (BA) */
    QMap<QString, Attribute> attributeValues; // for network
    // moved to Node (BU) for nodes
    // moved to Message (BO) for messages
    // moved to Signal (SG) for signals
    // moved to Environment Variable (EV) for environment variables

    /** Attribute Values on Relations (BA_REF) */
//    QSet<AttributeRelation> attributeRelationValues;

    /* Value Descriptions (VAL) */
    // moved to Signals (BO) for signals
    // moved to EnvironmentVariable (EV) for environment variables

    /* Category Definitions (CAT_DEF, obsolete) */

    /* Categories (CAT, obsolete) */

    /* Filters (FILTER, obsolete) */

    /* Signal Type Refs (SGTYPE, obsolete) */
    // moved to Signal (SG)

    /* Signal Groups (SIG_GROUP) */
    // moved to Message (BO)

    /* Signal Extended Value Types (SIG_VALTYPE, obsolete) */
    // moved to Signal (SG)

    /* Extended Multiplexors (SG_MUL_VAL) */
    // moved to Signal (SG)

    Node *findNodeByName(QString name);
    bool addNode(Node &node);
    void removeAllMessages();
    Message *findMsgByID(unsigned int id);
    Message *findMsgByName(QString name);
    Signal *findSignalByName(const QString name);
    bool addMessage(Message &msg);
    bool addSignalPointer(Signal *sig);
    AttributeDefinition *findAttributeDefinitionByName(QString name);
    EnvironmentVariable *findEnvrionmentVariablebyName(QString name);

};

}
}
