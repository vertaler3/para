-- 0004_notifications_and_prefs.sql — уведомления, подписки, настройки, ICS, сессии.
-- Миграция Э1 (шаг 5): DDL перенесён из решения 02 §4.1 без изменений
-- (поля, порядок, FK, ON DELETE). Индексы 02 §4.2 в этой миграции:
--   idx_notifications_user — бейдж непрочитанных;
--   idx_sessions_expiry — лимитер входа (очистка истёкших).
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5).
CREATE TABLE notifications (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  type       TEXT NOT NULL CHECK (type IN ('schedule','deadline','application','announcement')),
  title      TEXT NOT NULL,
  body       TEXT,
  deep_link  TEXT,                     -- тап → карточка переноса / таймлайн заявки (4.5)
  read_at    TEXT,
  created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
CREATE INDEX idx_notifications_user ON notifications(user_id, read_at, created_at);

CREATE TABLE push_subscriptions (
  id         INTEGER PRIMARY KEY AUTOINCREMENT,
  user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  endpoint   TEXT NOT NULL UNIQUE,
  keys_json  TEXT NOT NULL,
  created_at TEXT NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);

CREATE TABLE ics_tokens (
  user_id    INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  token      TEXT NOT NULL UNIQUE,     -- приватная ссылка подписки (5.4)
  created_at TEXT NOT NULL,
  revoked    INTEGER NOT NULL DEFAULT 0
);

CREATE TABLE dashboard_settings (
  user_id           INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  widget_order_json TEXT NOT NULL DEFAULT '[]',
  hidden_json       TEXT NOT NULL DEFAULT '[]'
);

CREATE TABLE settings (
  user_id             INTEGER PRIMARY KEY REFERENCES users(id) ON DELETE CASCADE,
  notify_schedule     INTEGER NOT NULL DEFAULT 1,   -- три тумблера 4.5
  notify_deadlines    INTEGER NOT NULL DEFAULT 1,
  notify_applications INTEGER NOT NULL DEFAULT 1,
  quiet_hours_json    TEXT NOT NULL DEFAULT '{"from":"23:00","to":"07:00"}',
  theme               TEXT NOT NULL DEFAULT 'system',
  onboarding_json     TEXT NOT NULL DEFAULT '[]'   -- закрытые шаги чеклиста онбординга С5 (4 шага)
);

CREATE TABLE sessions (               -- персистентность сессий (риск 6, раздел 6.1)
  id         TEXT PRIMARY KEY,        -- случайный 128-бит токен, в cookie — только он
  user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  created_at TEXT NOT NULL,
  expires_at TEXT NOT NULL
);
CREATE INDEX idx_sessions_expiry ON sessions(expires_at);
