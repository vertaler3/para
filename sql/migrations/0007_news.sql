-- 0007_news.sql — новости и события университета (5.17).
-- Миграция фичи 5.17 (шаг 1 BE-дорожки): DDL — дословно из постановки
-- фичи; индекс один — idx_news_published (лента по убыванию даты публикации
-- с фильтром категории). Сущность НЕ дублирует announcements (0003_workflow):
-- объявления — authored-записи сотрудников (author_id, публикация без push
-- и без уведомлений, 01 §4.5), news — редакционные новости/события с
-- категорией, местом и временем события. Внешних ключей нет: таблица не
-- ссылается на users/groups — адресация аудитории значениями полей
-- audience/audience_value (словарь в CHECK), поэтому FK-фаза Р-24 (Э8)
-- тривиальна: schema-фаза раннера идёт с выключенным enforcement, гейт
-- PRAGMA foreign_key_check на каждом старте эту таблицу не касается.
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5);
-- файлы миграций пишутся без BEGIN/COMMIT — границы транзакции ставит
-- раннер.
CREATE TABLE news (
  id             INTEGER PRIMARY KEY AUTOINCREMENT,
  category       TEXT NOT NULL CHECK (category IN ('official','conference','org','event')),
  title          TEXT NOT NULL,
  body           TEXT,                     -- расширенный текст (может быть NULL)
  event_at       TEXT,                     -- ISO 8601 UTC; NULL — новость без события
  location       TEXT,                     -- «Корпус Б, ауд. 2105» (связка с картой 5.16)
  audience       TEXT NOT NULL DEFAULT 'all' CHECK (audience IN ('all','group','course')),
  audience_value TEXT,                     -- id группы (1–3) или курса (1/4); NULL при 'all'
  published_at   TEXT NOT NULL             -- ISO 8601 UTC, лента по убыванию
);
CREATE INDEX idx_news_published ON news(published_at DESC, category);
