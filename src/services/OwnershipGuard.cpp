#include "services/OwnershipGuard.h"

#include <string_view>

#include <drogon/drogon.h>

using namespace drogon;
using namespace drogon::orm;

namespace para {
namespace {

// Белый список enum Table → фиксированные SQL-литералы (02 §6.4: в запрос
// подставляется только параметр $1, конкатенации строк нет). Добавление
// таблицы — новый case здесь и новое значение enum в заголовке.
// string_view: DbClient::operator<< для const char* неоднозначен (три
// перегрузки: string/string&&/string_view), string_view биндится точно.
std::string_view sqlFor(OwnershipGuard::Table table)
{
    switch (table)
    {
        case OwnershipGuard::Table::Applications:
            return "SELECT user_id FROM applications WHERE id = $1";
        case OwnershipGuard::Table::Grades:
            return "SELECT user_id FROM grades WHERE id = $1";
        case OwnershipGuard::Table::Notifications:
            return "SELECT user_id FROM notifications WHERE id = $1";
    }
    return {};  // недостижимо при полном switch над enum
}

// Человекочитаемое имя таблицы для лога (не попадает в SQL).
const char* labelFor(OwnershipGuard::Table table)
{
    switch (table)
    {
        case OwnershipGuard::Table::Applications:
            return "applications";
        case OwnershipGuard::Table::Grades:
            return "grades";
        case OwnershipGuard::Table::Notifications:
            return "notifications";
    }
    return "?";  // недостижимо при полном switch над enum
}

}  // namespace

void OwnershipGuard::check(drogon::orm::DbClientPtr db,
                           Table table,
                           int64_t sessionUserId,
                           int64_t objectId,
                           std::function<void(Verdict)> cb)
{
    // cb копируется в ОБЕ ветки (result/exception): стреляет ровно одна,
    // но при `cb = std::move(cb)` в первой лямбде вторая получила бы пустой
    // std::function (moved-from гарантированно пуст) → std::bad_function_call
    // в event loop на первом же сбое БД. Проверено юнит-тестом Error-ветки.
    *db << sqlFor(table) << objectId >>
        [cb, sessionUserId](const Result& rows) {
            if (rows.empty())
            {
                // Несуществующий id → 404 (02 §6.3): не раскрываем чужие
                // объекты и не маскируем их под forbidden.
                cb(Verdict::NotFound);
                return;
            }
            const int64_t owner = rows[0]["user_id"].as<int64_t>();
            cb(owner == sessionUserId ? Verdict::Own : Verdict::Forbidden);
        } >>
        [cb, table, objectId](const DrogonDbException& e) {
            // Fail-closed (02 §6.3: чужое не отдаётся никогда): ошибка БД —
            // не «разрешено»; контроллер отвечает 500, детали — только в
            // серверном логе (02 §5.1, §6.5).
            LOG_ERROR << "OwnershipGuard::check(" << labelFor(table)
                      << ", id=" << objectId << "): " << e.base().what();
            cb(Verdict::Error);
        };
}

}  // namespace para
