#pragma once

#include <QLoggingCategory>
#include <QDebug>
#include <QString>


Q_DECLARE_LOGGING_CATEGORY(rrLogCat)


namespace RRLog {
void install (bool alsoWriteToFile = true, const QString &filePath = QString());
QString logFilePath();
}

#ifdef RR_LOG_ENABLED
#  define RR_LOG_D() QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO).debug(rrLogCat)
#  define RR_LOG_I() QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO).info(rrLogCat)
#  define RR_LOG_W() QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO).warning(rrLogCat)
#  define RR_LOG_E() QMessageLogger(__FILE__, __LINE__, Q_FUNC_INFO).critical(rrLogCat)
#else
#  define RR_LOG_D() QT_NO_QDEBUG_MACRO()
#  define RR_LOG_I() QT_NO_QDEBUG_MACRO()
#  define RR_LOG_W() QT_NO_QDEBUG_MACRO()
#  define RR_LOG_E() QT_NO_QDEBUG_MACRO()
#endif

#define RR_LOG() RR_LOG_D()