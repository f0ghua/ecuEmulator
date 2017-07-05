
#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "AttributeValueType.h"

namespace Vector {
namespace DBC {

/**
 * Attribute Value (BA) and
 * Attribute Default (BA_DEF_DEF) and
 * Attribute Default Value on Relation (BA_DEF_DEF_REL)
 */
class VECTOR_DBC_EXPORT Attribute
{
public:
    Attribute();

    /** Name */
    QString name;

    /** Value Type */
    AttributeValueType valueType;

    union {
        /** Integer Value of type AttributeValueType::Int */
        int integerValue;

        /** Hex Value of type AttributeValueType::Hex */
        int hexValue;

        /** Float Value of type AttributeValueType::Float */
        double floatValue;

        /** Enum Value of type AttributeValueType::Enum (used only for BA enums) */
        int enumValue;

        // std::string doesn't work in a union, so it's below
    };

    /** String Value of type AttributeValueType::String (used only for BA_DEF_DEF enums) */
    QString stringValue;
};

}
}
