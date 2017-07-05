#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

namespace Vector {
namespace DBC {

/**
 * Attribute Value Type
 */
enum class AttributeValueType {
    Int, /**< Integer */
    Hex, /**< Hex */
    Float, /**< Float */
    String, /**< String */
    Enum /**< Enum */
};

}
}
