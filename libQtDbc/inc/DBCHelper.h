#ifndef DBCHELPER_H
#define DBCHELPER_H

#include "vector_dbc_export.h"
#include "Network.h"

class DBCHelper
{
public:
    DBCHelper();

	static const Vector::DBC::Attribute *getAttributeByName(
    	const QString name,
    	const QMap<QString, Vector::DBC::Attribute> &attrMap
    	);
    static const QStringList *getAttributeDefinitionEnum(
    	Vector::DBC::AttributeDefinition::ObjectType objType,
    	QString name,
    	const Vector::DBC::Network *pNetwork
		);
	static int getAttributeValueInt(const Vector::DBC::Message *pMessage,
		const Vector::DBC::Network *pNetwork,
		const QString name, bool *ok = Q_NULLPTR);
	static int getAttributeValueInt(const Vector::DBC::Signal *pSignal,
		const Vector::DBC::Network *pNetwork,
		const QString name, bool *ok = Q_NULLPTR);
	static QString getAttributeValueEnum(const Vector::DBC::Message *pMessage,
		const Vector::DBC::Network *pNetwork,
		const QString name, bool *ok
		);
    static QString getAttributeValueEnum(const Vector::DBC::Signal *pSignal,
		const Vector::DBC::Network *pNetwork,
		const QString name, bool *ok
		);
    static QList<Vector::DBC::Message *> getTxMessages4Node(
    	const QString nodeName,
    	Vector::DBC::Network *pNetwork
    	);
    static QList<Vector::DBC::Message *> getRxMessages4Node(
    	const QString nodeName,
    	Vector::DBC::Network *pNetwork
    	);
    static QString getSignalDescriptionValue(const Vector::DBC::Signal *pSignal, unsigned int key);
    static int getSignalDescriptionKey(const Vector::DBC::Signal *pSignal, QString value);
    static QStringList getSignalDescriptionValues(const Vector::DBC::Signal *pSignal);
	static QStringList getSignalDescriptions(const Vector::DBC::Signal *pSignal);
	static double getAttributeValueNum(const Vector::DBC::Signal *pSignal,
    	const Vector::DBC::Network *pNetwork,
    	const QString name, bool *ok);
	static Vector::DBC::AttributeValueType getAttributeDefinitionType(
    	Vector::DBC::AttributeDefinition::ObjectType objType,
    	QString name,
    	const Vector::DBC::Network *pNetwork
		);
	static double getDefaultSignalValue(
		const Vector::DBC::Signal *pSignal,
		const Vector::DBC::Network *pNetwork,
		bool *ok);
	static int getMessageCycleTime(const Vector::DBC::Message *pMessage,
    	const Vector::DBC::Network *pNetwork, const quint8 *data);
	static int getMessageCycleTimeStrict(const Vector::DBC::Message *pMessage,
    	const Vector::DBC::Network *pNetwork, const quint8 *data);

};

#endif // DBCHELPER_H