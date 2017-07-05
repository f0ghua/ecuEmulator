#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "Attribute.h"
#include "ValueDescriptions.h"

namespace Vector {
namespace DBC {

/**
 * Environment Variable (EV)
 */
class VECTOR_DBC_EXPORT EnvironmentVariable
{
public:
    EnvironmentVariable();

    /** Name */
    QString name;

    /** Type */
    enum class Type : char {
        Integer, /**< Integer */
        Float, /**< Float */
        String, /**< String */
        Data /**< Data */
    };

    /** Type */
    Type type;

    /** Minimum */
    double minimum;

    /** Maximum */
    double maximum;

    /** Unit */
    QString unit;

    /** Initial Value */
    double initialValue;

    /** Identifier */
    unsigned int id;

    /** Access Type */
    enum class AccessType {
        Unrestricted = 0, /**< Unrestricted */
        Read = 1, /**< Read */
        Write = 2, /**< Write */
        ReadWrite = 3 /**< Read and Write */
    };

    /** Access Type */
    AccessType accessType;

    /** Access Nodes */
    QStringList accessNodes;

    /** Value Descriptions (VAL) */
    ValueDescriptions valueDescriptions;

    /** Environment Variables Data (ENVVAR_DATA) */
    unsigned int dataSize;

    /** Comment (CM) */
    QString comment;

    /** Attribute Values (BA) */
    QMap<QString, Attribute> attributeValues;
};

}
}
