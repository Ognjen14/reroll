cmake_minimum_required(VERSION 3.16)

project(reroll VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup(REQUIRES 6.10)

option(RR_LOG "Enable RR_LOG logging" ON)


qt_add_executable(appReroll
    main.cpp

    #SORUCE
    src/AppSettings.cpp
    src/AndroidSystem.cpp
    src/RRLog.cpp

    #INCLUDE
    include/AppSettings.h
    include/AndroidSystem.h
    include/RRLog.h
)

target_compile_definitions(appReroll
    PRIVATE $<$<BOOL:${RR_LOG}>:RR_LOG_ENABLED>
)

set_source_files_properties(qml/Singletons/AppTheme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(appReroll
    URI reroll
    QML_FILES
        qml/Main.qml

        #Singletons
        qml/Singletons/AppTheme.qml

        #Controls
        qml/Controls/ThemedIcon.qml
        qml/Controls/ThemePicker.qml
        qml/Controls/AccentPicker.qml
        qml/Controls/FontSizePicker.qml

        qml/Controls/NavigationBar.qml

        #Pages
        qml/Pages/SettingsPage.qml
        qml/Pages/DiscoverPage.qml
        qml/Pages/MyList.qml
        qml/Pages/HomePage.qml
)

qt_add_resources(appReroll "ASSETS"
    PREFIX "/"

    FILES
    assets/moon_dark_theme.svg
    assets/sun_light_theme.svg
    assets/system_theme.svg
    assets/check.png

    assets/nav/discover.svg
    assets/nav/home.svg
    assets/nav/my_list.svg
    assets/nav/settings.svg
    assets/nav/taste.svg
)

set_target_properties(appReroll PROPERTIES
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

target_link_libraries(appReroll
    PRIVATE Qt6::Quick
)

install(TARGETS appReroll
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
