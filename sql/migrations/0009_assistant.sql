-- 0009_assistant.sql — история чата AI-ассистента (идея 5.19, Р-36).
-- Одна строка — одна реплика: role 'user' (вопрос студента) или
-- 'assistant' (ответ модели). Пару пишет AssistantService одной
-- транзакцией и только при успешном ответе LLM — «висячих» вопросов без
-- ответа в истории нет. Владелец — user_id; выборки только по user_id
-- сессии (02 §6.3 п.2), удаление пользователя чистит историю каскадом.
-- Индекс (user_id, id) — «последние N своих» без full scan.
-- Применяется миграционным раннером при старте (Р-5): без BEGIN/COMMIT,
-- user_version дублём становится 9 (02 §8.3).
CREATE TABLE assistant_messages (
  id         INTEGER PRIMARY KEY,
  user_id    INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
  role       TEXT    NOT NULL CHECK (role IN ('user','assistant')),
  content    TEXT    NOT NULL,
  created_at TEXT    NOT NULL DEFAULT (strftime('%Y-%m-%dT%H:%M:%SZ','now'))
);
CREATE INDEX idx_assistant_user ON assistant_messages(user_id, id);
