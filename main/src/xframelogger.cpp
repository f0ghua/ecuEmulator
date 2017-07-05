#include "xframelogger.h"

#include <QFile>
#include <QFileInfo>
#include <QDebug>

XFrameLogger::XFrameLogger(QObject *parent) :
    QObject(parent),
    m_file(NULL),
    m_tsFile(NULL),
    m_isLogging(false)
{

}

XFrameLogger::~XFrameLogger()
{
    stopLog();
}

void XFrameLogger::init(const QString &fileName, int maxFileSize, int maxBackupFiles)
{
    QFileInfo fi(fileName);
    m_fileBase = fi.path() + "/" + fi.completeBaseName();
    m_fileExt = fi.suffix();

    m_maxBackupFiles = maxBackupFiles;
    m_maxFileSize = maxFileSize;
    m_fileSize = 0;
    m_isFirstWrite = true;

    m_file = NULL;
    m_tsFile = NULL;
}

void XFrameLogger::startLog(const QString &fileName, int maxFileSize, int maxBackupFiles)
{
    init(fileName, maxFileSize, maxBackupFiles);
    m_isLogging = true;
}

void XFrameLogger::stopLog()
{
    m_isLogging = false;
    closeFile();
}

QString XFrameLogger::buildFileName(int fileIndex)
{
    QString fileName;

    fileName = m_fileBase;
    if (fileIndex > 0) {
        fileName += QString(".%1").arg(fileIndex);
    }
    fileName += QString(".%1").arg(m_fileExt);

    return fileName;
}

int XFrameLogger::openFile()
{
    QString fileName = buildFileName(); 
    m_file = new QFile(fileName);
    
    QFile::OpenMode mode = QIODevice::WriteOnly | QIODevice::Text;
    //mode |= QIODevice::Append;
	//mode |= QIODevice::Truncate;
    //mode |= QIODevice::Unbuffered;
    
    if (!m_file->open(mode)) {
        delete m_file;
        m_file = NULL;
        return -1;
    }

    m_tsFile = new QTextStream(m_file);
    m_fileSize = 0;

    return 0;
}

void XFrameLogger::closeFile()
{
    if (m_file) {
        delete m_tsFile;
        m_tsFile = NULL;

        m_file->close();
        delete m_file;
        m_file = NULL;
    }

    return;
}

int XFrameLogger::rollOver()
{
    closeFile();

    QString fileName = buildFileName(m_maxBackupFiles);
    QFile f(fileName);
    if (f.exists() && !f.remove()) {
        return -1;
    }

    for (int fileIdx = m_maxBackupFiles - 1; fileIdx >= 0; --fileIdx)
    {
        QString currentFileName = buildFileName(fileIdx);
        QString nextFileName = buildFileName(fileIdx + 1);
        QFile::rename(currentFileName, nextFileName);
    }

    return openFile();
}

