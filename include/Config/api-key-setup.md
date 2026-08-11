# Setting Up Your TMDB Key

The app needs a TMDB credential to fetch movie data. That credential lives in `include/Config/ApiKeys.h`, which is **not** in Git — so you'll need to create it yourself the first time you clone the repo on a new machine. You only have to do this once.

## 1. Make your own copy of the template

From the project root, in PowerShell:

```powershell
Copy-Item -LiteralPath "include\Config\ApiKeys.h.example" -Destination "include\Config\ApiKeys.h"
```

## 2. Paste in your credential

Open `include/Config/ApiKeys.h` and swap the placeholder:

```
"REPLACE_WITH_YOUR_TMDB_CREDENTIAL"
```

for your actual key. Either of these works:

- **API Read Access Token** — the long token from your TMDB developer portal. The app sends it in an `Authorization: Bearer` header.
- **v3 API key** — the shorter 32-character one. The app still supports it and sends it as an `api_key` query parameter.

Just replace the text inside the quotes. Leave the rest of the C++ around it alone.

## Good to know

- `ApiKeys.h` is gitignored. Don't commit it, and don't paste your key anywhere else in the project.
- `ApiKeys.h.example` is only a template. It's never compiled — the app includes `ApiKeys.h`.
