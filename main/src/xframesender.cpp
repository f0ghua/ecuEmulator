#include "xframesender.h"
#include "utils.h"
#include "xbusmgr.h"

#include <QDebug>
#include <QCoreApplication>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#define CCHR_SPL '#'    // split char of command string
#define CCHR_IND '@'    // indicate char

const char g_keyEnable[] = "en@";
const char g_keyId[] = "id@";
const char g_keyBus[] = "bus@";
const char g_keyData[] = "data@";
const char g_keyTrigger[] = "tr@";
const char g_keyModifier[] = "mo@";

XFrameSender::XFrameSender(XBusMgr *mgr, QObject *parent) : QObject(parent)
{
    m_busMgr = mgr;
}

FrameSendData *XFrameSender::findSendDataById(quint32 id)
{
    QList<FrameSendData>::iterator end = m_sendingData.end();
    QList<FrameSendData>::iterator it;

    for (it = m_sendingData.begin(); it != end; ++it) {
        if ((*it).id() == id)
            return &(*it);
    }

    return NULL;
}

#ifdef Q_OS_WIN
// This function must to be called after moveToThread()
void XFrameSender::setWorkThreadPriority()
{
    HANDLE realProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _getpid());
    if (NULL == realProcessHandle) {
        return;
    }
    if (0 == SetPriorityClass(realProcessHandle, REALTIME_PRIORITY_CLASS)) {
        CloseHandle(realProcessHandle);
        return;
    }
    HANDLE currentThreadHandle = GetCurrentThread();
    HANDLE currentProcessHandle = GetCurrentProcess();
    HANDLE realThreadHandle(0);
    if (0 == DuplicateHandle(currentProcessHandle, currentThreadHandle, currentProcessHandle, &realThreadHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        qDebug() << "DuplicateHandle fail";
    }
    if (0 == SetThreadPriority(realThreadHandle, THREAD_PRIORITY_TIME_CRITICAL)) {
        qDebug() << "SetThreadPriority fail";
    }
#ifndef F_NO_DEBUG
    qDebug() << tr("set thread %1 to time critical priority success.").arg(GetCurrentThreadId());
#endif
    CloseHandle(realThreadHandle);
    CloseHandle(realProcessHandle);

    return;
}
#endif

void XFrameSender::stopThread()
{
#ifndef F_NO_DEBUG
    qDebug() << tr("got stop Thread signal");
#endif
    m_abortRun = true;
}

void XFrameSender::run()
{
#ifdef Q_OS_WIN
    //initialWorkThread();

    // setup waitable Win32 periodic timer
    HANDLE hTE = (HANDLE)CreateWaitableTimer(NULL, false, NULL);

    if (hTE != NULL) {
        LARGE_INTEGER tLT;
        tLT.QuadPart = 0;
        if (!SetWaitableTimer(hTE, &tLT, m_clockRate, NULL, NULL, true)) {
            CloseHandle(hTE);
            hTE = NULL;
        }
    }
#endif

    // wait for 10 ticks for stable timing
    for (int i = 0; i < 10; i++) {
#ifdef Q_OS_WIN
        if (hTE == NULL) Sleep(m_clockRate);
        else WaitForSingleObject(hTE, m_clockRate*10);
#else
        QThread::msleep(m_clockRate);
#endif
    }

    m_elapsedTimer.start();

    // start our timing loop, wait for abort flag
    while (!m_abortRun) {
#ifdef Q_OS_WIN
        if (hTE == NULL) Sleep(m_clockRate);
        else WaitForSingleObject(hTE, m_clockRate);
#else
        QThread::msleep(m_clockRate);
#endif

        // we should give chance for signal/slot and so on
        QCoreApplication::processEvents();

        // run timer tick after wait
        handleTick();
    }

#ifdef Q_OS_WIN
    // shut down Win32 timer
    if (hTE != NULL) CloseHandle(hTE);
#endif

    emit finished();
}

void XFrameSender::handleTick()
{
    qint64 elapsed64 = 0;

    qint64 currCounter = QDateTime::currentMSecsSinceEpoch();
    if (m_preTickCounter == 0) {
        elapsed64 = m_countPerMilliSecond;
    } else {
        elapsed64 = currCounter - m_preTickCounter;
    }
    m_preTickCounter = currCounter;

    processSendingData(elapsed64);
}

void XFrameSender::processSendingData(qint64 elapsed)
{
    FrameSendData *sendData;
    Trigger *trigger;

    //elapsed = m_elapsedTimer.restart();
    //if (elapsed == 0) elapsed = 1;

    //Modifier modifier;
    for (int i = 0; i < m_sendingData.count(); i++) {
        sendData = &m_sendingData[i];
        if (!sendData->enabled) continue; //abort any processing on this if it is not enabled.
        if (sendData->triggers.count() == 0) return;
        for (int j = 0; j < sendData->triggers.count(); j++) {
            trigger = &sendData->triggers[j];
            if (trigger->currCount >= trigger->maxCount) continue; //don't process if we've sent max frames we were supposed to
            if (!trigger->readyCount) continue; //don't tick if not ready to tick
            //is it time to fire?
            trigger->msCounter += elapsed; //gives proper tracking even if timer doesn't fire as fast as it should
            if (trigger->msCounter >= trigger->milliseconds) {
                trigger->msCounter = 0;
                sendData->count++;
                trigger->currCount++;
                doModifiers(i);
#ifndef F_NO_DEBUG
                //qDebug() << tr("About to try to send frame id = %1, bus = %2").\
                    arg(sendData->id(), 3, 16, QChar('0')).arg(sendData->bus());
#endif
                emit m_busMgr->sendRawFrame(&m_sendingData[i]);
                if (trigger->ID > 0) trigger->readyCount = false; //reset flag if this is a timed ID trigger
            }
        }
    }
}

void XFrameSender::doModifiers(int idx)
{
    int shadowReg = 0; //shadow register we use to accumulate results
    int first=0, second=0;

    FrameSendData *sendData = &m_sendingData[idx];
    Modifier *mod;
    ModifierOp op;

    if (sendData->modifiers.count() == 0) return; //if no modifiers just leave right now

    for (int i = 0; i < sendData->modifiers.count(); i++) {
        mod = &sendData->modifiers[i];
        for (int j = 0; j < mod->operations.count(); j++) {
            op = mod->operations.at(j);
            if (op.first.ID == -1) {
                first = shadowReg;
            }
            else first = fetchOperand(idx, op.first);
            second = fetchOperand(idx, op.second);
            switch (op.operation) {
            case ADDITION:
                shadowReg = first + second;
                break;
            case AND:
                shadowReg = first & second;
                break;
            case DIVISION:
                shadowReg = first / second;
                break;
            case MULTIPLICATION:
                shadowReg = first * second;
                break;
            case OR:
                shadowReg = first | second;
                break;
            case SUBTRACTION:
                shadowReg = first - second;
                break;
            case XOR:
                shadowReg = first ^ second;
                break;
            case MOD:
                shadowReg = first % second;
            }
        }
        //Finally, drop the result into the proper data byte
        sendData->payload()[mod->destByte] = (unsigned char) shadowReg;
    }
}

int XFrameSender::fetchOperand(int idx, ModifierOperand op)
{

    if (op.ID == 0) { //numeric constant
        if (op.notOper) return ~op.databyte;
        else return op.databyte;
    }
    else if (op.ID == -2) { //fetch data from a data byte within the output frame
        if (op.notOper) return ~m_sendingData.at(idx).payload()[op.databyte];
        else return m_sendingData.at(idx).payload()[op.databyte];
    }
    else {//look up external data byte
        //CANFrame *tempFrame = NULL;
        /*
        tempFrame = lookupFrame(op.ID, op.bus);
        if (tempFrame != NULL)
        {
            if (op.notOper) return ~tempFrame->data[op.databyte];
            else return tempFrame->data[op.databyte];
        }
        else return 0;
        */
        return 0;
    }
}

// en@0
// #id@0x061
// #bus@0
// #data@00112233
// #tr@id0x200 5ms 10x bus0,1000ms
// #mo@id:0x200:D7,D1=ID:0x200:D3+ID:0x200:D4&0xF0
void XFrameSender::slotCmdParser(const QString &cmdString)
{
    QString tokEnable, tokId, tokBus, tokData;
    QString tokTr, tokMod;

    if (cmdString.isEmpty())
        return;

    QStringList sl = cmdString.simplified().split(CCHR_SPL);
    for (int i = 0; i < sl.size(); ++i) {
        const QString &s = sl.at(i);

        if (s.startsWith(g_keyEnable)) {
            tokEnable = s.right(s.length() - strlen(g_keyEnable));
#ifndef F_NO_DEBUG
            qDebug() << tr("enable = %1").arg(tokEnable);
#endif
        } else if (s.startsWith(g_keyId)) {
            tokId = s.right(s.length() - strlen(g_keyId));
#ifndef F_NO_DEBUG
            qDebug() << tr("id = %1").arg(tokId);
#endif
        } else if (s.startsWith(g_keyBus)) {
            tokBus = s.right(s.length() - strlen(g_keyBus));
#ifndef F_NO_DEBUG
            qDebug() << tr("bus = %1").arg(tokBus);
#endif
        } else if (s.startsWith(g_keyData)) {
            tokData = s.right(s.length() - strlen(g_keyData));
#ifndef F_NO_DEBUG
            qDebug() << tr("data = %1").arg(tokData);
#endif
        } else if (s.startsWith(g_keyTrigger)) {
            tokTr = s.right(s.length() - strlen(g_keyTrigger));
#ifndef F_NO_DEBUG
            qDebug() << tr("trigger = %1").arg(tokTr);
#endif
        } else if (s.startsWith(g_keyModifier)) {
            tokMod = s.right(s.length() - strlen(g_keyModifier));
#ifndef F_NO_DEBUG
            qDebug() << tr("modifier = %1").arg(tokMod);
#endif
        }
    }

    if (tokId.isEmpty())
        return;

    bool ok;
    quint32 id = tokId.toUInt(&ok, 0);
    if (!ok) return;

    int bus = 0;
    if (!tokBus.isEmpty()) {
        bus = tokBus.toInt();
    }

    FrameSendData *pSd = findSendDataById(id);
    if (pSd == NULL) {
        FrameSendData tempSendData(bus);
        m_sendingData.append(tempSendData);
        pSd = &m_sendingData.last();
    }

    pSd->setBus(bus);
    pSd->setId(id);
    if (!tokData.isEmpty()) {
        QByteArray ba = QByteArray::fromHex(tokData.toLatin1());
        pSd->setPayload(ba);
    }

    if (!tokEnable.isEmpty()) {
        int en = tokEnable.toInt();
        pSd->enabled = en;
    }
    if (!tokTr.isEmpty()) {
        processTriggerText(tokTr, pSd);
    }
    if (!tokMod.isEmpty()) {
        processModifierText(tokMod, pSd);
    }
}

void XFrameSender::slotChangeData(quint32 id, const QByteArray &payload)
{
    FrameSendData *pSd = findSendDataById(id);
    if (pSd == NULL) {
        return;
    }

    pSd->setPayload(payload);
}

void XFrameSender::processModifierText(const QString &tokMod, FrameSendData *pSd)
{
    QString modString;
    //bool firstOp = true;
    bool abort = false;
    QString token;
    ModifierOp thisOp;

    //Example line:
    //d0 = D0 + 1,d1 = id:0x200:d3 + id:0x200:d4 AND 0xF0 - Original version
    //D0=D0+1,D1=ID:0x200:D3+ID:0x200:D4&0xF0
    //This is certainly much harder to parse than the trigger definitions.
    //the left side of the = has to be D0 to D7. After that there is a string of
    //data. Spaces used to be required but no longer are. This makes parsing harder but data entry easier

    //yeah, lots of operations on this one line but it's for a good cause. Removes the convenience English versions of the
    //logical operators and replaces them with the math equivs. Also uppercases and removes all superfluous whitespace
    modString = tokMod.toUpper().trimmed().replace("AND", "&").replace("XOR", "^").replace("OR", "|").replace(" ", "");
    if (modString != "") {
        QStringList mods = modString.split(',');
        pSd->modifiers.clear();
        pSd->modifiers.reserve(mods.length());
        for (int i = 0; i < mods.length(); i++) {
            Modifier thisMod;
            thisMod.destByte = 0;
            //firstOp = true;

            QString leftSide = Utils::Base::grabAlphaNumeric(mods[i]);
            if (leftSide.startsWith("D") && leftSide.length() == 2) {
                thisMod.destByte = leftSide.right(1).toInt();
                thisMod.operations.clear();
            } else {
                qDebug() << "Something wrong with lefthand val";
                continue;
            }
            if (!(Utils::Base::grabOperation(mods[i]) == "=")) {
                qDebug() << "Err: No = after lefthand val";
                continue;
            }
            abort = false;

            token = Utils::Base::grabAlphaNumeric(mods[i]);
            if (token[0] == '~') {
                thisOp.first.notOper = true;
                token = token.remove(0, 1); //remove the ~ character
            }
            else thisOp.first.notOper = false;
            parseOperandString(token.split(":"), thisOp.first);

            if (mods[i].length() < 2) {
                abort = true;
                thisOp.operation = ADDITION;
                thisOp.second.ID = 0;
                thisOp.second.databyte = 0;
                thisOp.second.notOper = false;
                thisMod.operations.append(thisOp);
            }

            while (!abort) {
                QString operation = Utils::Base::grabOperation(mods[i]);
                if (operation == "") {
                    abort = true;
                } else {
                    thisOp.operation = parseOperation(operation);
                    QString secondOp = Utils::Base::grabAlphaNumeric(mods[i]);
                    if (mods[i][0] == '~')
                    {
                        thisOp.second.notOper = true;
                        mods[i] = mods[i].remove(0, 1); //remove the ~ character
                    }
                    else thisOp.second.notOper = false;
                    thisOp.second.bus = pSd->bus();
                    thisOp.second.ID = pSd->id();
                    parseOperandString(secondOp.split(":"), thisOp.second);
                    thisMod.operations.append(thisOp);
                }

                thisOp.first.ID = -1; //shadow register
                if (mods[i].length() < 2) abort = true;
            }

            pSd->modifiers.append(thisMod);
        }
    }
    //there is no else for the modifiers. We'll accept there not being any
}

void XFrameSender::processTriggerText(const QString &tokTr, FrameSendData *pSd)
{
    QString trigger;

    //Example line:
    //id0x200 5ms 10x bus0,1000ms
    //trigger has two levels of syntactic parsing. First you split by comma to get each
    //actual trigger. Then you split by spaces to get the tokens within each trigger
    //trigger = ui->tableSender->item(line, 5)->text().toUpper().trimmed().replace(" ", "");
    trigger = tokTr.toUpper();
    if (trigger != "") {
        QStringList triggers = trigger.split(',');
        pSd->triggers.clear();
        pSd->triggers.reserve(triggers.length());
        for (int k = 0; k < triggers.length(); k++) {
            Trigger thisTrigger;
            //start out by setting defaults - should be moved to constructor for class Trigger.
            thisTrigger.bus = -1; //-1 means we don't care which
            thisTrigger.ID = -1; //the rest of these being -1 means nothing has changed it
            thisTrigger.maxCount = -1;
            thisTrigger.milliseconds = -1;
            thisTrigger.currCount = 0;
            thisTrigger.msCounter = 0;
            thisTrigger.readyCount = true;

            QStringList trigToks = triggers[k].split(' ');
            for (int x = 0; x < trigToks.length(); x++) {
                QString tok = trigToks.at(x);
                if (tok.left(2) == "ID") {
                    thisTrigger.ID = Utils::Base::parseStringToNum(tok.right(tok.length() - 3));
                    if (thisTrigger.maxCount == -1) thisTrigger.maxCount = 10000000;

                    if (thisTrigger.milliseconds == -1) thisTrigger.milliseconds = 0; //by default don't count, just send it upon trigger
                    thisTrigger.readyCount = false; //won't try counting until trigger hits
                } else if (tok.endsWith("MS")) {
                    thisTrigger.milliseconds = Utils::Base::parseStringToNum(tok.left(tok.length()-2));
                    if (thisTrigger.maxCount == -1) thisTrigger.maxCount = 10000000;
                    if (thisTrigger.ID == -1) thisTrigger.ID = 0;
                } else if (tok.endsWith("X")) {
                    thisTrigger.maxCount = Utils::Base::parseStringToNum(tok.left(tok.length() - 1));
                    if (thisTrigger.ID == -1) thisTrigger.ID = 0;
                    if (thisTrigger.milliseconds == -1) thisTrigger.milliseconds = 10;
                } else if (tok.startsWith("BUS")) {
                    thisTrigger.bus = Utils::Base::parseStringToNum(tok.right(tok.length() - 3));
                }
            }
            //now, find anything that wasn't set and set it to defaults
            if (thisTrigger.maxCount == -1) thisTrigger.maxCount = 10000000;
            if (thisTrigger.milliseconds == -1) thisTrigger.milliseconds = 100;
            if (thisTrigger.ID == -1) thisTrigger.ID = 0;
            pSd->triggers.append(thisTrigger);
        }
    } else { //setup a default single shot trigger
        Trigger thisTrigger;
        thisTrigger.bus = -1;
        thisTrigger.ID = 0;
        thisTrigger.maxCount = 1;
        thisTrigger.milliseconds = 10;
        pSd->triggers.append(thisTrigger);
    }
}

void XFrameSender::parseOperandString(QStringList tokens, ModifierOperand &operand)
{
    qDebug() << "parseOperandString";
    //example string -> bus:0:id:200:d3

    operand.bus = -1;
    operand.ID = -2;
    operand.databyte = 0;

    for (int i = 0; i < tokens.length(); i++) {
        if (tokens[i] == "BUS") {
            operand.bus = Utils::Base::parseStringToNum(tokens[++i]);
        } else if (tokens[i] == "ID") {
            operand.ID = Utils::Base::parseStringToNum(tokens[++i]);
        } else if (tokens[i].length() == 2 && tokens[i].startsWith("D")) {
            operand.databyte = Utils::Base::parseStringToNum(tokens[i].right(tokens[i].length() - 1));
        } else {
            operand.databyte = Utils::Base::parseStringToNum(tokens[i]);
            operand.ID = 0; //special ID to show this is a number not a look up.
        }
    }
}

ModifierOperationType XFrameSender::parseOperation(QString op)
{
    if (op == "+") return ADDITION;
    if (op == "-") return SUBTRACTION;
    if (op == "*") return MULTIPLICATION;
    if (op == "/") return DIVISION;
    if (op == "&") return AND;
    if (op == "|") return OR;
    if (op == "^") return XOR;
    if (op == "%") return MOD;
    return ADDITION;
}
