#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "ValueDescriptions.h"

namespace Vector {
namespace DBC {

/**
 * Value Table (VAL_TABLE)
 */
class VECTOR_DBC_EXPORT ValueTable
{
public:
    ValueTable();

    /** Name */
    QString name;

    /** Value Descriptions */
    ValueDescriptions valueDescriptions;
};

}
}
