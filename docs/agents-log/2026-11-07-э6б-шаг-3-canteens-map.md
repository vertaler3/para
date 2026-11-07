# Э6б, шаг 3 — CanteensCtrl и MapCtrl: меню и поиск кабинета

- **Дата:** 07.11.2026
- **Агент:** сборщик Э6б (BE-дорожка, динамический workflow)
- **Промпт (суть):** «Прочитай AGENTS.md, гайд-бэкенд.md раздел 9б (Э6б), 02-техническое-решение.md §5.2 (строки /v1/canteens*, /v1/map*). Шаг 3 Э6б: 1) CanteensService + CanteensCtrl тонкий: GET /v1/canteens (список: корпус, часы), GET /v1/canteens/{id}/menu (цены, комплексные, КБЖУ; несуществующая → 404); 2) MapService + MapCtrl тонкий: GET /v1/map/buildings, GET /v1/map/rooms?q=<номер>&building_id= — поиск кабинета: номер, корпус, этаж, тип (rooms c building_id/floor/kind из 0006); 3) юнит-тесты: поиск кабинета по номеру (1207 → корпус/этаж/тип), меню с КБЖУ; 4) коммит. Верни CampusStep: roomQuery/roomName/roomFloor/roomBuildingId».

## Сделано

- **`src/services/CanteensService.{h,cpp}`** — вся SQL параметрами (AGENTS.md §5.5, 02 §6.4):
  - `list()` → `{"canteens":[{id, name, building_id, building, hours}]}` — JOIN buildings (02 §5.2 «корпус, часы»), ORDER BY id; пустая таблица — 200 [];
  - `canteenById()` — Lookup Found/NotFound/Error (паттерн RoomsService::roomById);
  - `menu()` → 200 `{"canteen_id", "canteen", "building", "hours", "menu":[{id, name, category, is_combo(bool), price(₽), kcal, protein, fat, carbs}]}` — двухфазно (существование → выборка по idx_menu_items_canteen, линейная цепочка — урок Э5а шаг 3), несуществующая → NotFound (404, 02 §5.2 «200 · 404»); КБЖУ nullable — вода без КБЖУ (fixtures Э6б) отдаётся null, не 0; пустое меню существующей столовой — 200 [];
  - каталог общий (OwnershipGuard не применяется — шапка файла; {id} — публичная столовая, не личный объект).
- **`src/services/MapService.{h,cpp}`**:
  - `buildings()` → `{"buildings":[{id, name, address}]}` — ORDER BY id;
  - `rooms()` → `{"rooms":[{id, name, building_id, building, floor, kind}]}` — поиск кабинета (гайд §9б «?q=1207 → корпус, этаж, тип»): фиксированная SQL «filters-or-empty» (паттерн kQueueSql; порядок первых вхождений $N = порядку биндинга — правило Э5 шаг 2, повторов нет), q — подстрока по name («204» находит «Б-204»; LIKE с ESCAPE '\' — спецсимволы экранированы, как в LibraryService), building_id — точный фильтр; INNER JOIN buildings — строки без корпуса (building_id NULL) карту не показывают; floor/kind nullable → null; сортировка по id.
- **`src/controllers/CanteensCtrl.{h,cpp}`, `src/controllers/MapCtrl.{h,cpp}`** — тонкие (AGENTS.md §5.5): четыре GET под LoginFilter (без cookie → 401 от фильтра); branch_id-стиль валидации: building_id не целое → 400 bad_request; несуществующая столовая → 404 not_found «Столовая не найдена»; успешные ответы — api::jsonResponse (кириллица сырыми UTF-8 байтами, гейт-конвенция Э6 р1), ошибки — api::errorResponse 02 §5.1.
- **`CMakeLists.txt`** — CanteensCtrl.cpp/MapCtrl.cpp в `para` (строки 90–91), CanteensService.cpp/MapService.cpp в `para` (118–119) и `para-tests` (195–196), `tests/unit/canteens_map_test.cpp` в `para-tests` (168).
- **`tests/unit/canteens_map_test.cpp`** — 11 кейсов / 125 утверждений на временной SQLite: обязательные из постановки — поиск «1207» → id 1, name «1207», building_id 2, «Корпус Б», floor 2, kind «аудитория» (контрольная строка гайда §9б и fixtures Э6а) и меню с КБЖУ (комбо is_combo true, price 320 ₽, kcal 620, 28/22/65; у воды kcal/protein/fat/carbs null); плюс list (состав/порядок/пустая), 404 несуществующей, пустое меню существующей (столовая 3 — помеченное дополнение теста), buildings, подстрока «204», фильтр building_id, комбо-фильтр, «%»-экранирование, без фильтров, fail-closed на БД без схемы. Данные — fixtures Э6а/Э6б дословно.

## Находки шага

1. **Расхождение сервиса с собственным контрактом шапки поймано юнит-тестом до коммита:** первая редакция menu() возвращала шапку столовой как {id, name, building_id, building, hours} (ключи canteenById), а контракт шапки CanteensCtrl.h и тест ждали {canteen_id, canteen, building, hours, menu}; CHECK на «Столовая №1» упал на прогоне — сервис приведён к контракту (имя столовой в поле «canteen»).
2. **Ловушка имён Result** (RoomsService.cpp:19 / LibraryService.cpp) — учтена сразу: тип строк в лямбдах пишется полностью, drogon::orm::Result; компиляция с первого прогона без этой ошибки.
3. Лимит состава: is_combo в JSON — bool (не 0/1), КБЖУ nullable —契约 5.13 «КБЖУ» с демо-водой без значений; «категории» отданы как есть (combo|main|soup|drink|bakery — словарь DDL 0006).

## Проверки (одноразовый контейнер drogonframework/drogon:v1.9.14, amd64 на arm64-хосте; сборка в /tmp-томе с libargon2-dev и libharu v2.4.6 с пином Р-12; docker compose НЕ запускался — внешний гейт)

- **Сборка:** make para-tests para security_tests → MAKE_EXIT=0, warnings 0 (grep -ci warning).
- **ctest — 100%:** para-tests Passed, security_tests Passed («100% tests passed, 0 tests failed out of 2»).
- **para-tests:** «test cases: 151 | 151 passed», «assertions: 3916 | 3916 passed» — **сходится:** 140/3791 (Э6б шаг 2) + 11 кейсов / 125 утверждений шага = 151/3916.
- **campus-only:** `-tc="*CanteensService*,*MapService*"` → 10/10 кейсов, 116/116 утверждений (11-й — fail-closed кейс с составным именем «Canteens/Map — …», в фильтр префикса не попадает; в полном прогоне зелёный).
- **Честные границы:** HTTP-E2E-цепочка curl на живом порту не гонялась — проверен уровень сервис+тесты (прецедент Э6б шаг 2); 401 без cookie закрывает LoginFilter, E2E — контурный прогон гейта. Контур должен быть пересеян (down -v) — фикстуры Э6б с маркером.
