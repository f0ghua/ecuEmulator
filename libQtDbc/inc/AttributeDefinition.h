#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "AttributeValueType.h"

namespace Vector {
namespace DBC {

/**
 * Attribute Definition (BA_DEF) and
 * Attribute Definition for Relations (BA_DEF_REL)
 */
class VECTOR_DBC_EXPORT AttributeDefinition
{
public:
    AttributeDefinition();

    /** Name */
    QString name;

    /** Object Type */
    enum class ObjectType {
        Network, /**< Network */
        Node, /**< Node */
        Message, /**< Message */
        Signal, /**< Signal */
        EnvironmentVariable, /**< Environment Variable */
        ControlUnitEnvironmentVariable, /**< Control Unit - Env. Variable */
        NodeTxMessage, /**< Node - Tx Message */
        NodeMappedRxSignal /**< Node - Mapped Rx Signal */
    };

    /** Object Type */
    ObjectType objectType;

    /** Attribute Value Type */
    AttributeValueType valueType;

    union {
        struct {
            int minimumIntegerValue; /**< Min Value of type AttributeValueType::Int */
            int maximumIntegerValue; /**< Min Value of type AttributeValueType::Int */
        };

        struct {
            int minimumHexValue; /**< Min Value of type AttributeValueType::Hex */
            int maximumHexValue; /**< Max Value of type AttributeValueType::Hex */
        };

        struct {
            double minimumFloatValue; /**< Min Value of type AttributeValueType::Float */
            double maximumFloatValue; /**< Max Value of type AttributeValueType::Float */
        };
    };

    /** Values of type AttributeValueType::Enum */
    QStringList enumValues;
};

}
}
