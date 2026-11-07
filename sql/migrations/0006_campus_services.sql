-- 0006_campus_services.sql — профили людей, библиотека, столовая, карта (5.11–5.16).
-- Миграция Э6а (шаг 1): DDL перенесён из решения 02 §4.1 (блок 0006) без
-- изменений, КРОМЕ одной строки — см. «Отступление от спеки» ниже.
-- Индексы 02 §4.2 в этой миграции:
--   idx_library_items_branch, idx_menu_items_canteen, idx_rooms_building —
--   каталоги библиотеки/столовой/карты (Э6б).
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5);
-- файлы миграций пишутся без BEGIN/COMMIT — границы транзакции ставит раннер.
--
-- ОТСТУПЛЕНИЕ ОТ СПЕКИ (docs/decisions.md Р-13): строка 02 §4.1
--   ALTER TABLE schedule_events ADD COLUMN teacher_id INTEGER REFERENCES teacher_profiles(id);
-- НЕ выполняется — колонка schedule_events.teacher_id существует с миграции
-- 0002_academic.sql (FK REFERENCES teachers(id); teachers создана там же,
-- 0002 строки 16–20) и заполнена фикстурами Э1 (события 1–10 → teacher_id
-- 1–5). Повторный ALTER упал бы на старте: «duplicate column name:
-- teacher_id». Разрешение расхождения: teacher_profiles (0006) — расширенный
-- публичный профиль (должность/почта/кабинет), id фикстур выровнены 1:1 с
-- teachers, так что существующая колонка валидна для обоих чтений;
-- enforcement FK в рантайме выключен (находка ревью Э1), но данные
-- согласованы и с включённым FK. Контроллеры библиотеки/столовой/карты —
-- Э6б (здесь только схема).

CREATE TABLE teacher_profiles (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  fio        TEXT NOT NULL,
  title      TEXT,                     -- «доцент», «старший преподаватель»
  department TEXT,                     -- кафедра (вымышленная)
  email      TEXT NOT NULL UNIQUE,     -- @example.edu — вымышленный вузовский домен
  office     TEXT                      -- «корпус Б, ауд. 2105» (связка с картой)
);

CREATE TABLE buildings (               -- карта корпусов (5.16)
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  name      TEXT NOT NULL,             -- «Корпус Б»
  address   TEXT NOT NULL
);

ALTER TABLE rooms ADD COLUMN building_id INTEGER REFERENCES buildings(id);
ALTER TABLE rooms ADD COLUMN floor     INTEGER;   -- этаж
ALTER TABLE rooms ADD COLUMN kind      TEXT;      -- аудитория | лаборатория | деканат

CREATE TABLE library_branches (
  id          INTEGER PRIMARY KEY AUTOINCREMENT,
  name        TEXT NOT NULL,           -- «Главная библиотека», «Библиотека ФМО»
  building_id INTEGER NOT NULL REFERENCES buildings(id)
);
CREATE TABLE library_items (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  title      TEXT NOT NULL,
  author     TEXT,
  kind       TEXT NOT NULL DEFAULT 'book',  -- book | journal | textbook
  branch_id  INTEGER NOT NULL REFERENCES library_branches(id),
  hall       TEXT,                          -- зал/стеллаж (демо)
  total      INTEGER NOT NULL DEFAULT 1,
  available  INTEGER NOT NULL DEFAULT 1
);
CREATE TABLE library_cards (
  user_id  INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  card_no  TEXT NOT NULL UNIQUE            -- вымышленный номер читательского
);

CREATE TABLE canteens (
  id          INTEGER PRIMARY KEY AUTOINCREMENT,
  name        TEXT NOT NULL,             -- «Столовая №1», «Кофе-пойнт ФМО»
  building_id INTEGER NOT NULL REFERENCES buildings(id),
  hours       TEXT NOT NULL DEFAULT '09:00–17:00'
);
CREATE TABLE menu_items (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  canteen_id INTEGER NOT NULL REFERENCES canteens(id),
  name       TEXT NOT NULL,
  category   TEXT NOT NULL,              -- combo | main | soup | drink | bakery
  is_combo   INTEGER NOT NULL DEFAULT 0,
  price      INTEGER NOT NULL,           -- ₽
  kcal       INTEGER, protein REAL, fat REAL, carbs REAL
);

CREATE INDEX idx_library_items_branch ON library_items(branch_id, available);
CREATE INDEX idx_menu_items_canteen   ON menu_items(canteen_id, category);
CREATE INDEX idx_rooms_building       ON rooms(building_id, floor);
