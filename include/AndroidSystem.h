#pragma once
#include <QObject>
#include <QString>
#include <QColor>
#include <QLocale>

class AndroidSystem : public QObject
{
    Q_OBJECT
public:
    explicit AndroidSystem(QObject * parent = nullptr);

    ///
    /// \brief setLightStatusBar
    /// \param light
    ///
    Q_INVOKABLE void setLightStatusBar(bool light);
    ///
    /// \brief setBarBackgroud
    /// \param color
    ///
    Q_INVOKABLE void setBarBackgroud(const QColor &color);
    ///
    /// \brief isSystemDarkMode
    /// \return Returns is android phone set to dark theme or not
    ///
    Q_INVOKABLE bool isSystemDarkMode() const;
    ///
    /// \brief minimizeApp
    /// Sends the task to the background, same as pressing Home.
    ///
    Q_INVOKABLE void minimizeApp();

signals:
    /// Emitted when the Android back button/gesture is pressed. QML decides
    /// whether to close a drawer, pop a page, or minimize the app.
    void backPressed();

private slots:
    /// Invoked by name via QMetaObject::invokeMethod from the JNI callback,
    /// which arrives on Android's UI thread — this hop lands it back on the
    /// Qt thread before emitting backPressed().
    void onBackInternal();
};
