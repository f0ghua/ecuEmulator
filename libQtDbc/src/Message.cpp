#include "Message.h"

namespace Vector {
namespace DBC {

Message::Message() :
    id(0),
    name(),
    size(0),
    transmitter(),
    m_signals(),
    transmitters(),
    signalGroups(),
    comment(),
    attributeValues()
{
    /* nothing to do here */
}

Signal *Message::findSignalByName(QString name) const
{
    auto it = m_signals.find(name);
    if (it != m_signals.end())
        return const_cast<Signal *>(&it.value());

    return NULL;
}

bool Message::addSignal(Signal &sig)
{
    m_signals.insert(sig.name, sig);
    return true;
}



}
}
