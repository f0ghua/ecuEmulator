#ifndef XFRAMESENDER_H
#define XFRAMESENDER_H

#include "xbusframe.h"

#include <QObject>
#include <QElapsedTimer>

class XBusMgr;

//Stores a single trigger.
class Trigger
{
public:
    bool readyCount; //ready to do millisecond ticks?
    int ID; //which ID to match against
    int milliseconds; //interval for triggering
    int msCounter; //how many MS have ticked since last trigger
    int maxCount; //max # of these frames to trigger for
    int currCount; //how many we have triggered for so far.
    int bus; //which bus to monitor (-1 if we aren't picky)
};

//referece for a source location for a single modifier.
//If ID is zero then this is a numeric operand. The number is then
//stored in databyte
//If ID is -1 then this is the temporary storage register. This is a shadow
//register used to accumulate the results of a multi operation modifier.
//if ID is -2 then this is a look up of our own data bytes stored in the class data.
class ModifierOperand
{
public:
    int ID;
    int bus;
    int databyte;
    bool notOper; //should a bitwise NOT be applied to this prior to doing the actual calculation?
};

//list of operations that can be done between the two operands
enum ModifierOperationType
{
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    AND,
    OR,
    XOR,
    MOD
};

//A single modifier operation
class ModifierOp
{
public:
    ModifierOperand first;
    ModifierOperand second;
    ModifierOperationType operation;
};

//All the operations that entail a single modifier. For instance D0+1+D2 is two operations
class Modifier
{
public:
    int destByte;
    QList<ModifierOp> operations;
};

//A single line from the data grid. Inherits from CANFrame so it stores a canbus frame
//plus the extra stuff for the data grid
class FrameSendData : public XBusFrame
{
public:
    FrameSendData(int bus) : XBusFrame(bus) {}
    
    bool enabled = false;
    int count = 0;
    QList<Trigger> triggers;
    QList<Modifier> modifiers;
};

class XFrameSender : public QObject
{
    Q_OBJECT
public:
    explicit XFrameSender(XBusMgr *mgr, QObject *parent = 0);

signals:
    void sendFrame(const XBusFrame *, int);
    void finished();

public slots:
    void slotCmdParser(const QString &cmdString);
    void slotChangeData(quint32 id, const QByteArray &payload);
#ifdef Q_OS_WIN
    void setWorkThreadPriority();
#endif
    void run();
    void stopThread();

private:
    void handleTick();
    void processSendingData(qint64 elapsed);
    void doModifiers(int idx);
    int fetchOperand(int idx, ModifierOperand op);
    FrameSendData *findSendDataById(quint32 id);
    void processModifierText(const QString &tokMod, FrameSendData *pSd);
    void processTriggerText(const QString &tokTr, FrameSendData *pSd);
    void parseOperandString(QStringList tokens, ModifierOperand &operand);
    ModifierOperationType parseOperation(QString op);

    XBusMgr *m_busMgr = NULL;
    QList<FrameSendData> m_sendingData;
    bool m_abortRun = false;
    int m_clockRate = 1; // ms
    qint64 m_countPerMilliSecond = 1;
    qint64 m_preTickCounter = 0;
    QElapsedTimer m_elapsedTimer;
};

#endif // XFRAMESENDER_H
