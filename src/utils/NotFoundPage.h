#pragma once

// Страница «не найдено» дизайна v3 (fe_rebuild, эталон design-drafts/v3/pages/404.html).
// Ставит кастомный обработчик ошибок Drogon (setCustomErrorHandler, HttpAppFramework.h
// v1.9.14): на 404 SSR-путей (всё вне /v1, /ws, /static) отдаёт view not_found на
// языке запроса (cookie para_lang → Accept-Language → ru; строки — словарь 5.18 до
// рендера, Р-16). Остальные коды и 404 API/статики уходят в прежний обработчик —
// API-пути и так ловит regex-роут JSON 02 §5.1 (main.cpp). Сессию не читает: на
// странице нет данных пользователя. Тот же вызов делает security_tests (сервер теста
// конфигурационно идентичен продовому).

namespace para
{
void registerNotFoundPage();
}  // namespace para
