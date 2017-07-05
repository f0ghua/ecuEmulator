#pragma once

#include <Qt>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>

namespace Utils {
class Base
{
	public:

		static bool decimalMode;
		static quint8 linVersion;
		static bool isSaintDevice;

		static uint64_t parseStringToNum(QByteArray input)
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

		static uint64_t parseStringToNum(QString input)
		{
			return parseStringToNum(input.toUtf8());
		}

		// either with "0x" prepend or not return the right value
		static uint64_t parseHexStringToNum(const QString &input)
		{
			return input.toLongLong(NULL, 16);
		}

		static uint64_t parseHexStringToNum(const QVariant &input)
		{
			return input.toString().toLongLong(NULL, 16);
		}

		static long getTimeMS()
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

		static QString formatHexNumber(uint64_t input)
		{
			if (input < 256)
				return QString::number(input, 16).toUpper().rightJustified(2,'0');
			if (input < 65536)
				return QString::number(input, 16).toUpper().rightJustified(4,'0');
			if (input < 4294967296)
				return QString::number(input, 16).toUpper().rightJustified(8,'0');
			return QString::number(input, 16).toUpper().rightJustified(16,'0');
		}

		static QString formatByteArray(const QByteArray *pba)
		{
			QString tempStr;

			for (int i = 0; i < pba->count(); i++)
			{
				tempStr.append(formatHexNumber(pba->at(i)&0xFF));
				if (i < pba->count()-1)
					tempStr.append(" ");
			}
			return tempStr;
		}

		static QString formatByteArrayML(const QByteArray *pba)
		{
			QString tempStr;

			for (int i = 0; i < pba->count(); i++)
			{
				tempStr.append(formatHexNumber(pba->at(i)&0xFF));
				if (i < pba->count()-1)
				{
					if ((i+1) % 16 == 0) tempStr.append("\n");
					else tempStr.append(" ");
				}
			}
			return tempStr;
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

		static QByteArray hexString2ByteArray(const QString &s)
		{
			QString tmpStr = s;
			tmpStr.simplified().remove(' ');
			return QByteArray::fromHex(s.toLatin1());
		}

		static uint64_t bcd2Dec(const QByteArray bcd)   
        {    
            uint64_t dec = 0;   
      
            for(int i = 0; i < bcd.count(); i++)   
            {   
                dec *= 10;
                dec += (bcd.at(i) >> 4) & 0x0F;
                dec *= 10;
                dec += bcd.at(i) & 0x0F;
            }   

            return dec;   
        }

        static QByteArray dec2Bcd(uint64_t dec)   
        {   
            int tmp; 
            QByteArray ba;
            
            while(dec > 0)
            {   
                tmp = dec % 100;   
                char c = ((tmp / 10) << 4) + ((tmp % 10) & 0x0F);   
                ba.append(c);
                dec /= 100;   
            }   

            QByteArray reverse;
            reverse.reserve(ba.size());
            for(int i = ba.size()-1; i >= 0; --i) 
                reverse.append(ba.at(i));
            return reverse;  
        }  

        static QString getDateTimeFormat1()
        {
            QDateTime time = QDateTime::currentDateTime();
            QString date = QLocale( QLocale::C ).toString(time, "yyyy-MM-dd hh:mm:ss");
            
            return date;
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
};

} // namespace Helper

