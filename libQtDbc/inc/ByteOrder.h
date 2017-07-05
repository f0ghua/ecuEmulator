#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

namespace Vector {
namespace DBC {

/**
 * Byte Order Type
 */
enum class ByteOrder : char {
    Motorola = '0', /** < Motorola / Big Endian */
    BigEndian = '0', /**< Bit Endian / Motorola */
    Intel = '1', /**< Intel / Little Endian */
    LittleEndian = '1' /**< Little Endian / Intel */
};

}
}
