#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

namespace Vector {
namespace DBC {

/**
 * Bit Timing (BS)
 */
class VECTOR_DBC_EXPORT BitTiming
{
public:
    BitTiming();

    /** Baud rate */
    unsigned int baudrate;

    /** Bit Timing Register 1 */
    unsigned int btr1;

    /** Bit Timing Register 2 */
    unsigned int btr2;
};

}
}
