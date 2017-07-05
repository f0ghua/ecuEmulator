#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "Attribute.h"

namespace Vector {
namespace DBC {

/**
 * Node (BU)
 */
class VECTOR_DBC_EXPORT Node
{
public:
    Node();

    /** Name */
    QString name;

    /** Comment (CM) */
    QString comment;

    /** Attribute Values (BA) */
    QMap<QString, Attribute> attributeValues;
};

}
}
