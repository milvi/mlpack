#!/usr/bin/env python3
"""Vérifie que src contient exactement les dépendances C++ de k-means et de ses tests."""
import json
import os
from pathlib import Path
import re
import shlex
import subprocess

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ['native_kmeans_api.cpp', 'tests/native_memory.cpp']
HEADERS = {p.resolve() for p in (ROOT / 'src').rglob('*') if p.is_file()}
needed = set()
for mode in ['-DNDEBUG', '-DDEBUG']:
    command = shlex.split(os.environ.get('CXX', 'g++')) + [
        '-std=c++17', '-fopenmp', '-DMLPACK_GO_MONOLITHIC', mode, '-I.', '-Isrc',
        *shlex.split(os.environ.get('CGO_CXXFLAGS', '')), '-MM', *SOURCES]
    result = subprocess.run(command, cwd=ROOT, check=True, capture_output=True, text=True, timeout=120)
    for line in result.stdout.replace('\\\n', ' ').splitlines():
        for token in shlex.split(line.split(':', 1)[1]):
            path = (ROOT / token).resolve()
            if path.is_relative_to(ROOT / 'src'):
                needed.add(path)
if HEADERS != needed:
    raise SystemExit('Fichiers C++ manquants ou superflus : ' + str(sorted(str(p.relative_to(ROOT)) for p in HEADERS ^ needed)))
for path in HEADERS | {ROOT / p for p in SOURCES}:
    text = path.read_text()
    for delimiter, name in re.findall(r'^\s*#include\s+([<"])([^>"]+)[>"]', text, re.MULTILINE):
        if name.startswith('mlpack/'):
            target = ROOT / 'src' / name
        elif delimiter == '"':
            target = path.parent / name
            if not target.is_file():
                target = ROOT / name
        else:
            continue
        if not target.is_file():
            raise SystemExit(f'Inclusion absente : {path.relative_to(ROOT)} → {name}')
    if re.search(r'#include\s*[<"](?:cereal|ensmallen)', text):
        raise SystemExit(f'Dépendance retirée réintroduite : {path.relative_to(ROOT)}')
units = sorted(p.name for p in ROOT.glob('*.cpp'))
if units != ['native_kmeans_api.cpp']:
    raise SystemExit(f'Unités de compilation inattendues : {units}')
print(json.dumps(dict(headers=len(HEADERS), source_bytes=sum(p.stat().st_size for p in HEADERS),
                     native_compilation_units=len(units), include_closure='exacte, release et debug'), ensure_ascii=False))
