#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

namespace Vector {
namespace DBC {

/**
 * Value Type
 */
enum class ValueType : char {
    Unsigned = '+', /**< Unsigned */
    Signed = '-', /**< Signed */
    SPFloat = '2',
    DPFloat = '3',
    String= '4'
};

}
}
