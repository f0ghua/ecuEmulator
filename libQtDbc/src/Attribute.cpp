#include "Attribute.h"

namespace Vector {
namespace DBC {

Attribute::Attribute() :
    name(),
    valueType(AttributeValueType::Int),
    integerValue(0),
    stringValue()
{
    /* nothing to do here */
}

}
}