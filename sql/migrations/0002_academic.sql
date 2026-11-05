-- 0002_academic.sql — дисциплины, преподаватели, расписание, оценки, сессия.
-- Миграция Э1 (шаг 5): DDL перенесён из решения 02 §4.1 без изменений
-- (поля, порядок, FK, ON DELETE). Индексы 02 §4.2 в этой миграции:
--   idx_schedule_group_time, idx_schedule_group_status — «следующая пара» (С1),
--     изменения недели (С3);
--   idx_grades_user — виджет «Учёба» (С4);
--   idx_session_plan_user — экран «Сессия».
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5).
CREATE TABLE disciplines (
  id           INTEGER PRIMARY KEY AUTOINCREMENT,
  code         TEXT NOT NULL UNIQUE,
  title        TEXT NOT NULL,
  control_type TEXT NOT NULL CHECK (control_type IN ('exam','credit','graded_credit'))
);

CREATE TABLE teachers (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  fio        TEXT NOT NULL,           -- вымышленные
  department TEXT
);

CREATE TABLE schedule_events (
  id             INTEGER PRIMARY KEY AUTOINCREMENT,
  group_id       INTEGER NOT NULL REFERENCES groups(id),
  discipline_id  INTEGER NOT NULL REFERENCES disciplines(id),
  teacher_id     INTEGER REFERENCES teachers(id),
  room           TEXT    NOT NULL,
  kind           TEXT    NOT NULL CHECK (kind IN ('lecture','seminar')),
  starts_at      TEXT    NOT NULL,
  ends_at        TEXT    NOT NULL,
  status         TEXT    NOT NULL DEFAULT 'normal'
                 CHECK (status IN ('normal','moved','cancelled','substituted')),
  prev_room      TEXT,               -- «было → стало»: сцена С1/С3
  prev_starts_at TEXT,
  prev_teacher_id INTEGER REFERENCES teachers(id),  -- замена преподавателя (С3): было → стало
  changed_by     INTEGER REFERENCES users(id),
  changed_at     TEXT
);
CREATE INDEX idx_schedule_group_time   ON schedule_events(group_id, starts_at);
CREATE INDEX idx_schedule_group_status ON schedule_events(group_id, status, starts_at);

CREATE TABLE grades (
  id            INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id       INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  discipline_id INTEGER NOT NULL REFERENCES disciplines(id),
  control_type  TEXT NOT NULL,
  component     TEXT NOT NULL,        -- 'модуль 1', 'семинары'…
  score         INTEGER NOT NULL,
  max_score     INTEGER NOT NULL,
  attempt       INTEGER NOT NULL DEFAULT 1,  -- пересдача: attempt ≥ 2 (отдельная ветка 4.3)
  comment       TEXT,
  graded_at     TEXT NOT NULL
);
CREATE INDEX idx_grades_user ON grades(user_id, discipline_id);

CREATE TABLE session_plan (
  id            INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id       INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  discipline_id INTEGER NOT NULL REFERENCES disciplines(id),
  kind          TEXT NOT NULL CHECK (kind IN ('consultation','exam','credit','retake')),
  starts_at     TEXT NOT NULL,
  room          TEXT NOT NULL
);
CREATE INDEX idx_session_plan_user ON session_plan(user_id, starts_at);
