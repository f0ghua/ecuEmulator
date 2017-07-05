#include "AttributeDefinition.h"

namespace Vector {
namespace DBC {

AttributeDefinition::AttributeDefinition() :
    name(),
    objectType(AttributeDefinition::ObjectType::Network),
    valueType(AttributeValueType::Int),
    minimumIntegerValue(0),
    maximumIntegerValue(0),
    enumValues()
{
    /* nothing to do here */
}

}
}