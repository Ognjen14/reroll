cmake_minimum_required(VERSION 3.16)

project(reroll VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Network Qml)

qt_standard_project_setup(REQUIRES 6.10)

option(RR_LOG "Enable RR_LOG logging" ON)

if(ANDROID)
    include(FetchContent)
    FetchContent_Declare(
        android_openssl
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        URL https://github.com/KDAB/android_openssl/archive/refs/heads/master.zip
    )
    FetchContent_MakeAvailable(android_openssl)
    include("${android_openssl_SOURCE_DIR}/android_openssl.cmake")
endif()


qt_add_executable(appReroll
    main.cpp

    #SORUCE
    src/AppSettings.cpp
    src/AndroidSystem.cpp
    src/RRLog.cpp

    src/Controllers/DiscoverController.cpp
    src/Controllers/HomeController.cpp
    src/Controllers/MyListController.cpp

    src/Domain/Candidate.cpp
    src/Domain/CandidatePool.cpp
    src/Domain/CandidateScorer.cpp
    src/Domain/FilterCriteria.cpp
    src/Domain/MyListEntry.cpp
    src/Domain/ScoringConfig.cpp
    src/Domain/SuggestionSession.cpp
    src/Domain/TitleSnapshot.cpp
    src/Domain/WeightedPicker.cpp

    src/ViewModels/Models/GenreListModel.cpp
    src/ViewModels/Models/GenreSectionListModel.cpp
    src/ViewModels/Models/MyListFilterModel.cpp
    src/ViewModels/Models/TitleListModel.cpp

    src/Infrastructure/ImageNetworkFactory.cpp
    src/Infrastructure/JsonStore.cpp
    src/Infrastructure/PosterUrlResolver.cpp
    src/Infrastructure/RuntimeConfig.cpp
    src/Infrastructure/TmdbClient.cpp
    src/Infrastructure/TmdbError.cpp
    src/Infrastructure/TmdbJsonReader.cpp
    src/Infrastructure/TmdbMapper.cpp
    src/Infrastructure/TmdbRequestBuilder.cpp
    src/Infrastructure/TmdbRuntimeConfig.cpp

    #INCLUDE
    include/AppSettings.h
    include/AndroidSystem.h
    include/RRLog.h

    include/Controllers/DiscoverController.h
    include/Controllers/HomeController.h
    include/Controllers/MyListController.h

    include/Domain/Candidate.h
    include/Domain/CandidateIdentity.h
    include/Domain/CandidatePool.h
    include/Domain/CandidateScorer.h
    include/Domain/FilterCriteria.h
    include/Domain/GenreMatchMode.h
    include/Domain/MediaType.h
    include/Domain/MediaTypeFilter.h
    include/Domain/MyListEntry.h
    include/Domain/ScoringConfig.h
    include/Domain/SuggestionSession.h
    include/Domain/TitleSnapshot.h
    include/Domain/WeightedPicker.h

    include/Infrastructure/ImageNetworkFactory.h
    include/Infrastructure/JsonStore.h
    include/Infrastructure/PosterUrlResolver.h
    include/Infrastructure/RuntimeConfig.h
    include/Infrastructure/TmdbClient.h
    include/Infrastructure/TmdbDtos.h
    include/Infrastructure/TmdbError.h
    include/Infrastructure/TmdbJsonReader.h
    include/Infrastructure/TmdbMapper.h
    include/Infrastructure/TmdbRequestBuilder.h
    include/Infrastructure/TmdbRuntimeConfig.h

    include/ViewModels/Models/GenreListModel.h
    include/ViewModels/Models/GenreSectionListModel.h
    include/ViewModels/Models/MyListFilterModel.h
    include/ViewModels/Models/TitleListModel.h
)

target_include_directories(appReroll
    PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_features(appReroll
    PRIVATE cxx_std_17
)

target_compile_definitions(appReroll
    PRIVATE $<$<BOOL:${RR_LOG}>:RR_LOG_ENABLED>
)

set_source_files_properties(qml/Singletons/AppTheme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
set_source_files_properties(qml/Singletons/PopupRegistry.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE)

qt_add_qml_module(appReroll
    URI reroll
    QML_FILES
        qml/Main.qml

        #Singletons
        qml/Singletons/AppTheme.qml
        qml/Singletons/PopupRegistry.qml

        #Controls
        qml/Controls/ThemedIcon.qml
        qml/Controls/ThemePicker.qml
        qml/Controls/AccentPicker.qml
        qml/Controls/FontSizePicker.qml

        qml/Controls/NavigationBar.qml

        qml/Controls/AppButton.qml
        qml/Controls/DiscoverPosterCard.qml
        qml/Controls/DiscoverPosterCardDelegate.qml
        qml/Controls/GenreChip.qml
        qml/Controls/GenreChipDelegate.qml
        qml/Controls/GenreMatchModeSelector.qml
        qml/Controls/GenreTag.qml
        qml/Controls/MediaTypeSelector.qml
        qml/Controls/PosterImage.qml
        qml/Controls/PosterPlaceholder.qml
        qml/Controls/RangeSliderInput.qml
        qml/Controls/RerollButton.qml
        qml/Controls/SegmentedOption.qml
        qml/Controls/SegmentedSelector.qml
        qml/Controls/SettingsLinkRow.qml
        qml/Controls/SliderInput.qml
        qml/Controls/StatePanel.qml
        qml/Controls/SuggestionActionBar.qml
        qml/Controls/SuggestionCard.qml
        qml/Controls/TmdbAttribution.qml
        qml/Controls/Toast.qml
        qml/Controls/ToggleSwitch.qml
        qml/Controls/UnappliedChangesNotice.qml

        #Drawers
        qml/Drawers/FilterDrawer.qml
        qml/Drawers/TitleDetailsDrawer.qml

        #Pages
        qml/Pages/SettingsPage.qml
        qml/Pages/DiscoverPage.qml
        qml/Pages/GenreDetailPage.qml
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
    assets/tmdb_logo_blue.svg
    assets/start_rating.png
    assets/alert-circle.svg

    assets/nav/discover.svg
    assets/nav/home.svg
    assets/nav/my_list.svg
    assets/nav/settings.svg

    assets/reroll_page/mark_watched.png
    assets/reroll_page/marked_watched.png
    assets/reroll_page/whichlist.png
    assets/reroll_page/whichlisted.png
    assets/reroll_page/trailler_play.svg
    assets/reroll_page/reroll.png
    assets/reroll_page/x.png
)

set_target_properties(appReroll PROPERTIES
    WIN32_EXECUTABLE TRUE
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
)

target_link_libraries(appReroll
    PRIVATE Qt6::Quick Qt6::Network Qt6::Qml
)

if(ANDROID)
    add_android_openssl_libraries(appReroll)
endif()

install(TARGETS appReroll
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
