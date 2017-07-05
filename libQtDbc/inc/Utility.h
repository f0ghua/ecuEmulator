#ifndef UTILITY_H
#define UTILITY_H

#include <stdint.h>

#include <Qt>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>

#include "ByteOrder.h"

namespace Vector {
namespace DBC {

class Utility
{
public:

    static bool decimalMode;

    static uint64_t ParseStringToNum(QByteArray input)
    {
        uint64_t temp = 0;

        input = input.toUpper();
        if (input.startsWith("0X") || input.startsWith("X")) //hex number
        {
            if (input.length() < 3) temp = 0;
            else temp = input.right(input.size() - 2).toLongLong(NULL, 16);
        }
        else if (input.startsWith("B")) //binary number
        {
            input = input.right(input.size() - 1); //remove the B
            for (int i = 0; i < input.length(); i++)
            {
                if (input[i] == '1') temp += (uint64_t)1 << (input.length() - i - 1);
            }
        }
        else //decimal number
        {
            temp = input.toLongLong();
        }

        return temp;
    }

    static uint64_t ParseStringToNum(QString input)
    {
        return ParseStringToNum(input.toUtf8());
    }

    static long GetTimeMS()
    {
        QDateTime stamp = QDateTime::currentDateTime();
        return (long)(((stamp.time().hour() * 3600) + (stamp.time().minute() * 60) + (stamp.time().second()) * 1000) + stamp.time().msec());
    }

    //prints hex numbers in uppercase with 0's filling out the number depending
    //on the size needed. Promotes hex numbers to either 2, 4, or 8 digits
    static QString formatHexNum(uint64_t input)
    {
        if (input < 256)
            return "0x" + QString::number(input, 16).toUpper().rightJustified(2,'0');
        if (input < 65536)
            return "0x" + QString::number(input, 16).toUpper().rightJustified(4,'0');
        if (input < 4294967296)
            return "0x" + QString::number(input, 16).toUpper().rightJustified(8,'0');
        return "0x" + QString::number(input, 16).toUpper().rightJustified(16,'0');
    }

    //uses decimalMode to see if it should show value as decimal or hex
    static QString formatNumber(uint64_t value)
    {
        if (decimalMode)
        {
            return QString::number(value, 10);
        }
        else return formatHexNum(value);
    }

    static QString formatByteAsBinary(uint8_t value)
    {
        QString output;
        for (int b = 7; b >= 0; b--)
        {
            if (value & (1 << b)) output += "1";
            else output += "0";
        }
        return output;
    }

    //parses the input string to grab as much of it as possible while staying alpha numeric
    static QString grabAlphaNumeric(QString &input)
    {
        QString builder;
        QChar thisChar;
        for (int i = 0; i < input.length(); i++)
        {
            thisChar = input[i];
            if (thisChar.isLetterOrNumber() || thisChar == ':' || thisChar == '~') builder.append(input[i]);
            else
            {
                //qDebug() << "i: "<< i << " len: " << input.length();
                if (i < (input.length() - 1)) input = input.right(input.length() - i);
                else input = "";
                return builder;
            }
        }
        //qDebug() << "Reached end of string in grabAlphaNumeric";
        input = "";
        return builder;
    }

    static QString grabOperation(QString &input)
    {
        QString builder;
        QChar thisChar = input[0];

        if (thisChar == '+' || thisChar == '-' || thisChar == '*' || thisChar == '/' || thisChar == '^' || thisChar == '&' || thisChar == '|' || thisChar == '=' || thisChar == '%')
        {
            input = input.right(input.length() - 1);
            builder = thisChar;
        }
        return builder;
    }

    //simple linear interpolation between value1 and value2. sample point is 0.0 to 1.0
    static double Lerp(double value1, double value2, double samplePoint)
    {
        return (value1 * (1.0 - samplePoint)) + (value2 * samplePoint);
    }

    static int64_t processIntegerSignal(const uint8_t *data, int startBit, int sigSize, ByteOrder byteOrder, bool isSigned)
    {

        int64_t result = 0;
        int bit;

        if (byteOrder == ByteOrder::LittleEndian)
        {
            bit = startBit;
            for (int bitpos = 0; bitpos < sigSize; bitpos++)
            {
/*
               Bits
      7  6  5  4  3  2  1  0

  0   7  6  5  4  3  2  1  0 <-- LOW
b 1   15 14 13 12 11 10 9  8
y 2   23 22 21 20 19 18 17 16
t 3   31 30 29 28 27 26 25 24
e 4   39 38 37 36 35 34 33 32
s 5   47 46 45 44 43 42 41 40
  6   55 54 53 52 51 50 49 48
  7 < 63 62 61 60 59 58 57 56
*/
                if (data[bit / 8] & (1 << (bit % 8)))
                    result += (1ULL << bitpos);
                bit++;
            }
        }
        else //motorola / big endian mode
        {
            bit = startBit;
            for (int bitpos = 0; bitpos < sigSize; bitpos++)
            {
/*
               Bits
      7  6  5  4  3  2  1  0

  0 < 7  6  5  4  3  2  1  0
b 1   15 14 13 12 11 10 9  8
y 2   23 22 21 20 19 18 17 16
t 3   31 30 29 28 27 26 25 24
e 4   39 38 37 36 35 34 33 32
s 5   47 46 45 44 43 42 41 40
  6   55 54 53 52 51 50 49 48
  7   63 62 61 60 59 58 57 56 <-- LOW
*/
                if (data[bit / 8] & (1 << (bit % 8)))
                    result += (1ULL << (sigSize - bitpos - 1));

                if ((bit % 8) == 0)
                    bit += 15;
                else bit--;

            }
        }

        if (isSigned)
        {
            int64_t mask = (1ULL << (sigSize - 1));
            if ((result & mask) == mask) //is the highest bit possible for this signal size set?
            {
                /*
                 * if so we need to also set every bit higher in the result int too.
                 * This leads to the below two lines that are nasty. Here's the theory behind that...
                 * If the value is signed and the highest bit is set then it is negative. To create
                 * a negative value out of this even though the variable result is 64 bit we have to
                 * run 1's all of the way up to bit 63 in result. -1 is all ones for whatever size integer
                 * you have. So, it's 64 1's in this case.
                 * signedMask is done this way:
                 * first you take the signal size and shift 1 up that far. Then subtract one. Lets
                 * see that for a 16 bit signal:
                 * (1 << 16) - 1 = the first 16 bits set as 1's. So far so good. We then negate the whole
                 * thing which flips all bits. Thus signedMask ends up with 1's everwhere that the signal
                 * doesn't take up in the 64 bit signed integer result. Then, result has an OR operation on
                 * it with the old value and -1 masked so that the the 1 bits from -1 don't overwrite bits from the
                 * actual signal. This extends the sign bits out so that the integer result reads as the proper negative
                 * value. We dont need to do any of this if the sign bit wasn't set.
                */
                uint64_t signedMask = ~((1ULL << sigSize) - 1);
                result = (-1LL & signedMask) | result;
            }
        }

        return result;
    }
};

}
}

#endif // UTILITY_H
