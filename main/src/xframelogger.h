#ifndef XFRAMELOGGER_H
#define XFRAMELOGGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMutexLocker>
#include <QDebug>

#include "xbusframe.h"

#define FRAMELOG_EXT    "bf"

template<typename T>
class FrameTraits;
template<>
class FrameTraits<int> {
public:
    static QString stringlize(int &f) {
        return QString::number(f);
    }
    static int unstringlize(const QString &s, int *f) {
        *f = s.toInt();
        return 0;
    }
};

template<>
class FrameTraits<const XBusFrame> {
public:
    static QString stringlize(const XBusFrame &f, qint64 baseTime) {
		QString s = f.toString(baseTime);
        return s;
    }
    static int unstringlize(const QString &s, XBusFrame *f) {
        //*f = s.toInt();
        return 0;
    }
};

class XFrameLogger : public QObject
{
    Q_OBJECT

public:
    explicit XFrameLogger(QObject *parent = 0);
    virtual ~XFrameLogger();
    void startLog(const QString &fileName, int maxFileSize = 0, int maxBackupFiles = 0);
    void stopLog();

    template<typename T>
    inline int readFrame(T &f) {
        //QString s = "12";
        //int rc = FrameTraits<T>::unstringlize(s, f);
        int rc = 0;
        return rc;
    }

    template<typename T>
    inline void writeFrame(T &f, qint64 baseTime) {
        if (!m_isLogging) return;

        QMutexLocker locker(&m_loggerMutex);

        if (m_isFirstWrite) {
            if (openFile() == -1)
                return;
            m_isFirstWrite = false;
        } else if ((m_maxFileSize > 0) && (m_fileSize > m_maxFileSize)) {
            if (rollOver() == -1)
                return;
        }

        QString line = FrameTraits<T>::stringlize(f, baseTime) + "\n";
        m_fileSize += line.size();
        (*m_tsFile) << line;
        //m_tsFile->flush();
    }

signals:

public slots:

private:
    void init(const QString &fileName, int maxFileSize, int maxBackupFiles);
    int rollOver();
    int openFile();
    void closeFile();
    QString buildFileName(int fileIndex = 0);

    int m_maxBackupFiles;
    int m_maxFileSize;
    QString m_fileBase;
    QString m_fileExt;
    QFile *m_file;
    QTextStream *m_tsFile;
    int m_fileSize;
    bool m_isFirstWrite;
    bool m_isLogging;
    QMutex m_loggerMutex;
};

#endif // XFRAMELOGGER_H
