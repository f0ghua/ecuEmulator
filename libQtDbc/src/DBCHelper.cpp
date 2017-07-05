#include "DBCHelper.h"

using namespace Vector::DBC;

DBCHelper::DBCHelper()
{

}

const Vector::DBC::Attribute *DBCHelper::getAttributeByName(
    const QString name,
    const QMap<QString, Vector::DBC::Attribute> &attrMap
    )
{
    QMap<QString, Vector::DBC::Attribute>::const_iterator ci;

    if ((ci = attrMap.find(name)) != attrMap.end())
        return &ci.value();

    return NULL;
}

const QStringList *DBCHelper::getAttributeDefinitionEnum(
    Vector::DBC::AttributeDefinition::ObjectType objType,
    QString name,
    const Vector::DBC::Network *pNetwork
)
{
    for (auto ci = pNetwork->attributeDefinitions.constBegin();
        ci != pNetwork->attributeDefinitions.constEnd();
        ci++)
    {
        if (ci.value().objectType != objType)
            continue;

        if (
            (ci.value().valueType == Vector::DBC::AttributeValueType::Enum) &&
            (name == ci.value().name)
            )
        {
            return (&(ci.value().enumValues));
        }
    }

    return Q_NULLPTR;
}

int DBCHelper::getAttributeValueInt(const Vector::DBC::Message *pMessage,
        const Vector::DBC::Network *pNetwork,
        const QString name, bool *ok)
{
    const Vector::DBC::Attribute *pAttr;

	if ((pMessage == NULL)||(pNetwork == NULL)) {
		if (ok) *ok = false;
		return -1;
	}

    if ((pAttr = getAttributeByName(name, pMessage->attributeValues))) {
        if (pAttr->valueType == Vector::DBC::AttributeValueType::Int) {
            if (ok) *ok = true;
            return pAttr->integerValue;
        } else if (pAttr->valueType == Vector::DBC::AttributeValueType::Float) {
            if (ok) *ok = true;
            return pAttr->floatValue;
        }
    }
    if ((pAttr = getAttributeByName(name, pNetwork->attributeDefaults))) {
        if (pAttr->valueType == Vector::DBC::AttributeValueType::Int) {
            if (ok) *ok = true;
            return pAttr->integerValue;
        } else if (pAttr->valueType == Vector::DBC::AttributeValueType::Float) {
            if (ok) *ok = true;
            return pAttr->floatValue;
        }
    }

/*
	const QMap<QString, Vector::DBC::Attribute> &avs = pMessage->attributeValues;
    QMap<QString, Vector::DBC::Attribute>::const_iterator ci;

    ci = avs.find(name);
    if ((ci != avs.end()) &&
        (ci.value().valueType == Vector::DBC::AttributeValueType::Int)) {
        *ok = true;
        return ci.value().integerValue;
    }
*/

    if (ok) *ok = false;
    return -1;
}

int DBCHelper::getAttributeValueInt(const Vector::DBC::Signal *pSignal,
    const Vector::DBC::Network *pNetwork,
    const QString name, bool *ok)
{
    const Vector::DBC::Attribute *pAttr;

	if ((pNetwork == NULL) || (pSignal == NULL)) {
		if (ok) *ok = false;
		return -1;
	}

    if ((pAttr = getAttributeByName(name, pSignal->attributeValues))) {
        if (pAttr->valueType == Vector::DBC::AttributeValueType::Int) {
            if (ok) *ok = true;
            return pAttr->integerValue;
        } else if (pAttr->valueType == Vector::DBC::AttributeValueType::Float) {
            if (ok) *ok = true;
            return pAttr->floatValue;
        }
    }
    if ((pAttr = getAttributeByName(name, pNetwork->attributeDefaults))) {
        if (pAttr->valueType == Vector::DBC::AttributeValueType::Int) {
            if (ok) *ok = true;
            return pAttr->integerValue;
        } else if (pAttr->valueType == Vector::DBC::AttributeValueType::Float) {
            if (ok) *ok = true;
            return pAttr->floatValue;
        }
    }
/*
	const QMap<QString, Vector::DBC::Attribute> &avs = pSignal->attributeValues;
    QMap<QString, Vector::DBC::Attribute>::const_iterator ci;

    ci = avs.find(name);
    if ((ci != avs.end()) &&
        (ci.value().valueType == Vector::DBC::AttributeValueType::Int)) {
        *ok = true;
        return ci.value().integerValue;
    }
*/
    if (ok) *ok = false;
    return -1;
}


QString DBCHelper::getAttributeValueEnum(const Vector::DBC::Message *pMessage,
	const Vector::DBC::Network *pNetwork,
	const QString name, bool *ok)
{
    const Vector::DBC::Attribute *pAttr;

    *ok = false;

	if ((pMessage == NULL)||(pNetwork == NULL)) {
		return QString();
	}

    if ((pAttr = getAttributeByName(name, pMessage->attributeValues)) &&
        (pAttr->valueType == Vector::DBC::AttributeValueType::Enum)) {
            *ok = true;
    }

    if ((!*ok) &&
        (pAttr = getAttributeByName(name, pNetwork->attributeDefaults)) &&
        (pAttr->valueType == Vector::DBC::AttributeValueType::Enum)) {
            *ok = true;
    }

    if (*ok)
    {
        const QStringList *sl;
        sl = getAttributeDefinitionEnum(
            Vector::DBC::AttributeDefinition::ObjectType::Message,
            name,
            pNetwork);
        if (sl != Q_NULLPTR)
        {
            *ok = true;
            return (*sl).at(pAttr->enumValue);
        }
    }

/*
	const QMap<QString, Vector::DBC::Attribute> &avs = pMessage->attributeValues;
    QMap<QString, Vector::DBC::Attribute>::const_iterator ci;
    ci = avs.find(name);
    if ((ci != avs.end()) &&
        (ci.value().valueType == Vector::DBC::AttributeValueType::Enum)) {
        const QStringList *sl;
        sl = getAttributeDefinitionEnum(
        	Vector::DBC::AttributeDefinition::ObjectType::Message,
        	name,
        	pNetwork);
        if (sl != Q_NULLPTR)
        {
        	*ok = true;
            return (*sl).at(ci.value().enumValue);
        }
    }
*/

    if (ok) *ok = false;
    return QString();
}

QString DBCHelper::getAttributeValueEnum(const Vector::DBC::Signal *pSignal,
	const Vector::DBC::Network *pNetwork,
	const QString name, bool *ok)
{
    const Vector::DBC::Attribute *pAttr;

    *ok = false;

	if ((pSignal == NULL)||(pNetwork == NULL)) {
		return QString();
	}

    if ((pAttr = getAttributeByName(name, pSignal->attributeValues)) &&
        (pAttr->valueType == Vector::DBC::AttributeValueType::Enum)) {
            *ok = true;
    }

    if ((!*ok) &&
        (pAttr = getAttributeByName(name, pNetwork->attributeDefaults)) &&
        (pAttr->valueType == Vector::DBC::AttributeValueType::Enum)) {
            *ok = true;
    }

    if (*ok)
    {
        const QStringList *sl;
        sl = getAttributeDefinitionEnum(
            Vector::DBC::AttributeDefinition::ObjectType::Signal,
            name,
            pNetwork);
        if (sl != Q_NULLPTR)
        {
            *ok = true;
            return (*sl).at(pAttr->enumValue);
        }
    }
/*
	const QMap<QString, Vector::DBC::Attribute> &avs = pSignal->attributeValues;
    QMap<QString, Vector::DBC::Attribute>::const_iterator ci;
    ci = avs.find(name);
    if ((ci != avs.end()) &&
        (ci.value().valueType == Vector::DBC::AttributeValueType::Enum)) {
        const QStringList *sl;
        sl = getAttributeDefinitionEnum(
        	Vector::DBC::AttributeDefinition::ObjectType::Signal,
        	name,
        	pNetwork);
        if (sl != Q_NULLPTR)
        {
        	*ok = true;
            return (*sl).at(ci.value().enumValue);
        }
    }
*/
    *ok = false;
    return QString();
}

QList<Vector::DBC::Message *> DBCHelper::getTxMessages4Node(
    const QString nodeName,
    Vector::DBC::Network *pNetwork
)
{
    QList<Vector::DBC::Message *> xMsgs;
    QMap<unsigned int, Vector::DBC::Message>::const_iterator ci;

    for (ci = pNetwork->messages.constBegin();
        ci != pNetwork->messages.constEnd();
        ci++)
    {
        Vector::DBC::Message *pMsg = const_cast<Vector::DBC::Message *>(&ci.value());

        if (pMsg->transmitter.isEmpty())
        {
            if (pMsg->transmitters.contains(nodeName))
                xMsgs.append(pMsg);
        }
        else
        {
            if (pMsg->transmitter == nodeName)
                xMsgs.append(pMsg);
        }
    }

    return xMsgs;
}

QList<Vector::DBC::Message *> DBCHelper::getRxMessages4Node(
    const QString nodeName,
    Vector::DBC::Network *pNetwork
)
{
    QList<Vector::DBC::Message *> xMsgs;
    QMap<unsigned int, Vector::DBC::Message>::const_iterator ci;

    for (ci = pNetwork->messages.constBegin();
        ci != pNetwork->messages.constEnd();
        ci++)
    {
        Vector::DBC::Message *pMsg = const_cast<Vector::DBC::Message *>(&ci.value());
        for (auto sig : ci.value().m_signals)
        {
            if (sig.receivers.contains(nodeName))
            {
                xMsgs.append(pMsg);
                break;
            }
        }
    }

    return xMsgs;
}

QString DBCHelper::getSignalDescriptionValue(const Vector::DBC::Signal *pSignal, unsigned int key)
{
	if (!pSignal->valueDescriptions.empty())
	{
	    ValueDescriptions::const_iterator ci;

        ci = pSignal->valueDescriptions.find(key);
        if (ci != pSignal->valueDescriptions.end()) {
            return ci.value();
        }
	}
	
	return QString();
}

int DBCHelper::getSignalDescriptionKey(const Vector::DBC::Signal *pSignal, QString value)
{
	if (!pSignal->valueDescriptions.empty()) {
		for (auto ci = pSignal->valueDescriptions.constBegin();
			ci != pSignal->valueDescriptions.constEnd();
 			ci++) {
			if (ci.value() == value) {
			    return ci.key();
			}
		}
	}
	
	return -1;
}

QStringList DBCHelper::getSignalDescriptionValues(const Vector::DBC::Signal *pSignal)
{
	QStringList sl;

	if (!pSignal->valueDescriptions.empty())
	{
		for (auto ci = pSignal->valueDescriptions.constBegin();
			ci != pSignal->valueDescriptions.constEnd();
			ci++)
		{
			sl.append(ci.value());
		}
	}
	return sl;
}

QStringList DBCHelper::getSignalDescriptions(const Vector::DBC::Signal *pSignal)
{
	QStringList sl;

	if (!pSignal->valueDescriptions.empty())
	{
		for (auto ci = pSignal->valueDescriptions.constBegin();
			ci != pSignal->valueDescriptions.constEnd();
			ci++)
		{
			QString vtValue = QStringLiteral("0x") + QString::number(ci.key(), 16).toUpper() +
				QStringLiteral(" : ") + ci.value();
			sl.append(vtValue);
		}
	}
	return sl;
}

double DBCHelper::getAttributeValueNum(const Vector::DBC::Signal *pSignal,
    const Vector::DBC::Network *pNetwork,
    const QString name, bool *ok)
{
    const Vector::DBC::Attribute *pAttr;

	if ((pNetwork == NULL) || (pSignal == NULL)) {
		if (ok) *ok = false;
		return -1;
	}

    if ((pAttr = getAttributeByName(name, pSignal->attributeValues))) {

        if (pAttr->valueType == Vector::DBC::AttributeValueType::Int) {
            if (ok) *ok = true;
            return pAttr->integerValue;
        } else
        if (pAttr->valueType == Vector::DBC::AttributeValueType::Float) {
            if (ok) *ok = true;
            return pAttr->floatValue;

        } else
        if (pAttr->valueType == Vector::DBC::AttributeValueType::Hex) {
            if (ok) *ok = true;
            return pAttr->hexValue;
        }
    }

    if ((pAttr = getAttributeByName(name, pNetwork->attributeDefaults))) {
		if (pAttr->valueType == Vector::DBC::AttributeValueType::Int) {
			if (ok) *ok = true;
			return pAttr->integerValue;
		} else
		if (pAttr->valueType == Vector::DBC::AttributeValueType::Float) {
			if (ok) *ok = true;
			return pAttr->floatValue;
		} else
		if (pAttr->valueType == Vector::DBC::AttributeValueType::Hex) {
			if (ok) *ok = true;
			return pAttr->hexValue;
		}
    }

    if (ok) *ok = false;
    return -1;
}


Vector::DBC::AttributeValueType DBCHelper::getAttributeDefinitionType(
    Vector::DBC::AttributeDefinition::ObjectType objType,
    QString name,
    const Vector::DBC::Network *pNetwork
)
{
    for (auto ci = pNetwork->attributeDefinitions.constBegin();
        ci != pNetwork->attributeDefinitions.constEnd();
        ci++)
    {
        if (ci.value().objectType != objType)
            continue;

        if (name == ci.value().name)
        {
            return (ci.value().valueType);
        }
    }

    return Vector::DBC::AttributeValueType::Int;
}

/* return default raw value of the signal */
double DBCHelper::getDefaultSignalValue(
	const Vector::DBC::Signal *pSignal,
	const Vector::DBC::Network *pNetwork, bool *ok)
{
    //const Vector::DBC::Attribute *pAttr;
	QString attrName = QStringLiteral("GenSigStartValue");

	if ((pNetwork == NULL) || (pSignal == NULL)) {
		*ok = false;
		return 0;
	}

    double v = getAttributeValueNum(pSignal, pNetwork, attrName, ok);
    return ((*ok)?v:0);
}

int DBCHelper::getMessageCycleTime(const Vector::DBC::Message *pMessage,
    const Vector::DBC::Network *pNetwork, const quint8 *data)
{
    bool ok;

    if (pMessage == NULL)
        return 0;

// The message is sent at a fixed cycle time independent of signal values
// (attribute: GenMsgCycleTime).
    int msgGenMsgCycleTime = DBCHelper::getAttributeValueInt(
        pMessage,
        pNetwork,
        "GenMsgCycleTime",
        &ok
        );
	// we don't care about the GenMsgSendType.
    if (ok) return msgGenMsgCycleTime;


	int msgGenMsgCycleTimeFast = DBCHelper::getAttributeValueInt(
		pMessage,
		pNetwork,
		"GenMsgCycleTimeFast",
		&ok
		);
	if (!ok) return 0;

// The message is sent at a fast cycle time (attribute: GenMsgCycleTimeFast) if
// a signal of the message is set to an active value (i.e. unlike the attribute
// GenSigInactiveValue). As soon as all values are inactive (i.e. they are
// conform to the attribute GenSigInactiveValue), the message is no longer sent.
//
    QString msgGenMsgSendType = DBCHelper::getAttributeValueEnum(
        pMessage,
        pNetwork,
        "GenMsgSendType",
        &ok
        );
    if ((ok) && (msgGenMsgSendType == "IfActive")) {
        //bool isContinue = false;
        for (auto sig : pMessage->m_signals) {
            int sigGenSigInactiveValue = DBCHelper::getAttributeValueInt(
                &sig,
                pNetwork,
                "GenSigInactiveValue",
                &ok
                );
            if (!ok) continue;

            if (sig.decode(data) != (quint64)sigGenSigInactiveValue) {
                return msgGenMsgCycleTimeFast;
            }
        }
    }

// No message send method found, get it from signals

    for (auto sig : pMessage->m_signals) {
// The message is sent at a fixed cycle time independent of signal values
// (attribute: GenMsgCycleTime).
        QString sigGenSigSendType = DBCHelper::getAttributeValueEnum(
            &sig,
            pNetwork,
            "GenSigSendType",
            &ok
            );
        if (!ok) return 0;
		
        int sigGenSigInactiveValue = DBCHelper::getAttributeValueInt(
            &sig,
            pNetwork,
            "GenSigInactiveValue",
            &ok
            );
        if (!ok) return 0;

        if (sigGenSigSendType == "IfActive") {
            if (sig.decode((quint8 *)data) != (quint64)sigGenSigInactiveValue)
                return msgGenMsgCycleTimeFast;
        }
    }

    return 0;
}


int DBCHelper::getMessageCycleTimeStrict(const Vector::DBC::Message *pMessage,
    const Vector::DBC::Network *pNetwork, const quint8 *data)
{
    bool ok;

    if (pMessage == NULL)
        return 0;

// The message is sent at a fixed cycle time independent of signal values
// (attribute: GenMsgCycleTime).
    QString msgGenMsgSendType = DBCHelper::getAttributeValueEnum(
        pMessage,
        pNetwork,
        "GenMsgSendType",
        &ok
        );
    if (!ok) return 0;

    int msgGenMsgCycleTime = DBCHelper::getAttributeValueInt(
        pMessage,
        pNetwork,
        "GenMsgCycleTime",
        &ok
        );
    if (!ok) return 0;

    if (msgGenMsgSendType == "Cyclic") {
        return msgGenMsgCycleTime;
    }

    int msgGenMsgCycleTimeFast = DBCHelper::getAttributeValueInt(
        pMessage,
        pNetwork,
        "GenMsgCycleTimeFast",
        &ok
        );
    if (!ok) return 0;

// The message is sent at a fast cycle time (attribute: GenMsgCycleTimeFast) if
// a signal of the message is set to an active value (i.e. unlike the attribute
// GenSigInactiveValue). As soon as all values are inactive (i.e. they are
// conform to the attribute GenSigInactiveValue), the message is no longer sent.
//
    if (msgGenMsgSendType == "IfActive") {
        //bool isContinue = false;
        for (auto sig : pMessage->m_signals) {
            int sigGenSigInactiveValue = DBCHelper::getAttributeValueInt(
                &sig,
                pNetwork,
                "GenSigInactiveValue",
                &ok
                );
            if (!ok) continue;

            if (sig.decode(data) != (quint64)sigGenSigInactiveValue) {
                return msgGenMsgCycleTimeFast;
            }
        }
    }

// No message send method found, get it from signals
    bool cyclicFlag = true;

    for (auto sig : pMessage->m_signals) {
// The message is sent at a fixed cycle time independent of signal values
// (attribute: GenMsgCycleTime).
        QString sigGenSigSendType = DBCHelper::getAttributeValueEnum(
            &sig,
            pNetwork,
            "GenSigSendType",
            &ok
            );
        if (!ok) return 0;

        if (sigGenSigSendType == "Cyclic") {
            return msgGenMsgCycleTime;
        }

        int sigGenSigInactiveValue = DBCHelper::getAttributeValueInt(
            &sig,
            pNetwork,
            "GenSigInactiveValue",
            &ok
            );
        if (!ok) return 0;

        if (sigGenSigSendType == "IfActive") {
            if (sig.decode((quint8 *)data) != (quint64)sigGenSigInactiveValue)
                return msgGenMsgCycleTimeFast;
        }

        if (sigGenSigSendType != "NoSigSendType")
            cyclicFlag = false;
    }

// If the send type of all the signals of a message is set to NoSigSendType, the
// message's send type must be set to Cyclic.
    if (cyclicFlag)
        return msgGenMsgCycleTime;

    return 0;
}
