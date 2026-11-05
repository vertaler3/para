-- 0001_identity.sql — пользователи, роли, группы, профили.
-- Миграция Э1 (шаг 5): DDL перенесён из решения 02 §4.1 без изменений
-- (поля, порядок, FK, ON DELETE); собственных индексов в §4.2 для этой
-- миграции нет. Применяется миграционным раннером при старте приложения
-- (src/services/MigrationRunner, docs/decisions.md Р-5); файлы миграций
-- пишутся без BEGIN/COMMIT — границы транзакции ставит раннер.
PRAGMA foreign_keys = ON;

CREATE TABLE users (
  id              INTEGER PRIMARY KEY AUTOINCREMENT,
  login           TEXT    NOT NULL UNIQUE,
  password_hash   TEXT    NOT NULL,   -- только argon2id-encoded: $argon2id$v=19$m=19456,t=2,p=1$...
  role            TEXT    NOT NULL CHECK (role IN ('student','staff')),
  failed_attempts INTEGER NOT NULL DEFAULT 0,
  locked_until    TEXT,               -- нарастающая задержка при неудачных входах
  created_at      TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);

CREATE TABLE groups (
  id     INTEGER PRIMARY KEY AUTOINCREMENT,
  name   TEXT    NOT NULL UNIQUE,     -- '1-ФМО', '4-МЭО'
  course INTEGER NOT NULL
);

CREATE TABLE profiles (
  user_id         INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  fio             TEXT    NOT NULL,   -- вымышленные ФИО (требование задания, строка 44)
  faculty         TEXT    NOT NULL,
  program         TEXT    NOT NULL,
  course          INTEGER NOT NULL,
  group_id        INTEGER NOT NULL REFERENCES groups(id),
  student_card_no TEXT    NOT NULL UNIQUE  -- вымышленный номер зачётки
);
