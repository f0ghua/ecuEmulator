#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

namespace Vector {
namespace DBC {

/**
 * Extended Multiplexor (SG_MUL_VAL)
 */
class VECTOR_DBC_EXPORT ExtendedMultiplexor
{
public:
    ExtendedMultiplexor();

    /** Switch Name */
    QString switchName;

    /** Value Range */
    //typedef std::pair<unsigned int, unsigned int> ValueRange;

    /** Value Range */
    //std::set<ValueRange> valueRanges;
};

}
}
