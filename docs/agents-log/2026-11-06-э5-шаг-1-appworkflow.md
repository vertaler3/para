# Журнал ИИ-сессии — Э5, шаг 1: AppWorkflow — автомат статусов заявок + application_events (BE-дорожка)

- **Дата:** 06.11.2026 · **Дорожка:** BE · **Инструмент:** агентская сессия в составе workflow шагов Э5
- **Промпт (суть):** «Прочитай AGENTS.md в корне репозитория, гайд-бэкенд раздел 7 (Э5), 02 §4.1 (applications, application_events) и §5.2, а также 01 сценарии С2 и С7 (С7.6: после ответа студента статус возвращается „В работе" — матрица должна разрешать need_info → in_progress). Шаг 1 Э5 — автомат заявок: 1) Сервис src/services/AppWorkflow: матрица переходов (константа в src/models — по дереву 05 §2 „матрица переходов статусов"): submitted→accepted→in_progress→need_info|ready→issued; need_info→in_progress (С7.6); draft→submitted; отклонение/отмена — если 02/01 их предполагают (сверься). Недопустимый переход — ошибка, которую контроллер отдаёт как 409 в формате 02 §5.1. 2) Каждое применение перехода пишет строку в application_events (from_status, to_status, comment, author_id). 3) Юнит-тесты в tests/unit: ВСЕ валидные переходы матрицы (каждый применён и событие записано) + невалидные (например, submitted→issued, ready→in_progress) → ошибка перехода. 4) Коммит „Э5: шаг 1 — AppWorkflow: автомат статусов + application_events"».

## Что сделано

- **`src/models/Applications.h`** — constexpr-константы автомата по дереву 05 §2 («models/ — constexpr-константы: каталог application_types, матрица переходов статусов, бейдж концепта»; каталог типов — шаг 2 Э5, здесь его нет):
  - `kApplicationStatuses` — словарь 9 статусов, дословно CHECK DDL applications (02 §4.1, миграция 0003);
  - `StatusTransition{kFrom→to}` + `kApplicationStatusTransitions` — **матрица 16 пар «из → в»** (05 §3 Э5 п.1: «списком пар, не цепочкой»):
    `draft→submitted|cancelled · submitted→accepted|rejected|cancelled · accepted→in_progress|rejected|cancelled · in_progress→ready|need_info|rejected|cancelled · need_info→in_progress|rejected|cancelled · ready→issued · переходы ИЗ issued/rejected/cancelled запрещены`;
  - constexpr-предикат `canApplicationTransition(from, to)` — линейный проход по 16 парам.
  - **Сверка про rejected/cancelled (пункт постановки «сверься»):** включены — 01 §4.4 прямо называет ветки «отклонена», «отменена студентом» (01:198); словарь CHECK DDL 02 §4.1 содержит `rejected`/`cancelled`; каноническая матрица 05 §3 Э5 (строки 278–284) перечисляет их явно, с пометкой «rejected (с причиной)» и «переходы ИЗ issued/rejected/cancelled — запрещены, 409». Гайд BE §7 задаёт магистраль `submitted → accepted → in_progress → need_info|ready → issued` — она подмножество матрицы 05.
- **`src/services/AppWorkflow.{h,cpp}`** — сервис-автомат:
  - `canTransition(from, to)` — делегирует constexpr-предикату models (контроллер может валидировать 409 до обращения к БД);
  - `apply(db, applicationId, toStatus, comment?, authorId, cb)` — чтение текущего статуса по PK (БЕЗ user_id-условия: владение проверяет контроллер через `OwnershipGuard::check(Table::Applications)` ДО вызова — правило код-ревью OwnershipGuard.h, паттерн `gradeById` Э4) → решение по матрице → **drogon-транзакция** (`newTransactionAsync`): условный `UPDATE applications SET status=$1, updated_at=strftime(...) WHERE id=$2 AND status=$3` + `INSERT INTO application_events (application_id, from_status, to_status, comment, author_id)`;
  - семантика результатов: `Applied` / `InvalidTransition` (вне матрицы **или** гонка — UPDATE вернул 0 строк при несовпадении `status=$3`; явный `rollback()`) / `NotFound` (404) / `Error` (fail-closed → 500); `fromStatus` в результате — для сообщения 409 («из X в Y нельзя»). Маппинг в HTTP — контроллер шага 2 (409 в формате 02 §5.1);
  - `comment` — `std::optional<std::string>`: nullopt → SQL NULL (SqlBinder v1.9.14 биндит `std::optional`/`nullptr_t` — SqlBinder.h:523/545, проверено компиляцией); `authorId` — пользователь сессии, инициировавший переход;
  - колбэк стреляет ровно один раз (`shared_ptr<atomic_bool>` + `compare_exchange_strong`): commit-callback (стреляет при уничтожении транзакции — исходник TransactionImpl v1.9.14: commit в деструкторе, после rollback не стреляет вовсе) не может дублировать error-ветки; `tx` захвачен в колбэки INSERT — последний держатель уничтожается после завершения INSERT, деструктор коммитит;
  - стартовое событие `NULL→submitted` (как в fixtures.sql) — работа контроллера создания/отправки заявки (шаг 2), в `apply()` from_status всегда ненулевой; уведомление автору заявки — шаг 2 (05 §3 Э5: «INSERT в notifications» — в staff-эндпоинте).
- **`tests/unit/app_workflow_test.cpp`** — 5 тест-кейсов (временная SQLite, схема — колонки/индексы/CHECK миграции 0003; заявки засеиваются ИЗ константы матрицы — тест и матрица не расходятся):
  1. **все 16 переходов матрицы** применяются по одному на заявку (ids 11–26, по паре) → `Applied`, статус в БД = целевой, `updated_at` продлён, **ровно одно событие** с дословными from/to/author=5, comment NULL при nullopt, `at` — DEFAULT DDL;
  2. **С7.6 — жизненный цикл одной заявки**: draft→submitted→accepted→in_progress→need_info («укажите кафедру», С7.5)→**in_progress (ответ студента, С7.6)**→ready→issued — 7 событий в порядке записи, авторы студент (4)/сотрудник (5) по смыслу шагов, комментарии у need_info и у ответа;
  3. **исчерпывающий перебор невалидных пар словаря** (81−16=65, включая названные в постановке `submitted→issued`, `ready→in_progress` и «issued → in_progress нет» 02 §5.2 поток 2) → `InvalidTransition` с `fromStatus == from`; после перебора — статусы и `updated_at` всех заявок байт-в-байт как до, событий 0; неизвестный статус назначения (`bogus_status`) и неизвестные статусы на уровне константы — так же;
  4. нет заявки → `NotFound` (404); пустая схема → `Error` (fail-closed);
  5. согласованность константы: все пары из словаря, дубликатов нет, число исходящих по статусам {2,3,3,4,3,1,0,0,0} (терминальные — ноль, из `ready` единственный `issued`), ключевые переходы гайда и С7.6 отмечены явно.
- **`CMakeLists.txt`** — `AppWorkflow.cpp` в `para` и `para-tests`; тест в `para-tests`.

## Ошибки ИИ, пойманные и исправленные в ходе

1. **Неизвестный API транзакций v1.9.14 не выдумывался** — семантика снята с фактических заголовков образа (`grep` по `/usr/local/include/drogon/orm/`): `newTransaction/newTransactionAsync` — DbClient.h:290–308; `Transaction::rollback/setCommitCallback` — DbClient.h:437–441 (commit() нет — только комментарий-заглушка); момент коммита уточнён по исходнику TransactionImpl v1.9.14 (GitHub raw): commit — при уничтожении объекта, после rollback commit-callback не стреляет. Дизайн `apply()` построен на этих фактах, а не на памяти.
2. **`std::atomic` некопируем** — первая версия захватила `std::atomic_bool` по значению в лямбду `fire`; сборка бы не прошла/UB: заменено на `std::make_shared<std::atomic_bool>`.
3. **Опечатка `*tx` вне области захвата** — в лямбде `newTransactionAsync` параметр называется `transaction`, а оператор `<<` остался на `*tx` (gcc: «‘tx’ was not declared»); исправлено на `*transaction`.
4. **`DrogonDbException` без using** в тесте — unqualified имя не резолвилось, каскад «no match for call to lambda» в SqlBinder.h:273; добавлено `using drogon::orm::DrogonDbException`.
5. **doctest декомпозирует `&&` в CHECK** — `CHECK_FALSE(a && b)` → static assert «Expression Too Complex»; выражение сведено в `bool samePair`.
6. **Базовый образ не содержит `libargon2-dev`** (наш Dockerfile ставит его в build-стадии) — в одноразовом контейнере доустанавливался apt-пакет; сборка на Unix Makefiles (как в Э4).

## Проверки (одноразовый контейнер drogonframework/drogon:v1.9.14; сборка в /tmp внутри контейнера; docker compose up/build НЕ запускались — их выполняет внешний гейт)

- Сборка: `cmake -S /work -B /tmp/build -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-Wall -Wextra" && cmake --build /tmp/build -j` → exit 0; **в файлах шага (AppWorkflow.h/cpp, models/Applications.h, app_workflow_test.cpp) 0 warnings** (3 warnings всей сборки — прежние, не этого шага).
- `ctest --output-on-failure` → **100% tests passed, 0 failed out of 2** (para-tests + security_tests); doctest: **test cases: 47 | 47 passed, assertions: 1671 | 1671 passed** (было 42/757 после Э4; +5 кейсов, +914 утверждений этого шага). Список новых кейсов подтверждён `para-tests --list-test-cases`.
- **EXPLAIN QUERY PLAN** на БД из реальных миграций 0001–0005 + fixtures (sqlite3 в контейнере): `SELECT status FROM applications WHERE id=$1` и условный `UPDATE … WHERE id=$2 AND status=$3` — оба `SEARCH applications USING INTEGER PRIMARY KEY (rowid=?)` (поиск по PK; новых индексов шагу не нужно).

## Честно не сделано (вне шага 1, по постановке)

- Контроллеры/эндпоинты Э5 (`POST/GET /v1/applications`, `PUT /v1/applications/{id}`, таймлайн-карточка, `/messages`, staff-очередь, `POST /v1/staff/applications/{id}/status` с уведомлением автору, htmx-фрагменты) — шаг 2 Э5; 409-маппинг `InvalidTransition` и `OwnershipGuard::check(Table::Applications)` в path-id — там.
- `PdfBuilder` (libharu) — отдельный шаг Э5; каталог `application_types` и бейдж концепта — шаг 2.
- Интеграция С2/С7 через HTTP и OwnershipGuard-сценарии заявок в security_tests — шаг 2+ (гайд BE §7: «интеграция С2 и С7» на закрытие этапа).
- Обновление AGENTS.md (раздел 6) — шаг 4 Э5 по протоколу этапа.
