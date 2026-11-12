-- 0008_language.sql — язык интерфейса пользователя (5.18).
-- Миграция фичи 5.18 (шаг 2 BE-дорожки): settings.lang — выбранный язык
-- интерфейса (ru — русский по умолчанию, инвариант фичи; en; zh — упрощённый
-- китайский). Значения — те же канонические коды, что CHECK здесь, словарь
-- src/models/i18n.h (langCode/langFromCode) и cookie para_lang (мини-роут
-- /lang): источник истины рендера — cookie (резолв para_lang →
-- Accept-Language → ru), settings.lang хранит выбор авторизованного
-- (PUT /v1/notifications/settings {lang}).
-- ALTER по стилю каталога: только добавление колонки, существующие строки
-- получают DEFAULT 'ru'; FK-фаза не меняется (новых ссылок нет) — гейт
-- PRAGMA foreign_key_check раннера проходит тривиально.
-- Применяется миграционным раннером при старте (docs/decisions.md Р-5):
-- файлы миграций пишутся без BEGIN/COMMIT — границы транзакции ставит
-- раннер; user_version дублём становится 8 (02 §8.3).
ALTER TABLE settings ADD COLUMN lang TEXT NOT NULL DEFAULT 'ru'
  CHECK (lang IN ('ru','en','zh'));
