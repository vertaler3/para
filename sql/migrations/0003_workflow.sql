-- 0003_workflow.sql — заявки, таймлайн, дедлайны, объявления.
-- Миграция Э1 (шаг 5): DDL перенесён из решения 02 §4.1 без изменений
-- (поля, порядок, FK, ON DELETE). Индексы 02 §4.2 в этой миграции:
--   idx_applications_queue — очередь сотрудника (С8);
--   (idx_applications_user, idx_app_events, idx_deadlines_due — из DDL §4.1).
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5).
CREATE TABLE applications (
  id           INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id      INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  type         TEXT NOT NULL,        -- ключ из каталога application_types
  status       TEXT NOT NULL DEFAULT 'submitted'
               CHECK (status IN ('draft','submitted','accepted','in_progress',
                                 'need_info','ready','issued','rejected','cancelled')),
  payload_json TEXT NOT NULL DEFAULT '{}',
  created_at   TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now')),
  updated_at   TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
CREATE INDEX idx_applications_user ON applications(user_id, status);
CREATE INDEX idx_applications_queue ON applications(status, updated_at);  -- очередь сотрудника

CREATE TABLE application_events (     -- таймлайн заявки: подана → … → выдана
  id             INTEGER PRIMARY KEY AUTOINCREMENT,
  application_id INTEGER NOT NULL REFERENCES applications(id) ON DELETE CASCADE,
  from_status    TEXT,
  to_status      TEXT NOT NULL,
  comment        TEXT,
  author_id      INTEGER REFERENCES users(id),
  at             TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
CREATE INDEX idx_app_events ON application_events(application_id, at);

CREATE TABLE deadlines (
  id       INTEGER PRIMARY KEY AUTOINCREMENT,
  group_id INTEGER REFERENCES groups(id),  -- групповой дедлайн
  user_id  INTEGER REFERENCES users(id),   -- NULL/заполнено: персональный
  title    TEXT NOT NULL,
  kind     TEXT NOT NULL,                  -- 'work'|'credit'|'exam'|'consultation'|'retake'
  due_at   TEXT NOT NULL,
  reminded INTEGER NOT NULL DEFAULT 0      -- базовая рассылка «за 1 день» по дедлайну сделана
);

CREATE TABLE deadline_reminders (         -- персональное «Напомнить за день» (5.2)
  user_id     INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  deadline_id INTEGER NOT NULL REFERENCES deadlines(id) ON DELETE CASCADE,
  remind_at   TEXT NOT NULL,
  sent_at     TEXT,
  PRIMARY KEY (user_id, deadline_id)
);
CREATE INDEX idx_deadlines_due ON deadlines(due_at);

CREATE TABLE announcements (
  id           INTEGER PRIMARY KEY AUTOINCREMENT,
  author_id    INTEGER NOT NULL REFERENCES users(id),
  title        TEXT NOT NULL,
  body         TEXT NOT NULL,
  audience     TEXT NOT NULL DEFAULT 'all',  -- 'all' | JSON-список групп/курсов
  published_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
