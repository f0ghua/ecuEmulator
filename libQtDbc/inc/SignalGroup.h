#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

namespace Vector {
namespace DBC {

/**
 * Signal Group (SIG_GROUP)
 */
class VECTOR_DBC_EXPORT SignalGroup
{
public:
    SignalGroup();

    /** Message Identifier */
    unsigned int messageId;

    /** Name */
    QString name;

    /** Repetitions */
    unsigned int repetitions;

    /** Signals */
    QStringList signal;
};

}
}
