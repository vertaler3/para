-- 0005_rooms.sql — аудитории и брони (5.10).
-- Миграция Э1 (шаг 5): DDL перенесён из решения 02 §4.1 без изменений
-- (поля, порядок, FK, ON DELETE). Индексы 02 §4.2 в этой миграции:
--   idx_bookings_active — частичный UNIQUE: двойная бронь невозможна на
--     уровне БД (активные брони по аудитории и времени);
--   idx_bookings_user — «мои брони».
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5).
CREATE TABLE rooms (
  id        INTEGER PRIMARY KEY AUTOINCREMENT,
  name      TEXT NOT NULL,             -- «1207», «Б-204»
  building  TEXT NOT NULL,             -- корпус (вымышленный)
  capacity  INTEGER NOT NULL,
  features  TEXT NOT NULL DEFAULT ''   -- «проектор, доска, компьютеры»
);

CREATE TABLE room_bookings (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  room_id    INTEGER NOT NULL REFERENCES rooms(id),
  user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  starts_at  TEXT NOT NULL,            -- ISO 8601 UTC, слоты по 60 минут
  ends_at    TEXT NOT NULL,
  status     TEXT NOT NULL DEFAULT 'booked'
             CHECK (status IN ('booked','cancelled','blocked')),
  created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
CREATE UNIQUE INDEX idx_bookings_active     -- двойная бронь невозможна на уровне БД
  ON room_bookings(room_id, starts_at) WHERE status = 'booked';
CREATE INDEX idx_bookings_user ON room_bookings(user_id, starts_at);
