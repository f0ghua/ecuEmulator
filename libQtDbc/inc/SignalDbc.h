#pragma once

#include "platform.h"
#include "vector_dbc_export.h"

#include "Attribute.h"
#include "ByteOrder.h"
#include "ExtendedMultiplexor.h"
#include "ValueDescriptions.h"
#include "ValueType.h"

namespace Vector {
namespace DBC {

class Message;

/**
 * Signal (SG)
 */
class VECTOR_DBC_EXPORT Signal
{
public:
    Signal();

    /** Name */
    QString name;

    /** Multiplexed Signal (m) */
    bool multiplexedSignal; // m

    /** Multiplexer Switch Value */
    unsigned int multiplexerSwitchValue;

    /** Multiplexor Switch (M) */
    bool multiplexorSwitch; // M

    /** Start Bit */
    unsigned int startBit;

    /** Bit Size */
    unsigned int bitSize;

    /** Byte Order */
    ByteOrder byteOrder;

    /** Value Type */
    ValueType valueType;

    /** Factor */
    double factor;

    /** Offset */
    double offset;

    /** Minimun Physical Value (or 0 if auto calculated) */
    double minimumPhysicalValue;

    /** Maximum Physical Value (or 0 if auto calculated) */
    double maximumPhysicalValue;

    /** Unit */
    QString unit;

    /** Receivers */
    QStringList receivers;

    /** Signal Extended Value Type (SIG_VALTYPE, obsolete) */
    enum class ExtendedValueType : char {
        Undefined = ' ',
        Integer = '0',
        Float = '1',
        Double = '2'
    };

    /** Signal Extended Value Type (SIG_VALTYPE, obsolete) */
    ExtendedValueType extendedValueType;

    /** Value Descriptions (VAL) */
    ValueDescriptions valueDescriptions;

    /** Signal Type Refs (SGTYPE, obsolete) */
    QString type;

    /** Comment (CM) */
    QString comment;

    /** Attribute Values (BA) */
    QMap<QString, Attribute> attributeValues;

    /** Extended Multiplexors (SG_MUL_VAL) */
    QList<ExtendedMultiplexor> extendedMultiplexors;

    Message *parentMessage;

    /**
     * @brief Convert from Raw to Physical Value
     * @param[in] rawValue Raw Value
     * @return Raw Value
     *
     * Converts a value from raw to physical representation.
     */
    double rawToPhysicalValue(double rawValue) const;

    /**
     * @brief Convert from Physical to Raw Value
     * @param[in] physicalValue Physical Value
     * @return Physical Value
     *
     * Converts a value from physical to raw representation.
     */
    double physicalToRawValue(double physicalValue) const;

#if 0
    /**
     * @brief Get minimum Physical Value
     * @return Minimum Physical Value
     *
     * Based on size, valueType and extendedValueType this calculates the minimum raw value.
     */
    double minimumRawValue();

    /**
     * @brief Get maximum Raw Value
     * @return Maximum Raw Value
     *
     * Based on size, valueType and extendedValueType this calculates the maximum raw value.
     */
    double maximumRawValue();
#endif

    /**
     * @brief Decodes/Extracts a signal from the message data
     * @param[in] data Data
     * @return Raw signal value
     *
     * Decodes/Extracts a signal from the message data.
     *
     * @note Multiplexors are not taken into account.
     */
    uint64_t decode(const uint8_t *msgData) const;

    /**
     * @brief Encodes a signal into the message data
     * @param[inout] data Data
     * @param[in] rawValue Raw signal value
     *
     * Encode a signal into the message data.
     *
     * @note Multiplexors are not taken into account.
     */
    void encode(uint8_t *msgData, uint64_t rawValue) const;

    bool processAsText(const uint8_t *msgData, QString &outString) const;
    bool processAsInt(const uint8_t *msgData, int32_t &outValue) const;
    bool processAsDouble(const uint8_t *msgData, double &outValue) const;
	double minimumRawValue() const;
	double maximumRawValue() const;
	double minimumPhyValue() const;
	double maximumPhyValue() const;
	
    void encodePhy(uint8_t *msgData, uint64_t phyValue) const;
    double decodePhy(const uint8_t *msgData) const;
};

}
}
