# Reroll

Reroll is a free, open-source movie and TV suggestion app. Instead of a
browsing grid, it gives you one suggestion at a time based on filters you
choose, and you reroll until something sticks.

No ads, no subscription, no in-app purchases - free forever, by design.

## How it works

- **Reroll (Home)** - set filters (media type, year range, minimum rating,
  genres, watched status) and get a single suggestion. Reroll for another,
  add it to your watchlist, mark it watched, play its trailer, or hide it
  so it never comes up again.
- **Discover** - browse Trending and Popular rows for movies and TV,
  search by title, or browse by genre. Tap any title for a details drawer
  with its overview, genres, rating, and where to stream it.
- **My List** - everything you've added to your watchlist or marked
  watched, filterable by list, media type, or hidden titles. Move a title
  between watchlist and watched, or remove it, right from the grid.
- **Settings** - light/dark/system theme, an accent color picker, font
  size scaling, and links to the app's Terms of Use and Privacy Policy.

## Tech stack

- **Qt 6.10.3** - Qt Quick / QML for the UI, C++17 for application logic.
- **[TMDB](https://www.themoviedb.org/)** - the only data source, used
  under its non-commercial API license. Every screen that shows TMDB data
  credits TMDB and links back to themoviedb.org, per their API terms.
- **CMake** - build system, via `qt_add_executable` / `qt_add_qml_module`.
- Desktop (Windows, MinGW) for development; Android is the release target.

### Architecture

```text
qml/                       Presentation only - binds to Controllers,
                            forwards user actions.
include/Controllers/        QObject classes exposed to QML (Q_PROPERTY /
src/Controllers/             Q_INVOKABLE): HomeController, DiscoverController,
                            MyListController. Own orchestration - call
                            Infrastructure, hold Domain state, expose
                            QML-safe state.
include/Domain/             Plain C++ types and pure logic - no Qt Network,
src/Domain/                  no QML, no file I/O. FilterCriteria, Candidate,
                            CandidatePool, SuggestionSession, WeightedPicker.
include/Infrastructure/     TmdbClient (requests + JSON parsing), JsonStore
src/Infrastructure/          (the one persisted app-data file), PosterUrlResolver,
                            RuntimeConfig (credential + base URL indirection).
include/ViewModels/Models/  QAbstractListModel subclasses for anything shown
src/ViewModels/Models/       in a ListView/GridView.
```

Raw TMDB JSON never reaches QML or a Controller - `TmdbClient` always
returns typed values. The TMDB credential is read in one place and never
appears in QML, logs, or persisted data.

## Getting started

### Prerequisites

- Qt 6.10.3 (Quick, Qml, Network modules) with a MinGW or MSVC kit for
  desktop, and the Android kit if you're building for Android.
- CMake 3.16+.
- A [TMDB API key](https://www.themoviedb.org/settings/api) (the v3 key or
  the v4 read access token both work).

### Configure your TMDB credential

```bash
cp include/Config/ApiKeys.h.example include/Config/ApiKeys.h
```

Then edit `include/Config/ApiKeys.h` and replace the placeholder with your
own TMDB credential. This file is gitignored - it never gets committed.

### Build (desktop)

Open `CMakeLists.txt` in Qt Creator and configure it against a Qt 6.10.3
desktop kit, or from the command line:

```bash
cmake -B build -S .
cmake --build build
```

### Build (Android)

Configure against a Qt 6.10.3 Android kit in Qt Creator (targets
`compileSdk`/`targetSdk` 36, `minSdk` 28, NDK 27.3.13750724) and deploy to
a device or emulator the normal Qt Creator way.

## License

GPLv3 - see [LICENSE](LICENSE).

TMDB data is used under TMDB's non-commercial API terms. This product uses
the TMDB API but is not endorsed or certified by TMDB.
