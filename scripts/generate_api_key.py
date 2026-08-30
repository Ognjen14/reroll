import getpass
import os
import random

REPO_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TARGET_PATH = os.path.join(REPO_ROOT, "include", "Config", "ApiKeys.h")


def to_c_array(data):
    return ", ".join(f"0x{b:02x}" for b in data)


def main():
    key = getpass.getpass("Paste your TMDB API Read Access Token or v3 API key: ").strip()
    if not key:
        raise SystemExit("No key entered, aborting.")

    key_bytes = key.encode("utf-8")

    mask_len = 16
    mask = bytes(random.randint(1, 255) for _ in range(mask_len))
    obfuscated = bytes(b ^ mask[i % mask_len] for i, b in enumerate(key_bytes))

    decoded = bytes(obfuscated[i] ^ mask[i % mask_len] for i in range(len(obfuscated)))
    assert decoded == key_bytes, "round-trip mismatch, aborting"

    content = f'''#pragma once

#include <cstddef>

namespace Reroll::Config
{{
inline constexpr char TmdbBaseUrl[] = "https://api.themoviedb.org/3";

inline constexpr unsigned char TmdbApiKeyMask[] = {{ {to_c_array(mask)} }};
inline constexpr unsigned char TmdbApiKeyObfuscated[] = {{ {to_c_array(obfuscated)} }};
inline constexpr std::size_t TmdbApiKeyLength = sizeof(TmdbApiKeyObfuscated);
}}
'''

    with open(TARGET_PATH, "w", encoding="utf-8") as f:
        f.write(content)

    print(f"Wrote {TARGET_PATH} ({len(key_bytes)}-byte key, obfuscated).")


if __name__ == "__main__":
    main()
