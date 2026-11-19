# design-drafts/v3 — эталон дизайна «Пара», версия 3 (navy)

Эталон нового внешнего вида приложения (fe_rebuild, 18.11.2026). Источник —
переносимый прототип «Пара — копия.html» (один HTML 3,3 МБ, все страницы,
стили и шрифты внутри). Прототип задаёт **только визуал, структуру и
состояния**; логика, данные и эндпоинты в проде — продовые (никаких
localStorage-демо-данных прототипа в `static/`).

## Состав

| Путь | Что это |
|---|---|
| `prototype-bundle.html` | оригинал, копия файла без изменений (открывается в браузере сам по себе) |
| `unpack.py` | одноразовый распаковщик: JSON из `<script id="para-bundle">` → `pages/` + `assets/` |
| `pages/*.html` | 41 страница прототипа, общий CSS/JS подключены ссылками на `../assets/` |
| `assets/prototype.css` | общий CSS прототипа (шрифты вынесены в `fonts/`) — источник токенов и компонентов |
| `assets/prototype.js` | демо-интерактив прототипа (localStorage, Three.js карты) — **только для просмотра, в прод не переносится** |
| `assets/fonts/inter-{latin,cyrillic}.woff2` | Inter variable 100–900; побайтно совпадают с `static/fonts/` (сверено `cmp`) |
| `assets/img/login-*.png` | эмблема вымышленного «Университета Меридиан» с экрана входа |
| `assets/licenses/` | лицензии из бандла: Inter (OFL 1.1), Three.js (MIT), OpenStreetMap (ODbL, атрибуция) |

Карта соответствия «прототип ↔ прод» и расхождения с решениями — журнал
`docs/agents-log/2026-11-18-дизайн-v3-каркас.md`, разделы «Карта» и «Расхождения».

Предыдущая версия (v2, зелёная палитра «спокойно видеть главное») — файлы
`design-drafts/*.html` + `design-drafts/assets/`; CSS v3 — надмножество v2:
из 336 правил v2 в v3 изменились только 20 токенов (палитра), +777 новых правил
(site-header с primary-nav/campus-nav, вордмарк, 404, discipline-english и др.).

## Как смотреть

```bash
python3 -I design-drafts/v3/unpack.py      # пересобрать pages/ и assets/ (идемпотентно)
open design-drafts/v3/pages/dashboard.html # любая страница; ссылки между страницами — относительные
```

Либо открыть `prototype-bundle.html` целиком (навигация через `#/страница.html`).
Состояния экранов — страницы `*-states.html` и параметр `?state=loading|empty|error|offline`.
Витрины `index.html`, `prototype.html`, `developer.html`, `components.html` —
служебные страницы прототипа, в прод не переносятся (`components.html` —
источник спецификации компонентов для 03 §2.8).
