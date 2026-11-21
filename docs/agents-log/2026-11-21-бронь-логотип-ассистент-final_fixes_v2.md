# 21–22.11.2026 — три бага: бронь, логотип, ассистент (ветка `final_fixes_v2`)

Журнал по §5.7 AGENTS.md. Ветка **`final_fixes_v2`** (git: `On branch final_fixes_v2`,
ahead of `origin/final_fixes_v2` **by 3 commits**, рабочее дерево чистое — `git status`,
проверено 22.11). Коммиты фиксов: `5cd5b8a` (22.11 00:19 +0300), `9f8dd5f` (00:32),
`dc56273` (00:46). Push этой сессией не выполнялся.

## 1. Три бага — диагноз «be», подтверждение true

По всем трём пунктам сводки проверки: **диагноз «be» (баг есть), подтверждение true**
(данные из задачи сессии). Состав (по коммитам и диффам):

1. **Бронь (rooms, BE):** «Забронировано → пустые „Мои брони"» — `create` принимал
   уже закончившиеся интервалы (201 + тост «посмотреть в моих бронях»), а
   upcoming-фильтр `myBookings` (`ends_at >= now`) такие брони скрывал; сетка
   прошедшего дня при этом отдавала прошедшие слоты как «свободные», которые
   создать уже нельзя (рассинхрон веток одного сервиса).
2. **Логотип (topbar, FE):** в SVG-вордмарке «пара» оставался векторный декор —
   конструктивные точки/линии (`para-wordmark__guides`/`__nodes`) и hover-анимации.
3. **Ассистент (dashboard, FE):** на мобильном (≤599px) плавающая кнопка
   ассистента коллапсировала в кружок 56×56, подпись пряталась (sr-only).

## 2. Сделанные правки (файлы из коммитов)

### `5cd5b8a` — rooms: прошедшие интервалы и слоты (баг 1)

- `src/services/RoomsService.cpp` / `.h` — `create`: новый параметр `nowIso`
  (контроллер передаёт `SessionService::utcNow()`), `startsAt <= nowIso` →
  `InvalidBody` (400) **до** INSERT — без записи в БД; `slots`: слот, закончившийся
  к `nowIso` (`slotEnd <= nowIso`), в состоянии `"free"` помечается `"booked"`
  (в трёх состояниях 02 §5.2 не-«free» не тапается); «ваша бронь» `"mine"`
  остаётся и у прошедшего — отмена из сетки работает. Комментарии в шапках
  `slots`/`create` фиксируют контракт.
- `src/controllers/RoomsCtrl.cpp` — оба вызова сервиса получают
  `SessionService::utcNow()`; текст 400 дополнен «интервал в будущем».
- `tests/unit/rooms_service_test.cpp` (+92 строки) — хелперы `slotsOf`/`createOf`
  приняли `nowIso` (дефолт `2000-01-01T00:00:00Z` — прежние кейсы детерминированы);
  +2 TEST_CASE: «slots — прошедшие слоты дня не „свободно"» (прошедшие → booked,
  mine сохраняется, будущий день при утреннем now — все 14 free) и
  «create — прошедший интервал → InvalidBody» (живая репродукция бага:
  09:00–10:00 при now 20:53, граница starts == now, начало в прошлом).
- `tests/security_tests.cpp` — слот предусловия шага 8 заменён с прошедшей
  13.11 на `2099-06-01` (тест не зависит от wall-clock).
- Тем же коммитом добавлен снимок внешней страницы «Карта Университета»
  (`docs/source/Карта Университета_files/`, 22 файла, ~22 тыс. строк —
  saved-page-ассеты) — справочный материал, к коду фикса не относится.

### `9f8dd5f` — topbar: чистый текстовый знак «пара» (баг 2)

- `views/partials/topbar.csp`, `views/login.csp`, `views/not_found.csp` — из
  SVG-вордмарка удалены группы `__guides` (пунктирные линии) и `__nodes`
  (точки/квадраты); остались только буквенные пути.
- `design-drafts/v3/assets/prototype.css` (эталон) и `static/css/app.css` —
  идентичными правками убраны стили узлов/направляющих и hover-анимации
  (`para-wordmark__nodes/__guides`, `para-node-breathe` на бренде).

### `dc56273` — dashboard: fab-пилюля на мобильном (баг 3)

- `tools/build_app_css.py` (генератор, предпочтительный путь) и
  `static/css/app.css` — идентичной правкой удалён мобильный коллапс в кружок
  (`@media(max-width:599px){width:56px;height:56px;…}` со sr-only-подписью):
  на всех ширинах — та же пилюля с иконкой и подписью «Ассистент»
  (тап-зона 143.8×52 ≥ 44px).

## 3. Результаты гейтов и e2e

Сводка проверки (из задачи сессии, дословно):

```json
{"visibleWithUpcomingFilter":true,"cancelled":true,"evidence":"1) ВХОД: `curl -isS -c c.txt -X POST localhost:8080/v1/auth/login -H 'Content-Type: application/json' -d '{\"login\":\"anya.demo\",\"password\":\"para-demo-2026\"}'` → `HTTP/1.1 200 OK` + `set-cookie: para_session=318dd0125fdb70cb808e878c260b6cfb; ... HttpOnly; Secure; SameSite=Lax` + `Set-Cookie: csrf=dd605cbd...; ...` + тело `{\"login\":\"anya.demo\",\"role\":\"student\"}`.\n2) СВОБОДНЫЙ СЛОТ НА ЗАВТРА (`date -v+1d +%Y-%m-%d` → 2026-11-23): `curl -b c.txt \"localhost:8080/v1/rooms/1/slots?date=2026-11-23\"` → 200, все 14 слотов `state:\"free\"`, выбран 10:00–11:00 UTC.\n3) СОЗДАНИЕ: `curl -isS -b c.txt -X POST localhost:8080/v1/rooms/1/bookings -H 'Content-Type: application/json' -H \"X-CSRF-Token: $CSRF\" -d '{\"starts_at\":\"2026-11-23T10:00:00Z\",\"ends_at\":\"2026-11-23T11:00:00Z\"}'` (CSRF из cookie-jar: `awk '$6==\"csrf\" {print $7}' c.txt`) → `HTTP/1.1 201 Created` + `{\"ends_at\":\"2026-11-23T11:00:00Z\",\"id\":7,\"room_id\":1,\"starts_at\":\"2026-11-23T10:00:00Z\",\"status\":\"booked\"}` — 409/400 не было, слот был свободен.\n4) ЭКРАН «МОИ БРОНИ»: `curl -isS -b c.txt \"localhost:8080/v1/room-bookings?upcoming=1\"` → `HTTP/1.1 200 OK` + `{\"bookings\":[{\"building\":\"Корпус Б\",\"ends_at\":\"2026-11-23T11:00:00Z\",\"id\":7,\"room\":\"1207\",\"room_id\":1,\"starts_at\":\"2026-11-23T10:00:00Z\",\"status\":\"booked\"}]}` — свежая бронь id=7 видна (комната 1207, Корпус Б, 10:00–11:00 UTC 23.11). БЕЗ ФИЛЬТРА `curl -b c.txt localhost:8080/v1/room-bookings` → 200, `bookings` из двух: прошедшая `{\"id\":2,\"room\":\"3105\",\"starts_at\":\"2026-11-06T14:00:00Z\",...}` и новая `{\"id\":7,...}` — фильтр upcoming=1 корректно отсекает прошедшую id=2, без фильтра видны обе.\n5) УДАЛЕНИЕ: `curl -isS -b c.txt -X DELETE localhost:8080/v1/room-bookings/7 -H \"X-CSRF-Token: $CSRF\" -w '\\nHTTP_CODE=%{http_code}'` → `HTTP/1.1 204 No Content` / `HTTP_CODE=204`.\n6) КОНТРОЛЬ ПОСЛЕ УДАЛЕНИЯ: `curl -b c.txt \"localhost:8080/v1/room-bookings?upcoming=1\"` → `{\"bookings\":[]}`; слот снова свободен: `GET /v1/rooms/1/slots?date=2026-11-23` → 10:00 slot state: ['free'].\nНичего в репозитории не менялось (только curl-запросы, cookie-jar в /tmp/para_e2e_r1)."}
```

Кратко: гейты `visibleWithUpcomingFilter` = **true**, `cancelled` = **true**;
e2e-цепочка на живом контуре (localhost:8080) — вход 200 (+cookie para_session
HttpOnly/Secure/SameSite=Lax + csrf) → слот завтра 2026-11-23 свободен (14/14) →
создание 201 (id=7, 1207/Корпус Б) → `?upcoming=1` показывает id=7 (без фильтра —
прошедшая id=2 + новая id=7) → DELETE 204 → `upcoming=1` пуст, слот снова «free».
Cookie-jar — `/tmp/para_e2e_r1` (есть на диске, 22.11 00:18).

**Дополнительные живые контроли летописца (22.11, только чтение):**

- `md5 -q static/css/app.css` и `curl -s localhost:8080/static/css/app.css | md5` →
  оба `127788443c42098c4c5a2c99b683f583` — контур отдаёт свежую сборку;
- `curl -s localhost:8080/ | grep -c 'para-wordmark__nodes\|para-wordmark__guides'`
  → **0** (декор логотипа не отдаётся — фикс 2 вживую);
- `curl -s localhost:8080/static/css/app.css | grep -c 'фикс 22.11 (баг 3)'` → 1
  (правка fab в отданном CSS);
- `curl -s localhost:8080/healthz` → `ok`;
- сетка прошедшего дня (фикс 1): вход anya.demo → `GET /v1/rooms/1/slots?date=2026-11-06`
  → `Counter({'booked': 14})` — ни одного «free».

## 4. Отступления от правил

Не зафиксировано:

- заметка e2e-исполнителя: «Ничего в репозитории не менялось (только curl-запросы…)»;
- `static/css/app.css` правился через генератор/эталон: проверено пересборкой —
  `git show 7feaccd:static/css/app.css > /tmp/para_old_app.css && python3 -I
  tools/build_app_css.py /tmp/para_old_app.css | diff - static/css/app.css` →
  **0 строк расхождений** (файл байт-в-байт равен выводу генератора);
- `src/services/RoomsService.{h,cpp}` правлен в рамках явного разрешения задачи
  (баг 1 — логика сервиса), `sql/` и токены 03 не тронуты (`git show --stat`
  всех трёх коммитов).

Честные оговорки: контурный ctest-гейт (`docker compose --profile tests run --rm
--build tests`) этой сессией не запускался — последних записанных счётчиков после
`5cd5b8a` нет (актуальные на `main` — para-tests 189/8333, security_tests 119/119
из `e2e5204`, до трёх фиксов); AGENTS.md §6 этой веткой не обновлялся (багфикс-раунд,
не этап 05 §3). Живые проверки выше — только read-only curl/GET, состояние БД контура
не менялось (созданная e2e бронь id=7 удалена самим e2e, слот возвращён в «free»).
