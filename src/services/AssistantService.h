#pragma once

#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>

#include <drogon/orm/DbClient.h>
#include <json/json.h>

#include "services/SessionService.h"

namespace trantor {
class EventLoop;
}

namespace para {

// ============================================================================
// AI-ассистент — идея 5.19 (docs/decisions.md Р-36, таблица
// assistant_messages миграции 0009):
//
//   POST /v1/assistant/chat     {message}  → 200 {reply} · 400 · 429 · 502
//   GET  /v1/assistant/history             → 200 {messages:[…]} (свои 20)
//
// Модель — внешний OpenAI-совместимый endpoint (YandexGPT). ВСЕ настройки —
// только переменные окружения сервера (AGENTS §5.2, 02 §6.5): ключ, URL,
// модель и системный промт в коде и репозитории не живут, в лог не пишутся.
//
// Изоляция (02 §6.3 п.2): контекст запроса собирается ТОЛЬКО из данных
// пользователя сессии — его группа (расписание сегодня/завтра), его план
// сессии, его заявки, дедлайны его группы и личные, новости видимой ему
// аудитории; история — WHERE user_id = сессия. Чужих строк в промте нет по
// построению — path-id у роутов нет, OwnershipGuard не применим.
//
// fail-closed: ошибка БД при сборке контекста → Error (500), частичный
// контекст модели не уходит (иначе она «честно» ответит «пар нет»). Ошибка
// LLM (сеть, таймаут 20 с, не-200, пустой ответ, не настроено) →
// Unavailable (502 assistant_unavailable). Тело ответа апстрима не
// логируется и клиенту не отдаётся: в нём служебные id облака.
//
// История пишется ПАРОЙ (вопрос + ответ) одним INSERT и только при успехе
// LLM — висячих вопросов без ответа в ленте нет.
// ============================================================================
class AssistantService
{
  public:
    static constexpr std::size_t kMaxMessageChars = 700;  // кодпоинты UTF-8
    static constexpr int kHistoryForPrompt = 10;          // в промт
    static constexpr int kHistoryForClient = 20;          // GET history
    static constexpr int kMaxTokens = 500;
    static constexpr double kTimeoutSec = 20.0;

    // --- Настройки из окружения ------------------------------------------
    struct Config
    {
        std::string url;           // ASSISTANT_API_URL
        std::string key;           // ASSISTANT_API_KEY
        std::string model;         // ASSISTANT_MODEL (уже разрешённый URI)
        std::string systemPrompt;  // ASSISTANT_SYSTEM_PROMPT
        bool configured() const
        {
            return !url.empty() && !key.empty() && !model.empty() &&
                   !systemPrompt.empty();
        }
    };
    static Config configFromEnv();

    // Значение из .env может прийти «сырым» (docker run --env-file не
    // разбирает кавычки): "…" снимаются, \n \" \\ \t раскрываются.
    // Значение без внешних кавычек возвращается как есть.
    static std::string unquoteEnv(const std::string& raw);

    // YandexGPT ждёт URI gpt://<каталог>/<модель>/latest. Полный URI
    // (с «://») — как есть; короткое имя + каталог → URI; каталога нет —
    // имя как есть (для иных OpenAI-совместимых провайдеров).
    static std::string resolveModel(const std::string& model,
                                    const std::string& folderId);

    // --- Валидация входа ---------------------------------------------------
    static std::size_t utf8Length(const std::string& s);
    enum class Input
    {
        Ok,
        Empty,    // нет поля / не строка / одни пробелы → 400
        TooLong,  // > kMaxMessageChars кодпоинтов → 400
    };
    // body — JSON тела (nullptr — не JSON). out — обрезанный текст.
    static Input validate(const Json::Value* body, std::string& out);

    // --- Лимит частоты на пользователя (в памяти процесса) ---------------
    class RateLimiter
    {
      public:
        static constexpr int kMax = 10;
        static constexpr int64_t kWindowSec = 60;
        // 0 — попытка учтена и разрешена; >0 — отказ, секунды до слота.
        int64_t acquire(int64_t userId, int64_t nowSec);
        static RateLimiter& instance();

      private:
        std::mutex mu_;
        std::unordered_map<int64_t, std::deque<int64_t>> hits_;
    };

    // --- Контекст и запрос к модели (чистые функции — юнит-тесты) --------
    // ISO «…Z» → «HH:MM» по Москве (UTC+3, без перехода на летнее время).
    static std::string mskTime(const std::string& iso);
    // ISO «…Z» → «ДД.ММ HH:MM» по Москве.
    static std::string mskDateTime(const std::string& iso);

    struct ContextData
    {
        std::string today;      // YYYY-MM-DD (UTC проекта)
        Json::Value events;     // события группы сегодня+завтра (eventJson)
        Json::Value exams;      // план сессии (GradesService::exams)
        Json::Value deadlines;  // [{title, kind, due_at}]
        Json::Value applications;  // активные заявки (appJson)
        Json::Value campus;     // будущие события ленты news
    };
    static std::string renderContext(const ContextData& d);

    static Json::Value buildRequest(const Config& cfg,
                                    const std::string& contextText,
                                    const Json::Value& history,
                                    const std::string& message);

    // Ответ модели из тела 200: choices[0].message.content (непустой).
    static std::optional<std::string> parseReply(int httpStatus,
                                                 const std::string& body);

    // --- Транспорт к LLM -------------------------------------------------
    // reply — nullopt при любой ошибке (сеть/таймаут/статус/формат).
    using ReplyCb = std::function<void(std::optional<std::string> reply)>;
    using Transport =
        std::function<void(const Config&, const Json::Value& payload,
                           ReplyCb)>;
    // Реальный HTTP(S) через drogon::HttpClient. loop == nullptr — главный
    // цикл приложения (прод); тесты передают свой EventLoopThread.
    static Transport httpTransport(trantor::EventLoop* loop = nullptr);

    // --- Операции сервиса ------------------------------------------------
    enum class Status
    {
        Ok,           // 200
        Error,        // 500 — БД (fail-closed)
        Unavailable,  // 502 — LLM недоступен / не настроен
    };
    struct ChatResult
    {
        Status status = Status::Error;
        std::string reply;
    };
    using ChatCb = std::function<void(ChatResult)>;

    // Собрать контекст только этой сессии (тестам — отдельно от chat).
    using ContextCb = std::function<void(bool ok, ContextData)>;
    static void collectContext(drogon::orm::DbClientPtr db,
                               const SessionInfo& session,
                               const std::string& today,
                               ContextCb cb);

    static void chat(drogon::orm::DbClientPtr db,
                     const SessionInfo& session,
                     const std::string& message,
                     const Config& cfg,
                     Transport transport,
                     ChatCb cb);

    // Последние limit своих сообщений, по возрастанию id.
    using HistoryCb = std::function<void(bool ok, Json::Value messages)>;
    static void history(drogon::orm::DbClientPtr db,
                        int64_t userId,
                        int limit,
                        HistoryCb cb);
};

}  // namespace para
