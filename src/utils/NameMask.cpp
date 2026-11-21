#include "utils/NameMask.h"

namespace para
{
namespace
{

// Длина первого UTF-8 символа по старшему байту (RFC 3629): continuation-
// байты 10xxxxxx не бывают первыми, 11110xxx — 4 байта максимум. Не-UTF8
// мусор с ведущим единичным байтом трактуем как 1 байт — не падаем.
std::size_t utf8CharLen(unsigned char lead)
{
    if (lead < 0x80)
        return 1;  // 0xxxxxxx — ASCII
    if ((lead & 0xE0) == 0xC0)
        return 2;  // 110xxxxx
    if ((lead & 0xF0) == 0xE0)
        return 3;  // 1110xxxx
    if ((lead & 0xF8) == 0xF0)
        return 4;  // 11110xxx
    return 1;
}

}  // namespace

std::string maskFullName(const std::string& fio)
{
    // Первое слово (фамилия) и второй символ для инициала имени.
    const std::size_t firstSpace = fio.find(' ');
    if (firstSpace == std::string::npos || firstSpace == 0)
        return fio;  // одно слово / пустая строка — маскировать нечего

    std::size_t nameStart = firstSpace + 1;
    while (nameStart < fio.size() && fio[nameStart] == ' ')
        ++nameStart;  // двойные пробелы — не разрезаем инициал
    if (nameStart >= fio.size())
        return fio.substr(0, firstSpace);  // хвост из пробелов

    const std::size_t initialLen =
        utf8CharLen(static_cast<unsigned char>(fio[nameStart]));
    return fio.substr(nameStart, initialLen) + ". " +
           fio.substr(0, firstSpace);
}

std::string maskRatingName(const std::string& fio)
{
    // Инициал имени (первый не-пробельный символ после первого пробела,
    // как в maskFullName); фамилия не отдаётся вовсе — вместо неё
    // фиксированные «****» (длину не выдаём). Одно слово — инициал его
    // первого символа; одни пробелы/пустая строка — пустая строка.
    const std::size_t firstSpace = fio.find(' ');
    const std::size_t searchFrom =
        firstSpace == std::string::npos ? 0 : firstSpace + 1;
    const std::size_t initialStart = fio.find_first_not_of(' ', searchFrom);
    if (initialStart == std::string::npos)
        return std::string();
    const std::size_t initialLen =
        utf8CharLen(static_cast<unsigned char>(fio[initialStart]));
    return fio.substr(initialStart, initialLen) + "****";
}

}  // namespace para
