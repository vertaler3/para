# AGENTS.md — «Пара» (цифровой деканат, концепт хакатона)

> **Живой файл продолжения работы.** Любая сессия — человека или ИИ-агента — начинается с чтения этого файла: он отвечает на вопросы «что это, что уже готово, как запустить, что дальше». Обновляется в конце каждого этапа (протокол ведения — раздел 7).

## 1. Что это за проект

**«Пара»** — работающий прототип личного кабинета студента («деканат в кармане») для хакатона. Это **концепт для конкурса**: все данные вымышленные, это не официальный сервис вуза, логотип вуза не используется. Задание — «Хакатон от мгимо - вайбкодинг.md» в корне рабочей области.

Ключевая идея: главный экран «Сейчас важно» отвечает на вопрос «что мне делать сегодня»; ядро — вход/профиль, расписание с переносами («было → стало» + push), успеваемость, заявка на справку конечным автоматом с таймлайном, уведомления, ведущие к объекту. Mobile-first от 360 px, светлая и тёмная темы.

## 2. Где лежат решения (читать в этом порядке)

| Файл | Что это |
|---|---|
| `решения/01-продуктовое-решение.md` | продукт: персоны, сценарии С1–С8, идеи 5.1–5.16, DoD, тестовые аккаунты |
| `решения/02-техническое-решение.md` | стек, модель данных + DDL (миграции 0001–0006), API, безопасность, деплой, тесты |
| `решения/03-дизайн-решение.md` | дизайн-система на токенах, вайрфреймы экранов, состояния, доступность |
| `решения/05-план-реализации-и-работа-с-ИИ.md` | этапы Э1–Э11, ролевая модель агентов, шаблоны промптов, чек-лист сдачи |
| `гайды/гайд-бэкенд.md` · `гайды/гайд-фронтенд.md` | пошаговые дорожки BE/FE: что делать последовательно, чем проверять, как интегрироваться (контракт разделения — 05 §4.5) |
| `решения/04-обзор-зарубежных-open-source-проектов.md` | справочный обзор аналогов — **во входные документы кодирования не входит** (решение команды; просматривается отдельно) |
| `docs/decisions.md` (с Э1) | журнал решений: контекст → варианты → выбор → причина |
| `docs/agents-log/` (с Э1) | журнал ИИ-сессий: промпты, ошибки ИИ, правки (сырьё для PDF-отчёта жюри) |

## 3. Стек (зафиксирован в 02 §2)

- **Бэкенд:** C++20, Drogon v1.9.14 (MIT), SSR на CSP-шаблонах. Контроллеры тонкие (10–30 строк), логика в `src/services`; владение каждым объектом проверяет один `OwnershipGuard` на каждом path-id (чужой id → 403).
- **БД:** SQLite в WAL-режиме; миграции `/sql/migrations/0001…0006` применяются при старте приложения; демо-данные — `/sql/fixtures.sql` (только вымышленные). PostgreSQL — опция роста (тот же `DbClient`).
- **Фронтенд:** CSP-шаблоны + чистый CSS на токенах из 03 §2 + htmx + Alpine.js — вендоренно в `static/vendor/`, **без npm-сборки и внешних CDN**.
- **Инфраструктура:** Docker multi-stage + docker-compose, Nginx + systemd, бэкап SQLite `.backup` по cron; хостинг — российский VPS, доступный из РФ без VPN.

## 4. Команды

> Фактически проверенные на Docker-контуре Э1 (последний полный прогон — гейт закрытия Э1, 05.11.2026). `<DEMO_PASSWORD>` — плейсхолдер единого демо-пароля: значение в репо не хранится (§5.2), публикуется в комментарии к работе. Локальная сборка без Docker (`cmake`/`ctest` на хосте) в Э1 не проверялась.

- **Сборка и запуск:** `cd para && docker compose up -d --build` → exit 0 (демо-контур сеет фикстуры при старте — ENTRYPOINT с `--seed`, Р-8);
- **Живость:** `curl localhost:8080/healthz` → HTTP 200, тело «ok»;
- **Тесты:** `docker compose --profile tests run --rm tests` → ctest зелёный (para-tests + security_tests);
- **Смок входа** (JSON-тело — как в гайде BE §2 и в реализации AuthCtrl; тело ошибок — единый формат 02 §5.1 `{error:{code,message}}`):
  ```bash
  curl -i -c c.txt -X POST localhost:8080/v1/auth/login \
    -H 'Content-Type: application/json' \
    -d '{"login":"anya.demo","password":"<DEMO_PASSWORD>"}'  # 200 + Set-Cookie: para_session=…; HttpOnly; Secure; SameSite=Lax
  curl -i -b c.txt localhost:8080/v1/me                      # 200
  curl -i -b c.txt -X POST localhost:8080/v1/auth/logout     # 200
  curl -i -b c.txt localhost:8080/v1/me                      # 401 {"error":{"code":"unauthorized",…}}
  ```
  неверный пароль → 401 `invalid_credentials` без деталей; 4-я попытка подряд → 429 + `Retry-After: 15`;
- **Идемпотентность сидирования:** `docker compose restart para` → `users` до/после равны (маркер `fixtures` в schema_migrations, Р-5/Р-8); пересеивание фикстур с нуля (в т.ч. после ротации пароля) — `docker compose down -v`, затем `up` снова.
- **BE-часть Э2** (фактически проверено 05.11.2026 в одноразовом контейнере `drogonframework/drogon:v1.9.14` — сборка в /tmp, не `docker compose`; compose-прогоны гейта поверх того же кода — ниже):
  - **настройки (JSON, оба маршрута под LoginFilter; тело ошибок — 02 §5.1):**
    ```bash
    curl -s -c c.txt -X POST localhost:8080/v1/auth/login \
      -H 'Content-Type: application/json' \
      -d '{"login":"anya.demo","password":"<DEMO_PASSWORD>"}'
    curl -s -b c.txt localhost:8080/v1/me                       # 200, profile: fio/faculty/program/course/group/student_card_no
    curl -s -b c.txt localhost:8080/v1/notifications/settings   # 200 — дефолты DDL (тумблеры true, quiet 23:00–07:00, theme system, onboarding [])
    curl -s -b c.txt -X PUT localhost:8080/v1/notifications/settings \
      -H 'Content-Type: application/json' \
      -d '{"theme":"dark","notify_deadlines":false,"user_id":999}'  # 200; user_id из тела игнорируется
    curl -s -b c.txt localhost:8080/v1/notifications/settings   # 200 — сохранённое (theme dark, deadlines false)
    curl -s localhost:8080/v1/notifications/settings            # 401 {"error":{"code":"unauthorized",…}}
    ```
    невалидная тема (`"theme":"pink"`) или не-JSON в `quiet_hours_json`/`onboarding_json` → 400 `bad_request`;
  - **sqlite-проверка изоляции** (после PUT: строка изменена только у текущего пользователя):
    ```bash
    docker compose exec para sqlite3 /opt/para/data/para.db \
      "SELECT user_id, notify_schedule, notify_deadlines, notify_applications, quiet_hours_json, theme, onboarding_json FROM settings;"
    # строка user_id=1 (anya.demo) с новыми значениями; строк других пользователей нет/не изменены
    ```
    (в проверках сессии — тот же SELECT через sqlite3 C API в контейнере; вход в curl-цепочке замещался прямой вставкой валидной сессии в `sessions` — демо-пароль вне репо, §5.2);
  - **тесты:** `ctest` зелёный в одноразовом контейнере (para-tests 16/16 кейсов, 219 утверждений: OwnershipGuard — своё/чужое/несуществующее + fail-closed при ошибке БД; SettingsService — дефолты/UPSERT/изоляция/валидация; security_tests); контурный прогон — `docker compose --profile tests run --rm tests` (команда выше).
- **BE-часть Э3** (фактически проверено 05.11.2026 в одноразовом контейнере `drogonframework/drogon:v1.9.14` — сборка в /tmp, не `docker compose`; compose-прогоны гейта поверх того же кода — ниже):
  - **расписание (JSON, все роуты под LoginFilter; тело ошибок — 02 §5.1):**
    ```bash
    curl -s -c c.txt -X POST localhost:8080/v1/auth/login \
      -H 'Content-Type: application/json' \
      -d '{"login":"anya.demo","password":"<DEMO_PASSWORD>"}'
    curl -s -b c.txt localhost:8080/v1/schedule/today              # 200 {"date":"2026-11-05","events":[2 события: id 7 cancelled (prev_room 2109), id 8 normal]}
    curl -s -b c.txt "localhost:8080/v1/schedule/week?offset=0"    # 200 from=2026-11-02 to=2026-11-08 — 10 событий ids 1–10 (включая moved 04.11)
    curl -s -b c.txt "localhost:8080/v1/schedule/week?offset=-1"   # 200 from=2026-10-26 to=2026-11-01 — 0 событий
    curl -s -b c.txt "localhost:8080/v1/schedule/changes?days=7"   # 200 3 изменения: (5, moved, 3024→4210), (6, moved, 3105→4210), (7, cancelled)
    curl -s -b c.txt "localhost:8080/v1/schedule/changes?days=1"   # 200 только id 7 (окно — последние days суток, включая сегодня)
    curl -s -b c.txt "localhost:8080/v1/schedule/week?offset=abc"  # 400 {"error":{"code":"bad_request",…}}; days вне 1..31 — так же
    ```
    (в проверках сессии вход замещался прямой вставкой валидной сессии в `sessions` — демо-пароль вне репо, §5.2; группа берётся из профиля сессии — staff dean.demo → 200 с пустым списком);
  - **ICS-цикл (приватная подписка 5.4; GET /ics/{token} — публичный: токен и есть аутентификация, 02 §6.1):**
    ```bash
    curl -s -b c.txt -X POST localhost:8080/v1/ics/token           # 201 {"token":"…"} — поле token, 64 hex-символа (256 бит, Р-11); повторный POST — ротация: прежний перестаёт работать
    curl -s -o a.ics localhost:8080/ics/<ТОКЕН>                    # 200 Content-Type: text/calendar — 10 VEVENT (все статусы, prev-аудитория у moved), строки CRLF
    curl -s -o /dev/null -w '%{http_code}' localhost:8080/ics/нет-такого # 404 {"error":{"code":"not_found",…}}
    curl -s -b c.txt -X DELETE localhost:8080/v1/ics/token         # 204 (повторный DELETE — тоже 204)
    curl -s -o /dev/null -w '%{http_code}' localhost:8080/ics/<ТОКЕН>    # 404 после отзыва
    curl -s -X POST localhost:8080/v1/ics/token                    # 401 (POST/DELETE — только по сессии)
    ```
  - **тесты:** `ctest` зелёный в одноразовом контейнере (para-tests **34/34 кейса, 580 утверждений** — факт на закрытие Э3, после раунда 2 гейта: +4 кейса контракта токена Р-11 в раунде 2; на момент шагов 1–2 было 30/422: +8 кейсов ScheduleService/ScheduleCache — свой день/группа, смещение offset, фильтр статусов и окна days, prev_*, кэш — попадание до инвалидации и invalidate(group_id), fail-closed при ошибке БД; +6 кейсов IcsBuilder — структура VCALENDAR, CRLF, экранирование RFC 5545, DATE-TIME, SUMMARY-статусы, пустой набор; security_tests — до Э4 шага 3 заглушка). Актуальные счётчики на текущий main — блок **BE-часть Э4** ниже; контурный прогон — `docker compose --profile tests run --rm tests` (команда выше).

- **BE-часть Э4** (фактически проверено 06.11.2026 в одноразовом контейнере `drogonframework/drogon:v1.9.14` — сборка в отдельном томе, не `docker compose`; compose-прогоны гейта поверх того же кода — ниже):
  - **оценки и сессия (JSON, все роуты под LoginFilter; тело ошибок — 02 §5.1):**
    ```bash
    curl -s -c c.txt -X POST localhost:8080/v1/auth/login \
      -H 'Content-Type: application/json' \
      -d '{"login":"mark.demo","password":"<DEMO_PASSWORD>"}'
    curl -s -b c.txt localhost:8080/v1/grades            # 200 {"grades":[10 строк оценок Марка ids 1–10: id, discipline_id, code, discipline, control_type, component, score, max_score, attempt, comment, graded_at]}
    curl -s -b c.txt localhost:8080/v1/grades/summary    # 200 {"average":83,"disciplines":[6 дисциплин]} — «Мировая экономика» score_sum 78 / max_sum 82, percent 95, missing 4 («до допуска не хватает 4», 05 §3 Э4)
    curl -s -b c.txt localhost:8080/v1/grades/1          # 200 объект оценки id=1 (ME-101, «модуль 1», 30/30, comment null)
    curl -s -b c.txt localhost:8080/v1/grades/999999     # 404 {"error":{"code":"not_found",…}}
    curl -s -b c.txt localhost:8080/v1/exams             # 200 {"exams":[3 события по starts_at: retake 18.11/ауд. 1108 (История МЭО), consultation 21.11/4210, exam 24.11/4210]}
    curl -s localhost:8080/v1/grades                     # 401 {"error":{"code":"unauthorized",…}} — так же /grades/summary, /grades/{id}, /exams
    ```
    (в проверках сессии вход замещался прямой вставкой валидной сессии в `sessions` — демо-пароль вне репо, §5.2); пустое состояние (anya.demo — успеваемости нет, С5.6): `/v1/grades` → 200 `{"grades":[]}`, `/v1/grades/summary` → 200 `{"average":null,"disciplines":[]}`, `/v1/exams` → 200 `{"exams":[]}`;
  - **чужой grade → 403 (владение — только OwnershipGuard, 02 §6.3; дублирующего user_id-условия в выборке нет):**
    ```bash
    # с сессией anya.demo (оценок у Ани нет — любая существующая grade чужая):
    curl -s -b c.txt localhost:8080/v1/grades/1          # 403 {"error":{"code":"forbidden","message":"Доступ только к своим данным"}}
    ```
    (несуществующая — 404 not_found, не маскируется под 403; /v1/grades/summary не ловится роутом {id} — статический путь приоритетнее, сверено E2E; нечисловой id → 400 на стороне drogon);
  - **тесты:** `ctest` зелёный в одноразовом контейнере — para-tests **42/42 кейса, 757 утверждений** (пересчитано чистой сборкой 06.11.2026; +5 кейсов шага 1: пустой список и пустая сводка, список фикстур и изоляция, сводка Марка 78/82/missing 4/average 83, одна строка → average 100, fail-closed; +3 кейса шага 2: guard-сценарии `Table::Grades` — своё/чужое/несуществующее/ошибка БД через настоящий `OwnershipGuard::check`, gradeById Found/NotFound/Error, exams-выборка по пользователю с изоляцией) и security_tests — **реальный шаг 4 автотеста 02 §9.3** (DoD №2): две сессии Аня/Марк, чужой grade → 403 (все 10 id), свой → 200, несуществующий → 404, ошибка БД → fail-closed — «10/10 проверок, шаг 4 ПРОЙДЕН» (полный HTTP-вариант с живыми cookie-сессиями и шаги 1–3, 5–8 — Э8); контурный прогон — `docker compose --profile tests run --rm tests` (команда выше).

- **BE-часть Э5** (фактически проверено 06.11.2026 в одноразовом контейнере `drogonframework/drogon:v1.9.14` — сборка в /tmp с `-DHPDF_ROOT` на собранную из исходников libharu v2.4.6 (Р-12), не `docker compose`; compose-прогоны гейта поверх того же кода — ниже):
  - **заявки: подача, черновик, таймлайн, диалог (JSON, роуты под LoginFilter; тело ошибок — 02 §5.1):**
    ```bash
    curl -s -c c.txt -X POST localhost:8080/v1/auth/login \
      -H 'Content-Type: application/json' \
      -d '{"login":"dina.demo","password":"<DEMO_PASSWORD>"}'
    curl -s -b c.txt localhost:8080/v1/application-types   # 200 {"types":[5 типов каталога 03 §4.5.2: study_certificate, period_certificate, academic_certificate, thesis_topic, other]}
    curl -s -b c.txt -X POST localhost:8080/v1/applications \
      -H 'Content-Type: application/json' \
      -d '{"type":"academic_certificate","payload_json":"{\"copies\":2}","submit":true}'   # 201 {"id":N,"status":"submitted",…} + событие подачи NULL→submitted в таймлайне
    curl -s -b c.txt -X POST localhost:8080/v1/applications \
      -H 'Content-Type: application/json' \
      -d '{"type":"thesis_topic","payload_json":"{}","submit":false}'   # 201 {"status":"draft"} — черновик без событий
    curl -s -b c.txt localhost:8080/v1/applications             # 200 свои заявки, новые сверху; ?status=draft → фильтр; ?status=bogus → 400
    curl -s -b c.txt localhost:8080/v1/applications/1           # 200 карточка + timeline[] (fixtures: NULL→submitted, submitted→accepted «Принята в работа», accepted→in_progress)
    curl -s -b c.txt -X PUT localhost:8080/v1/applications/4 \
      -H 'Content-Type: application/json' -d '{"payload_json":"{\"topic\":\"…\"}"}'   # 200 — автосохранение своего черновика (С7; заявка создана POST-ом выше)
    curl -s -b c.txt -X PUT localhost:8080/v1/applications/5 \
      -H 'Content-Type: application/json' -d '{"payload_json":"{}"}'  # 409 {"error":{"code":"invalid_status_transition","message":"Править можно только черновик заявки"}} (свой submitted)
    curl -s -b c.txt -X PUT localhost:8080/v1/applications/3 …  # 403 forbidden «Доступ только к своим данным» — чужой черновик (guard ДО проверки статуса)
    curl -s -b c.txt -X POST localhost:8080/v1/applications/1/messages \
      -H 'Content-Type: application/json' -d '{"text":"вопрос по справке"}'   # 201 событие in_progress→in_progress с комментарием, автор — студент (+ application_status — итоговый статус заявки в ответе, правки по гейту раунд 1)
    curl -s -b c.txt -X POST localhost:8080/v1/applications/2/messages …  # 403 чужая
    ```
    (в проверках сессии вход замещался прямой вставкой валидной сессии в `sessions` — демо-пароль вне репо, §5.2); type вне каталога → 400, payload_json не-JSON → 400, пустой text → 400;
  - **С7.6 — возврат «В работе» после ответа студента (need_info→in_progress в матрице, шаг 1):**
    ```bash
    # заявку 5 перевести в need_info (на демо — staff-переходом; в проверках — sqlite-манипуляцией контура):
    curl -s -b c.txt -X POST localhost:8080/v1/applications/5/messages \
      -H 'Content-Type: application/json' \
      -d '{"text":"кафедра — мировая экономика"}'   # 201 событие need_info→in_progress, комментарий — текст ответа, автор — студент (+ application_status:"in_progress" — возврат «В работе» читается прямо в ответе, правки по гейту раунд 1)
    # статус заявки 5 после ответа: in_progress (проверено sqlite-SELECT и GET-карточкой)
    ```
  - **staff: очередь и смена статуса (StaffFilter; dean.demo — роль staff):**
    ```bash
    curl -s -c s.txt -X POST localhost:8080/v1/auth/login \
      -H 'Content-Type: application/json' \
      -d '{"login":"dean.demo","password":"<DEMO_PASSWORD>"}'
    curl -s -b s.txt localhost:8080/v1/staff/applications   # 200 очередь по updated_at ASC: owner fio/группа, флаг overdue «горит» (3 дня ожидания в активном статусе); ?status=submitted → фильтр; ?type=academic_certificate → фильтр; ?overdue=1 → только «горящие»; ?status=bogus и ?overdue=0 → 400
    curl -s -b c.txt localhost:8080/v1/staff/applications   # 403 forbidden «Доступ только для сотрудников» — так же POST status
    curl -s -b s.txt -X POST localhost:8080/v1/staff/applications/2/status \
      -H 'Content-Type: application/json' -d '{"to_status":"accepted"}'   # 200 карточка в новом статусе + уведомление АВТОРУ заявки (notifications: type application, title «Справка об обучении: Принята», deep_link /applications/2) + событие таймлайна author=staff
    curl -s -b s.txt -X POST localhost:8080/v1/staff/applications/2/status \
      -H 'Content-Type: application/json' -d '{"to_status":"ready"}'      # 409 invalid_status_transition «Недопустимый переход статуса заявки» — уведомление/статус не изменились (полный откат транзакции)
    curl -s -b s.txt -X POST localhost:8080/v1/staff/applications/999999/status …  # 404; to_status:"bogus" → 400 (ошибка тела, не перехода)
    ```
  - **PDF-справка (OwnershipGuard до проверки статуса; штамп):**
    ```bash
    curl -s -b c.txt localhost:8080/v1/applications/1/pdf   # 409 {"error":{"code":"not_ready","message":"Справка ещё не готова"}} — статус не ready|issued
    # заявку 1 → ready (на демо — staff-переходом in_progress→ready; в проверках — sqlite-манипуляцией контура):
    curl -s -b c.txt localhost:8080/v1/applications/1/pdf -o spravka.pdf
    # 200 application/pdf; %PDF-1.3; /FontFile2 (кириллица встроенным DejaVuSans.ttf); штамп-маркер в файле: DEMO-FAKE-DATA (ASCII-хвост KEYWORDS — находится и сырым поиском, и в любой декодированной форме; правки по гейту раунд 1); кириллическое «ДЕМО» в СЫРЫХ байтах — OCTAL-эскейпы UTF-8 \320\224\320\225\320\234\320\236 (поведение v2.4.6, Р-12; при декодировании PDF-парсером — mojibake, поэтому канон — ASCII-маркер)
    curl -s -b c.txt localhost:8080/v1/applications/2/pdf   # 403 — чужая (guard ДО проверки статуса: даже в issued → 403, не 409)
    curl -s -b c.txt localhost:8080/v1/applications/999999/pdf  # 404
    ```
  - **тесты:** `ctest` зелёный в одноразовом контейнере — para-tests **60/60 кейсов, 1978 утверждений** (пересчитано чистой сборкой 06.11.2026; +5 кейсов шага 1: все 16 переходов матрицы применены и события записаны, С7.6-жизненный цикл draft→…→issued с вопросом/ответом, исчерпывающий перебор 65 невалидных пар словаря (81−16) — статус/таймлайн не тронуты, NotFound/fail-closed, согласованность матрицы со словарём CHECK; +8 кейсов шага 2: каталог и parse-400, черновик+PUT-автосохранение, PUT не-draft → NotDraft, submit+событие подачи, список-изоляция+фильтр, messages-событие, С7.6-возврат, NotFound/fail-closed; +2 кейса шага 3: фильтры очереди на детерминированных updated_at (без фильтров/status/type/overdue/комбо, owner, overdue-флаги) и смена статуса (событие+уведомление автору, rejected с причиной в body, недопустимый переход — полный откат без уведомления, NotFound, fail-closed); +3 кейса шага 4: %PDF-//FontFile2/маркер в потоке, маркер в сохранённом файле, битый шрифт → ok=false; 42+5+8+2+3=60 — сходится) и security_tests — шаг 4 автотеста 02 §9.3 «10/10 проверок, шаг 4 ПРОЙДЕН»; контурный прогон — `docker compose --profile tests run --rm tests` (команда выше).

## 5. Соглашения (нарушать нельзя)

1. Только вымышленные данные; бейдж «концепт для хакатона, данные вымышленные» на входе и в подвале всех экранов.
2. Пароли — только argon2id-хэши; секреты и ключи — только в переменных окружения сервера, не в коде, не в репозитории, не в браузере.
3. Владение каждым объектом (заявка, оценка, уведомление, бронь) сверяется с сессией на сервере; чужой id → 403 — это проверяет `tests/security_tests.cpp`, его вывод прикладывается к отчёту.
4. Сторонняя библиотека подключается только после проверки лицензии (пермиссивная, без копилефта) и даты последнего коммита; запись об этом — в `docs/decisions.md`.
5. Контроллер тонкий, логика в сервисе; SQL — в сервисах; миграции нумерованные, применяются при старте; прямых обращений к БД из контроллеров нет.
6. Стиль и цвета — только из токенов 03; новая цветовая пара проходит `tools/contrast_check.py` с результатом 0 FAIL до коммита.
7. Каждый промпт ИИ-агенту и его результат фиксируются в `docs/agents-log/` — по ходу работы, не задним числом (критерий жюри «Масштаб использования нейротехнологий»).
8. Дорожки BE/FE (05 §4.5): `static/`, `views/` и токены 03 правит только FE-дорожка; `sql/`, `src/services`, фильтры и деплой — только BE. Общие файлы (`main.cpp`, `config.json`) — владелец BE; FE запрашивает изменение задачей. Шаблон и сервис одного экрана правит тот, кто вёз экран, — одним коммитом.

## 6. Текущий статус

- **Дата обновления:** 06.11.2026 · **Этап:** 5 — **BE-часть: закрыта** (шаги 1–5, артефакты ниже); **честная пометка:** FE-часть Э5 — за FE-дорожкой, этой сессией не начиналась (`static/` и `views/` не тронуты): каталог справок и «Новая заявка» (03 §4.5.1–4.5.2), форма 1–2 вопроса с автосохранением черновика (С2/С7), карточка заявки с таймлайном и мини-диалогом need_info, экран «Мои заявки» с пустым состоянием, очередь сотрудника с чипами-фильтрами и бейджем «горит», кнопки смены статуса, кнопка «Скачать PDF» (роут уже отдаёт файл); данные для FE уже отдаются: `/v1/application-types`, POST/GET `/v1/applications`, GET/PUT `/v1/applications/{id}`, POST `/v1/applications/{id}/messages`, GET `/v1/applications/{id}/pdf`, GET `/v1/staff/applications`, POST `/v1/staff/applications/{id}/status`. FE-части Э2 (профиль в UI, тема, онбординг-чеклист С5), Э3 (schedule.csp, бейджи «было → стало», экран «Изменения недели», карточка подписки ICS) и Э4 (grades.csp, карточка дисциплины с прогнозом допуска, «Сессия» с «В календарь») — тоже за FE-дорожкой. Следующий BE-этап — Э6 (уведомления, гайд BE §9) и вставки Э6а/Э6б. Первый деплой на VPS — Э9 (BE-гайд §11).
- Пакет решений 01–05 готов и отрецензирован; дорожки BE/FE (05 §4.5), гайды в `гайды/`.
- **Расширение объёма (решение от 05.11):** добавлены идеи 5.11–5.16 — профили людей с почтой `@example.edu` и расписанием (5.11), библиотека с наличием и читательским билетом (5.12), столовая с ценами и КБЖУ (5.13), рейтинг факультета с маскированием имён на сервере (5.14), зачётная книжка (5.15), карта корпусов с поиском кабинета (5.16). Зафиксировано в 01/02/03; этапы **Э6а/Э6б** — в 05 §3; DDL — миграция `0006_campus_services` в 02 §4.1. Приватность: чужой студент — только публичные поля и расписание его группы; оценки/заявки/брони чужих — никогда; DoD №2 не меняется. При ближайшем коммите: запись в `para/docs/decisions.md` (Р-10) и синхронизация копий `para/docs/`.
- **Артефакты Э1** (репозиторий `para/`, ветка `main`; корень репо — папка `para/`: штаб — задание, `решения/`, `гайды/` — канонические, в репо копии в `docs/`):
  - шаги 1–3 — каркас и Docker-контур (коммиты `d4d4565`, `e06924e`, ревью `cc37ab3`): дерево 05 §2, README/LICENSE MIT/`.gitignore`/env.example, CMake-скаффолд (`para`, `para-tests`, `security_tests`; `HealthCtrl` → GET `/healthz`), doctest v2.5.3 вендорен, Dockerfile multi-stage (`FROM --platform=linux/amd64`), docker-compose (порт 8080, том `para-data`, USER 10001); решения Р-1…Р-4;
  - шаги 4–7 — БД и вход (коммиты `f85194c`, `b0e19c6`, `b277f13`, `1c540db` + правки `0222161`, `d484eff`; журнал сессии `03df89f`): шаг 4 — миграционный раннер (`schema_migrations`, транзакция на файл, идемпотентный `--seed` маркером `fixtures`, Р-5); шаг 5 — миграции `0001…0005` (21 таблица, DDL 02 §4.1 + индексы §4.2); шаг 6 — фикстуры `sql/fixtures.sql` (5 аккаунтов, демо-неделя 2–8.11.2026, заявки, оценки, уведомления; Р-6); шаг 7 — вход (`AuthCtrl`, `LoginFilter`, сессии, лимитер 3→15 с с удвоением до 900 с; Р-7); правки гейта/ревью: `--seed` в ENTRYPOINT (Р-8), демо-пароль из decisions.md (§5.2), SQL из контроллера в `UserService` (§5.5);
  - три задачи закрытия (workflow, 05.11.2026): `42ab5a1` — ротация демо-пароля фикстур (все 5 `password_hash` перегенерированы argon2id t=2 m=19456 p=1, соль на аккаунт, верификация `argon2id_verify` в одноразовом контейнере; Р-9); `d3bc367` — формат ошибок API приведён к 02 §5.1 `{error:{code,message}}` (единый хелпер `src/utils/ApiError.h`; 6 ответов `AuthCtrl`/`LoginFilter`); актуализация README и AGENTS.md — данным коммитом;
  - гейт шагов 4–7: два раунда (раунд 0 красный — ENTRYPOINT без `--seed`, исправлено `0222161`; раунд 1 зелёный); ревью шагов 4–7: 8 находок, 2 обязательных применены (`d484eff`); medium/low (`foreign_keys=0`, FATAL MULTITHREAD в логе старта, CSP/nosniff, timing-оракул входа и др.) — без mustFix, учтены для плана Э8; журнал — `para/docs/agents-log/2026-11-05-э1-шаги-4-7-агентская-сессия.md`.
- **Честный статус шага 8 (push):** этой сессией НЕ выполнялся — выполняется владельцем отдельным шагом после финального смока гейта закрытия Э1; remote `git@github.com:vertaler3/para.git` настроен (приватный GitHub, ветка `main`); после push — клон на второй машине и сборка строго по README (гайд BE §2 шаг 8). Шаг 9 (ctest + коммит + актуализация AGENTS.md) — выполнен данным коммитом.
- **Артефакты Э2 — BE-часть** (репозиторий `para/`, ветка `main`; `static/` и `views/` не тронуты — FE-дорожка):
  - **шаг 1 — OwnershipGuard** (коммит `719194e`): `src/services/OwnershipGuard.{h,cpp}` — единственная функция `check()` по 02 §6.3: fetch-and-compare из (session_user_id, таблица, id из пути), вердикты Own/NotFound/Forbidden (+ Error — fail-closed при ошибке БД); белый список таблиц `Table{Applications,Grades,Notifications}` → фиксированные SQL-литералы, id — параметром (`$1`); правило код-ревью «контроллер с path-id обязан вызвать OwnershipGuard» зафиксировано в шапке заголовка; юнит-тесты на временной SQLite — 3 случая (своё → Own, чужое → Forbidden/403, несуществующее → NotFound/404) + fail-closed Error. В ходе шага пойман и исправлен двойной move колбэка в цепочке `>>` (пустой `std::function` → `std::bad_function_call` на error-пути БД); латентный аналог остался в `UserService::findByLogin/loadProfile` и `SessionService::findValid` (Э1-код, вне шага не правил) — учтено для плана Э8; журнал — `para/docs/agents-log/2026-11-05-э2-шаг-1-ownership-guard.md`.
  - **шаг 2 — GET /v1/me** (правок не потребовалось — коммита нет): сверён с 02 §5.2 и §4.1 — все шесть полей отдаются (`profile.fio/faculty/program/course/group/student_card_no`, SQL — `UserService::loadProfile`, контроллер тонкий); проверено E2E на фикстурах: 200 с 6/6 полями (машинная сверка с `sql/fixtures.sql`), без cookie → 401 в формате 02 §5.1.
  - **шаг 3 — настройки уведомлений** (коммит `aa9ae51`): `src/services/SettingsService.{h,cpp}` (load — SELECT строки settings по user_id, при отсутствии — дефолты DEFAULT из миграции 0004; save — UPSERT `ON CONFLICT(user_id) DO UPDATE`; applyPatch — частичное обновление, theme ∈ light/dark/system, `*_json` — валидный JSON) + тонкий `SettingsCtrl` — GET/PUT `/v1/notifications/settings` под LoginFilter; `user_id` из тела PUT игнорируется; страница /profile не обязательна — views/ не тронуты; юнит-тесты (para-tests 16/16 кейсов, 219 утверждений); журнал — `para/docs/agents-log/2026-11-05-э2-шаг-3-settings.md`.
  - **шаг 4** — актуализация AGENTS.md (данным коммитом) + синхронизация копии `para/AGENTS.md`.
- **Честный статус проверок Э2:** compose-команды (`docker compose up/build`, `--profile tests`) этой сессией НЕ запускались — их выполняет внешний гейт; проверки сессии — одноразовый контейнер `drogonframework/drogon:v1.9.14` (сборка 0 warnings, ctest 100%, E2E curl-цепочка на порту 18080 с временной БД и фикстурами; вход замещён прямой вставкой валидной сессии в `sessions`, демо-пароль вне репо не использовался).
- **Э3 — BE-часть: закрыта** (свёрнуто в одну строку решением шага 4 Э4; детали с хэшами и журналами: шаг 0 `f456145` — синхронизация решений Р-10; шаг 1 `5a3825a` — ScheduleCtrl today/week/changes + ScheduleCache TTL 60 с, кэш только offset=0; шаг 2 `5243d6f` — ICS-подписка 5.4 (IcsBuilder RFC 5545, IcsService — токен 256 бит/64 hex, Р-11, публичный GET /ics/{token}, POST-ротация/DELETE-отзыв); шаг 3 `8920be6` — AGENTS.md; правки гейта: раунд 1 `d4d51d9` (токен 128→256 бит, Р-11), раунд 2 `d08e89f` (гейт шёл на старом образе — доказано таймингами; устранён двойной Content-Type у GET /ics/{token} → `setContentTypeString`; контракт Р-11 закреплён 4 кейсами); журналы — `para/docs/agents-log/2026-11-05-э3-*.md` и `2026-11-06-э3-правки-по-гейту-раунд-2.md`; «ICS открывается в календаре телефона» — ручная проверка на устройстве не воспроизводима в контейнере, за FE-дорожкой/ручной верификацией). **Открытый вопрос (перенесён в раздел 8):** статус-код POST /v1/ics/token — API отдаёт 201, гейт Э3 ожидал 200 — решить на Э8.
- **Артефакты Э4 — BE-часть** (репозиторий `para/`, ветка `main`; `static/` и `views/` не тронуты — FE-дорожка):
  - **шаг 1 — GradesService: GET /v1/grades и /grades/summary** (коммит `4f1ac75`): `src/services/GradesService.{h,cpp}` — вся SQL параметрами по `idx_grades_user` (EXPLAIN QUERY PLAN проверен): список своих оценок (user_id из сессии, JOIN disciplines) и сводка семестра — порог допуска = Σ max_score компонентов дисциплины, `percent`/`missing` («что осталось»; демо-арифметика 05 §3 Э4: «Мировая экономика» 78/82, не хватает 4), `average` — взвешенный процент Σscore/Σmax, целые (jsoncpp в образе v1.9.14 сериализует дробные двойники полной репрезентацией: «83.2» → «83.200000000000003» — проба в контейнере); пустое состояние Ани — `{"average":null,"disciplines":[]}` (сверено байт-в-байт E2E); fail-closed при ошибке БД; тонкий `GradesCtrl` — 2 GET под LoginFilter; юнит-тесты +5 кейсов; в постановке шага «у Марка 8 записей» — фактически в фикстурах 10 (расхождение зафиксировано в журнале, тесты сверены с фактом); журнал — `para/docs/agents-log/2026-11-06-э4-шаг-1-grades.md`.
  - **шаг 2 — GET /v1/grades/{id} через OwnershipGuard; GET /v1/exams** (коммит `ec4de32`): первый path-id эндпоинт проекта — `OwnershipGuard::check(Table::Grades)` ДО любых данных (правило код-ревью OwnershipGuard.h; Table::Grades в белом списке с Э2 — сам guard не менялся); Own → 200, NotFound → 404 `not_found`, Forbidden → 403 `forbidden` «Доступ только к своим данным» (дословный пример 02 §5.1), Error → 500 fail-closed; владение ровно один раз — `gradeById` по PK без user_id-условия (постановка: не дублировать проверку мимо guard); `/v1/exams` — session_plan пользователя по `starts_at` (`idx_session_plan_user`, EXPLAIN проверен), все kind CHECK DDL без фильтра, Аня/staff — 200 `{"exams":[]}`; статический `/grades/summary` приоритетнее `{id}` — сверено E2E; юнит-тесты +3 кейса (guard-сценарии Grades, gradeById, exams-изоляция); журнал — `para/docs/agents-log/2026-11-06-э4-шаг-2-guard-exams.md`.
  - **шаг 3 — security-шаг 4 автотеста 02 §9.3** (коммит `bc25322`): `tests/security_tests.cpp` — заглушка Э1 заменена реальным сценарием (цель security_tests осталась самостоятельным ctest-тестом, CMakeLists линкует drogon/sqlite + сервисы): две сессии Аня user 1 (пустая успеваемость — предусловия проверяются) / Марк user 3 (10 оценок фикстур); чужой grade → Forbidden/403 — все 10 id, свой → Own + 200/Ok (та же цепочка guard → gradeById, что под контроллером), несуществующий → NotFound/404 для обеих сессий, ошибка БД → Error fail-closed; фактический прогон «10/10 проверок, шаг 4 ПРОЙДЕН» (exit 0), вывод прикладывается к отчёту (DoD №2); полный HTTP-вариант с живыми cookie-сессиями и шаги 1–3, 5–8 §9.3 — Э8 (честная шапка файла, FAIL → exit 1); журнал — `para/docs/agents-log/2026-11-06-э4-шаг-3-security-шаг-4.md`.
  - **шаг 4** — актуализация AGENTS.md (данным коммитом) + синхронизация копии `para/AGENTS.md`; устаревшие счётчики тестов исправлены в двух местах (было «30/30 кейсов, 422 утверждения» → на закрытие Э3 фактически 34/580 — журнал раунда 2, в блоке BE-часть Э3; на текущий main 42/757 — пересчитано чистой сборкой para-tests в одноразовом контейнере 06.11.2026, в блоке BE-часть Э4).
- **Честный статус проверок Э4:** compose-команды (`docker compose up/build`, `--profile tests`) этой сессией НЕ запускались — их выполняет внешний гейт; проверки — одноразовый контейнер `drogonframework/drogon:v1.9.14` (сборка 0 warnings в новых/изменённых файлах; ctest 100% — para-tests 42/42 кейсов, 757 утверждений + security_tests шаг 4; E2E curl-цепочки на порту 18080 с временной БД и фикстурами: список/сводка/`grades/{id}`/`exams` Марка, пустые состояния Ани, 401 без cookie, 403 чужой grade, 404 несуществующий, приоритет `/grades/summary`; вход замещён прямой вставкой валидной сессии в `sessions`, демо-пароль вне репо не использовался).
- **Артефакты Э5 — BE-часть** (репозиторий `para/`, ветка `main`; `static/` и `views/` не тронуты — FE-дорожка):
  - **шаг 1 — AppWorkflow: автомат статусов + application_events** (коммит `10813de`): матрица 16 переходов constexpr в `src/models/Applications.h` (05 §3 Э5: draft→submitted|cancelled; submitted→accepted|rejected|cancelled; accepted→in_progress|rejected|cancelled; in_progress→ready|need_info|rejected|cancelled; need_info→in_progress (С7.6 — «статус возвращается „В работе"»)|rejected|cancelled; ready→issued; переходы ИЗ issued/rejected/cancelled запрещены; rejected/cancelled включены — 01 §4.4 и словарь CHECK DDL 02 §4.1) + constexpr-предикат; `AppWorkflow::apply` — чтение статуса по PK (владение — guard контроллера) → решение по матрице → транзакция: условный UPDATE (0 строк — гонка статуса → InvalidTransition) + INSERT application_events (from/to/comment/author); исходы Applied/InvalidTransition (409 контроллером)/NotFound (404)/Error (fail-closed 500); юнит-тесты: ВСЕ 16 переходов применены и события записаны, С7.6-жизненный цикл draft→…→issued, исчерпывающий перебор 65 невалидных пар словаря (81−16) — статус/таймлайн не тронуты, NotFound/fail-closed, согласованность матрицы со словарём; журнал — `para/docs/agents-log/2026-11-06-э5-шаг-1-appworkflow.md`.
  - **шаг 2 — эндпоинты заявок** (коммит `ed44c72`): каталог `kApplicationTypes` (5 типов 03 §4.5.2, ключи = фикстурам) + GET `/v1/application-types`; `ApplicationsService` — POST `/v1/applications` (draft|submitted; при submit — событие подачи NULL→submitted, черновик без событий), GET список (+query status), GET {id} карточка+таймлайн (guard → по PK), PUT {id} payload — ТОЛЬКО draft (409 `invalid_status_transition`), POST {id}/messages — текст событием с комментарием (автор — текущий пользователь); **С7.6**: заявка в need_info → возврат в in_progress через автомат (событие-переход с текстом ответа); тонкий `ApplicationsCtrl`; юнит-тесты +8 кейсов. **Находка шага (поймана юнит-тестами):** в SQLite `$N` — имя параметра с индексом по первому вхождению в текст SQL (не как у `?N`), а drogon биндит аргументы позиционно — `INSERT..SELECT` с `$2,$3` раньше `$1` молча вставлял 0 строк; переписано, правило «порядок $N в тексте = порядку биндинга» зафиксировано в комментариях и журнале; журнал — `para/docs/agents-log/2026-11-06-э5-шаг-2-эндпоинты-заявок.md`.
  - **шаг 3 — staff: очередь и смена статуса** (коммит `2f139f8`): `StaffFilter` — роль из атрибута сессии (users.role через `SessionService::findValid`; параметр от клиента не принимается), не staff → 403, fail-closed при отсутствии атрибута (`Attributes::get` возвращает `T()` — сверено по заголовкам v1.9.14); GET `/v1/staff/applications` — фильтры status/type/overdue=1 одной параметризованной SQL «filters-or-empty», overdue («горит», 03 §4.8) = 3 дня ожидания в активном статусе (С2.4 «до 3 рабочих дней» — консервативно в календарных), draft/терминальные не горят, ORDER BY updated_at ASC, owner fio/группа (LEFT JOIN profiles/groups, С8.3); POST `/v1/staff/applications/{id}/status` — автомат + уведомление АВТОРУ заявки (notifications type 'application', title «Тип: Статус» по меткам models, deep_link /applications/{id}) В ТОЙ ЖЕ транзакции (новый опциональный хук `AppWorkflow::AfterEventTx` — ошибка уведомления откатывает весь переход); OwnershipGuard сознательно не вызывается (staff видит все — постановка), несуществующий → 404; юнит-тесты +2 кейса; журнал — `para/docs/agents-log/2026-11-06-э5-шаг-3-staff.md`.
  - **шаг 4 — PdfBuilder: PDF-справка со штампом ДЕМО** (коммит `4c503ca`): libharu v2.4.6 из исходников на build-слое Dockerfile с пином коммита `3467749fd1c0ab6ca6ed424d053b1ea53c1bf67c` (RUN проверяет rev-parse; лицензия zlib/libPNG; факт-фикс даты тега — 26.03.2026, не 11.03.2025) + DejaVuSans.ttf (apt fonts-dejavu-core → `/opt/para/fonts/`; лицензия bitstream-vera) — **Р-12** в decisions.md; `PdfBuilder`: A4-справка из профиля, кириллица встроенным TTF (`/FontFile2`), диагональный штамп 45° (SetTextMatrix) + маркер в сырых байтах — KEYWORDS как OCTAL-эскейпы UTF-8 «ДЕМО» = `\320\224\320\225\320\234\320\236` (**НЕ hex UTF-16BE, как предполагала постановка** — установлено пробником v2.4.6; **после гейта раунд 1 канон машинного маркера — ASCII-хвост `DEMO-FAKE-DATA`**: октал-эскейпы не находятся при декодировании PDF-текста — см. честный статус Э5), охраняется тестом); GET `/v1/applications/{id}/pdf` (`PdfCtrl`) — guard ДО проверки статуса (чужая даже issued → 403), не ready|issued → 409 `not_ready`, ready|issued → 200 application/pdf; юнит-тесты +3 кейса (включая маркер в сохранённом файле); журнал — `para/docs/agents-log/2026-11-06-э5-шаг-4-pdf.md`.
  - **шаг 5** — актуализация AGENTS.md (данным коммитом) + синхронизация копии `para/AGENTS.md`; счётчики тестов пересчитаны чистой сборкой para-tests в одноразовом контейнере 06.11.2026 (60/1978 — в блоке BE-часть Э5 раздела 4).
- **Честный статус проверок Э5:** compose-команды (`docker compose up/build`, `--profile tests`) этой сессией НЕ запускались — их выполняет внешний гейт; проверки — одноразовый контейнер `drogonframework/drogon:v1.9.14` (сборка -Wall -Wextra — 0 warnings в файлах шагов; ctest 100% — para-tests **60/60 кейсов, 1978 утверждений** + security_tests шаг 4 «10/10»; E2E curl-цепочки на порту 18080 с временной БД и фикстурами: С2 по curl — подача (201 draft|submitted) → список/таймлайн → статусы staff с уведомлением автору → PDF со штампом (200 application/pdf, маркер в сырых байтах); С7 — need_info → сообщение студента → возврат «В работе» (событие-переход с текстом); недопустимый переход → 409 (полный откат без уведомления); StaffFilter студент → 403; чужой id → 403 (guard до статуса, даже issued); 401/400/404; вход замещён прямой вставкой валидной сессии в `sessions`, демо-пароль вне репо не использовался). **Правки по гейту (раунд 1, 06.11.2026):** машинный маркер штампа канонизирован как ASCII-хвост `DEMO-FAKE-DATA` (октал-эскейпы «ДЕМО» не находятся при декодировании PDF-текста — Р-12 дополнена); 201-ответ messages дополнен полем `application_status` (статус заявки после сообщения — автоматический возврат С7.6 читаем прямо в ответе); полный прогон цепочки гейта после правок — зелёный (возврат подтверждён sqlite+GET+ответом, штамп — сырым поиском и после декодирования); журнал — `para/docs/agents-log/2026-11-06-э5-правки-по-гейту-раунд-1.md`.
- Как проверить — команды раздела 4 (фактически проверенные, включая блоки BE-частей Э2–Э5); значение демо-пароля в репо не хранится (§5.2) — публикуется в комментарии к работе.

## 7. Протокол ведения (обязательный)

1. Этап из 05 §3 закрыт = коммит, обновляющий этот файл: раздел 6 «Текущий статус» (этап, артефакты, как проверить) и раздел 8 «Открытые вопросы». **Этап без обновления AGENTS.md не считается закрытым.**
2. Каждая сессия (человек или агент) начинается с чтения этого файла; каждый промпт агенту начинается с фразы «Прочитай AGENTS.md в корне репозитория».
3. Изменили решение — сначала запись в `docs/decisions.md`, затем правка соответствующего документа в `решения/`, затем этот файл. Так AGENTS.md всегда описывает актуальное состояние, а не намерения.
4. Ведут файл по очереди все исполнители этапа; на чекпоинте этапа актуальность AGENTS.md проверяет тот, кто закрывает этап.

## 8. Открытые вопросы

- Статус-код POST /v1/ics/token: API отдаёт **201** (создание/ротация ресурса токена, Э3), гейт Э3 ожидал 200; 02 §5.2 код не фиксирует — выбрать канон (и поправить либо API, либо гейт) — решить на Э8.
- Выбор VPS-провайдера (кандидаты в 02 §8.5: Timeweb Cloud / Selectel / Beget / RuVDS) и проверка доступности из РФ без VPN.
- Порядок фазы 2 push (web-push) после М3 — решить на Э7; до фриза фич гарантия — WebSocket-канал.
- «Бронь аудитории» (5.10) — P1: если календарь сгорит, этап Э5а переносится после Э7 без влияния на ядро (решение фиксируется здесь и в `docs/decisions.md`).
- Ручная проверка «ICS открывается в календаре телефона» (гайд BE §5, Э3) — за FE-дорожкой/ручной верификацией: в контейнере проверены структура VCALENDAR, CRLF и полный HTTP-цикл (см. раздел 6), на устройстве — нет.
