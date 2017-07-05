#include "Network.h"

#include <QDebug>

namespace Vector {
namespace DBC {

Network::Network() :
    version(),
    newSymbols()
#if 0
    bitTiming(),
    nodes(),
    valueTables(),
    messages(),
    environmentVariables(),
    signalTypes(),
    comment(),
    attributeDefinitions(),
    attributeDefaults(),
    attributeValues(),
    attributeRelationValues()
#endif
{
    /* nothing to do here */
}

Node *Network::findNodeByName(QString name)
{
    auto it = nodes.find(name);
    if (it != nodes.end())
        return &it.value();

    return NULL;
}

bool Network::addNode(Node &node)
{
    nodes.insert(node.name, node);
    return true;
}

void Network::removeAllMessages()
{
    messages.clear();
}

Message *Network::findMsgByID(unsigned int id)
{
    auto it = messages.find(id);
    if (it != messages.end())
        return &it.value();

    return NULL;
}

Message *Network::findMsgByName(QString name)
{
    for (auto ciMsg = messages.constBegin();
        ciMsg != messages.constEnd();
        ciMsg++)
    {
        if (ciMsg.value().name == name)
        {
            return const_cast<Message *>(&ciMsg.value());
        }
    }

    return NULL;
}

Signal *Network::findSignalByName(const QString name)
{
#if 1
    for (auto ciMsg = messages.constBegin();
        ciMsg != messages.constEnd();
        ciMsg++)
    {
        auto it = ciMsg.value().m_signals.find(name);
        if (it != ciMsg.value().m_signals.end())
            return const_cast<Signal *>(&(it.value()));
    }
#else

    // be careful!! following format is wrong, because msg is a new local var
    // which will be released when function finish! and we also found stack
    // overflow issue.

    for (auto msg : messages)
    {
        auto it = msg.m_signals.find(name);
        if (it != msg.m_signals.end())
            return &it.value();
    }
#endif
    return NULL;
}

bool Network::addMessage(Message &msg)
{
    messages.insert(msg.id, msg);
    return true;
}

#ifdef SUPPORT_NETWORK_SIGNAL
bool Network::addSignalPointer(Signal *sig)
{
#ifndef F_NO_DEBUG
    if (pSignals.find(sig->name) != pSignals.end())
    {
        qDebug() << "Add duplicate signal " << sig->name;
    }
#endif
    pSignals.insert(sig->name, sig);
    return true;
}
#endif

AttributeDefinition *Network::findAttributeDefinitionByName(QString name)
{
    auto it = attributeDefinitions.find(name);
    if (it != attributeDefinitions.end())
        return &it.value();

    return NULL;
}

EnvironmentVariable *Network::findEnvrionmentVariablebyName(QString name)
{
    auto it = environmentVariables.find(name);
    if (it != environmentVariables.end())
        return &it.value();

    return NULL;
}







}
}
