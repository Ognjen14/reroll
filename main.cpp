#include <QGuiApplication>
#include <QNetworkAccessManager>
#include <QQmlApplicationEngine>
#include <QStandardPaths>
#include "include/AppSettings.h"
#include "include/AndroidSystem.h"
#include "include/RRLog.h"
#include "include/Controllers/DiscoverController.h"
#include "include/Controllers/HomeController.h"
#include "include/Controllers/MyListController.h"
#include "include/Infrastructure/ImageNetworkFactory.h"
#include "include/Infrastructure/JsonStore.h"
#include "include/Infrastructure/PosterUrlResolver.h"
#include "include/Infrastructure/RuntimeConfig.h"
#include "include/Infrastructure/TmdbClient.h"
#include "include/Infrastructure/TmdbRequestBuilder.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    app.setOrganizationName("TopicDev");
    app.setApplicationName("Reroll");
    app.setApplicationVersion("1.1.3");
    RRLog::install(true, QString());

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

    static QNetworkAccessManager networkAccessManager;
    static Reroll::Infrastructure::RuntimeConfig runtimeConfig;
    static Reroll::Infrastructure::TmdbRequestBuilder requestBuilder(runtimeConfig);
    static Reroll::Infrastructure::TmdbClient tmdbClient(networkAccessManager, requestBuilder);
    static Reroll::Infrastructure::PosterUrlResolver posterUrlResolver(tmdbClient);

    static Reroll::Infrastructure::JsonStore jsonStore;
    jsonStore.load();

    static Reroll::Controllers::HomeController homeController(
        tmdbClient, jsonStore, posterUrlResolver);
    static Reroll::Controllers::MyListController myListController(
        jsonStore, posterUrlResolver);
    homeController.setEligibilityFilter(
        [](const Reroll::Domain::CandidateIdentity &identity) {
            if (myListController.isHidden(identity))
            {
                return false;
            }
            return !homeController.appliedExcludeWatched()
                || !myListController.isWatched(identity);
        });

    static Reroll::Controllers::DiscoverController discoverController(
        tmdbClient, posterUrlResolver);

    qmlRegisterSingletonInstance("com.topicdev.reroll",1,0,"HomeController",&homeController);
    qmlRegisterSingletonInstance("com.topicdev.reroll",1,0,"MyListController",&myListController);
    qmlRegisterSingletonInstance("com.topicdev.reroll",1,0,"DiscoverController",&discoverController);
    qmlRegisterSingletonInstance("com.topicdev.reroll",1,0,"PosterUrlResolver",&posterUrlResolver);

    static Reroll::Infrastructure::ImageNetworkFactory imageNetworkFactory(
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/posters");

    QQmlApplicationEngine engine;
    engine.setNetworkAccessManagerFactory(&imageNetworkFactory);
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("reroll", "Main");

    homeController.start();
    discoverController.start();

    return QGuiApplication::exec();
}
