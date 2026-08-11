#include "include/AndroidSystem.h"

#ifdef Q_OS_ANDROID
# include <QJniObject>
# include <QCoreApplication>
#endif

AndroidSystem::AndroidSystem(QObject *parent) : QObject(parent) {}

void AndroidSystem::setLightStatusBar(bool light)
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(
        "com/ognje/reroll/RerollActivity",
        "setLightSystemBars",
        "(Z)V",
        static_cast<jboolean>(light)
        );
#else
    Q_UNUSED(light)
#endif
}

void AndroidSystem::setBarBackgroud(const QColor &color)
{
#ifdef Q_OS_ANDROID
    QJniObject::callStaticMethod<void>(
        "com/ognje/reroll/RerollActivity",
        "setBarBackground",
        "(I)V",
        static_cast<jint>(color.rgba())
        );
#else
    Q_UNUSED(color)
#endif
}

bool AndroidSystem::isSystemDarkMode() const
{
#ifdef Q_OS_ANDROID
    QJniObject context(QNativeInterface::QAndroidApplication::context());
    if (!context.isValid()) return false;

    QJniObject resources = context.callObjectMethod(
        "getResources", "()Landroid/content/res/Resources;");
    if (!resources.isValid()) return false;

    QJniObject config = resources.callObjectMethod(
        "getConfiguration", "()Landroid/content/res/Configuration;");
    if (!config.isValid()) return false;

    jint uiMode = config.getField<jint>("uiMode");
    return (uiMode & 0x30) == 0x20;
#else
    // Desktop fallback — Qt 6 can read the system palette on most platforms
    return QLocale::system().measurementSystem() == QLocale::MetricSystem
               ? false   // not a reliable signal, just return false
               : false;
#endif
}

