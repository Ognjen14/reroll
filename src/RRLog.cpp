 #include "include/RRLog.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMutex>
#include <QStandardPaths>
#include <QTextStream>


Q_LOGGING_CATEGORY(rrLogCat, "rr", QtDebugMsg)

namespace{

constexpr qint64 kMaxBytes = 2 * 1024 * 1024;

QtMessageHandler s_previous = nullptr;
QFile s_file;
QMutex s_mutex;

void rotateIfNeeded(){
    if(s_file.size() < kMaxBytes){
        return;
    }

    const QString path = s_file.fileName();
    QFile::remove(path + QStringLiteral(".1"));
    QFile::rename(path, path + QStringLiteral(".1"));
    s_file.setFileName(path);
    bool ok = s_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if(!ok){
        qCritical() << "RR_LOG file didn't opened after removing FULL ONE(2MB)";
    }
}

void rrMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    if (s_previous)
        s_previous(type, ctx, msg);         // console on desktop, logcat on Android

    QMutexLocker locker(&s_mutex);
    if (!s_file.isOpen())
        return;
    QTextStream out(&s_file);
    out << qFormatLogMessage(type, ctx, msg) << '\n';   // same pattern as console
    out.flush();
    rotateIfNeeded();
}
} //namespace


void RRLog::install(bool alsoWriteToFile, const QString &filePath)
{
#ifdef RR_LOG_ENABLED
    static bool installed = false;
    if (installed)
        return;
    installed = true;

    const QString path = filePath.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/logs/reroll.log" : filePath;

    qInfo() << path << filePath;

    if (qEnvironmentVariableIsEmpty("QT_MESSAGE_PATTERN")) {
        qSetMessagePattern(QStringLiteral(
            "[%{time hh:mm:ss.zzz}]"
            "[%{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}"
            "%{if-critical}E%{endif}%{if-fatal}F%{endif}]"
            "[%{category}] %{message}   (%{file}:%{line})"));
    }

    if (alsoWriteToFile) {
        QDir().mkpath(QFileInfo(path).absolutePath());
        s_file.setFileName(path);
        bool ok = s_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        if(!ok){
            qCritical() << "RR_LOG file didn't open on INSTALL!";
        }
    }

    s_previous = qInstallMessageHandler(rrMessageHandler);
#else
    Q_UNUSED(alsoWriteToFile)
#endif
}

QString RRLog::logFilePath()
{
    QMutexLocker locker(&s_mutex);
    return s_file.isOpen() ? s_file.fileName() : QString();
}