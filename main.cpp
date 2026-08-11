#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "include/AppSettings.h"
#include "include/AndroidSystem.h"
#include "include/RRLog.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationName("TopicDev");
    app.setApplicationName("Reroll");
    app.setApplicationVersion("0.0.1");
    RRLog::install(true,"F:/topic_dev/logs/reroll.log");
    RR_LOG_I() << "log file:" << RRLog::logFilePath();

    static AppSettings appSettings;
    qmlRegisterSingletonInstance("com.topicdev.reroll",1,0,"AppSettings",&appSettings);
    qmlRegisterType<AndroidSystem>("com.topicdev.reroll", 1, 0, "AndroidSystem");
    {
        AndroidSystem sysInfo;
        if(!appSettings.themeInitialized()){
            const bool sysDark =  sysInfo.isSystemDarkMode();
            appSettings.setDarkMode(sysDark);
            appSettings.setThemeMode(sysDark ? QStringLiteral("dark")
                                             : QStringLiteral("light"));
        }
    }
    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("reroll", "Main");

    return QGuiApplication::exec();
}
