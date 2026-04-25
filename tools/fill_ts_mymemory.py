#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Remplit les <translation> d'un fichier .ts (fr -> cible) via l'API gratuite MyMemory (usage modéré)."""
import argparse
import re
import sys
import time
import urllib.parse
import urllib.request
import xml.etree.ElementTree as ET

API = "https://api.mymemory.translated.net/get"


def translate_chunk(text: str, langpair: str) -> str:
    if not text.strip():
        return text
    q = urllib.parse.quote(text[:450])
    url = f"{API}?q={q}&langpair={langpair}"
    req = urllib.request.Request(url, headers={"User-Agent": "LeatherERP-lupdate-helper/1"})
    with urllib.request.urlopen(req, timeout=30) as resp:
        import json

        data = json.loads(resp.read().decode("utf-8"))
    out = data.get("responseData", {}).get("translatedText", "")
    if not out or out.startswith("MYMEMORY WARNING"):
        return text
    return out


def translate_text(text: str, langpair: str) -> str:
    parts = []
    max_len = 400
    for i in range(0, len(text), max_len):
        chunk = text[i : i + max_len]
        parts.append(translate_chunk(chunk, langpair))
        time.sleep(0.35)
    return "".join(parts)


def process_ts(path: str, langpair: str) -> None:
    tree = ET.parse(path)
    root = tree.getroot()
    ns = ""
    if root.tag.startswith("{"):
        ns = root.tag.split("}")[0] + "}"
    messages = root.findall(f".//{ns}message")
    n = 0
    for msg in messages:
        source = msg.find(f"{ns}source")
        trans = msg.find(f"{ns}translation")
        if source is None or trans is None:
            continue
        src_text = source.text or ""
        if not src_text.strip():
            continue
        if trans.text and trans.text.strip() and trans.get("type") != "unfinished":
            continue
        try:
            en = translate_text(src_text, langpair)
        except Exception as e:
            print(f"skip (error): {src_text[:40]!r} {e}", file=sys.stderr)
            continue
        trans.text = en
        if "type" in trans.attrib:
            del trans.attrib["type"]
        n += 1
        try:
            print(f"[{n}] ok: {src_text[:40]!r}")
        except UnicodeEncodeError:
            print(f"[{n}] ok")
    tree.write(path, encoding="utf-8", xml_declaration=True)
    print(f"Updated {n} messages in {path}")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("ts_file")
    ap.add_argument("--pair", default="fr|en", help="MyMemory langpair, e.g. fr|en or fr|ar")
    args = ap.parse_args()
    process_ts(args.ts_file, args.pair)
    return 0


if __name__ == "__main__":
    sys.exit(main())
