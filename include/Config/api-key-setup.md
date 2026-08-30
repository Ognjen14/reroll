# `Setting Up Your TMDB Key`

The app needs a TMDB credential to fetch movie data. That credential lives in `include/Config/ApiKeys.h`, which is **not** in Git — so you'll need to create it yourself the first time you clone the repo on a new machine. You only have to do this once.

`ApiKeys.h` stores the credential XOR-obfuscated rather than as a plain string, so it doesn't sit as a grep-able string in the compiled binary. You don't need to construct that obfuscation by hand - a script does it for you.

## Generate `ApiKeys.h`

From the project root:

```bash
python scripts/generate_api_key.py
```

It'll prompt for your TMDB credential (input isn't echoed to the terminal) and write `include/Config/ApiKeys.h` for you. Either of these works:

- **API Read Access Token** — the long token from your TMDB developer portal. The app sends it in an `Authorization: Bearer` header.
- **v3 API key** — the shorter 32-character one. The app still supports it and sends it as an `api_key` query parameter.

Run the same script again whenever you rotate your key - it always overwrites `ApiKeys.h` from scratch, so there's nothing to hand-edit.

## Good to know

- `ApiKeys.h` is gitignored. Don't commit it, and don't paste your key anywhere else in the project.
- `ApiKeys.h.example` is only a template showing the file's shape (its bytes decode to placeholder text, not a real key). It's never compiled — the app includes `ApiKeys.h`.
- The obfuscation only defeats a trivial `strings`/apktool dump of the compiled binary - it is not encryption. Anyone attaching a debugger to a running build can still recover the key from memory. Don't treat it as a substitute for TMDB-side key rotation if a key ever does leak.

