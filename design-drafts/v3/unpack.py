#!/usr/bin/env python3
"""Распаковщик эталона дизайна v3 («Пара — копия.html» → pages/ + assets/).

Одноразовый инструмент: читает JSON из <script id="para-bundle"> переносимого
прототипа и раскладывает его в просматриваемые файлы:

  pages/*.html               41 страница; общий CSS/JS подключены ссылками
  assets/prototype.css       общий CSS прототипа, шрифты вынесены в fonts/
  assets/prototype.js        демо-интерактив прототипа (localStorage — НЕ прод)
  assets/fonts/*.woff2       Inter (латиница+, кириллица), вариативный 100–900
  assets/img/*.png           растровые картинки страниц (эмблема входа)
  assets/licenses/*.txt      лицензии из бандла (Inter OFL, Three.js MIT, OSM)

Запуск из корня репозитория:
  python3 -I design-drafts/v3/unpack.py
Повторный запуск идемпотентен (перезаписывает pages/ и assets/).
"""
import base64
import hashlib
import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parent
BUNDLE = ROOT / "prototype-bundle.html"
PAGES = ROOT / "pages"
ASSETS = ROOT / "assets"

DATA_URI = re.compile(r"data:([a-z0-9.+/-]+);base64,([A-Za-z0-9+/=]+)")
EXT = {"font/woff2": "woff2", "image/png": "png", "image/jpeg": "jpg",
       "image/webp": "webp", "image/svg+xml": "svg"}


def load_bundle() -> dict:
    src = BUNDLE.read_text(encoding="utf-8")
    m = re.search(r'<script id="para-bundle" type="application/json">', src)
    if not m:
        sys.exit("нет <script id=para-bundle> в " + str(BUNDLE))
    end = src.index("</script>", m.end())
    return json.loads(src[m.end():end])


def write(path: pathlib.Path, data) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    if isinstance(data, bytes):
        path.write_bytes(data)
    else:
        path.write_text(data, encoding="utf-8")


def extract_fonts(css: str) -> str:
    """@font-face с data:-URI → assets/fonts/inter-{latin,cyrillic}.woff2."""
    def face(m: re.Match) -> str:
        body = m.group(1)
        name = "inter-cyrillic" if "U+0400" in body else "inter-latin"
        dm = DATA_URI.search(body)
        write(ASSETS / "fonts" / f"{name}.woff2", base64.b64decode(dm.group(2)))
        return "@font-face{" + DATA_URI.sub(f"fonts/{name}.woff2", body, count=1) + "}"
    return re.sub(r"@font-face\{(.*?)\}", face, css, flags=re.S)


def extract_images(page: str, stem: str) -> str:
    """Растровые data:-URI страницы → assets/img/<страница>-<hash>.<ext>."""
    def img(m: re.Match) -> str:
        mime, b64 = m.group(1), m.group(2)
        if mime not in EXT or mime == "font/woff2":
            return m.group(0)
        raw = base64.b64decode(b64)
        name = f"{stem}-{hashlib.sha1(raw).hexdigest()[:8]}.{EXT[mime]}"
        write(ASSETS / "img" / name, raw)
        return "../assets/img/" + name
    return DATA_URI.sub(img, page)


def main() -> None:
    b = load_bundle()
    write(ASSETS / "prototype.css", extract_fonts(b["css"]))
    write(ASSETS / "prototype.js", b["app"])
    for lic, text in b.get("licenses", {}).items():
        write(ASSETS / "licenses" / f"{lic.lower().replace('.', '')}.txt", text)
    link = ('<link rel="stylesheet" href="../assets/prototype.css">'
            '<link rel="icon" href="data:,">')
    script = '<script src="../assets/prototype.js" defer></script>'
    for name, html in sorted(b["pages"].items()):
        html = extract_images(html, pathlib.Path(name).stem)
        html = html.replace("</head>", link + "</head>", 1)
        html = html.replace("</body>", script + "</body>", 1)
        write(PAGES / name, html)
    print(f"pages: {len(b['pages'])} → {PAGES.relative_to(ROOT.parent.parent)}")
    for p in sorted(ASSETS.rglob("*")):
        if p.is_file():
            print(f"  {p.relative_to(ROOT)}  {p.stat().st_size} B")


if __name__ == "__main__":
    main()
