#ifndef AUTOENDLSTREAM_H
#define AUTOENDLSTREAM_H

#include <fstream>

namespace Utils
{
    class AutoEndlStream {
    private:
        std::ofstream& os;  // Ссылка на поток вывода (например, файл)
    public:
        // Конструктор: принимает поток вывода по ссылке
        AutoEndlStream(std::ofstream& o) : os(o) {}

        template<typename T>
        AutoEndlStream& operator<<(const T& str) {
            os << str << std::endl;
            return *this;  // Возвращаем ссылку для цепочки: stream << "text" << "more";
        }
    };
}

#endif // AUTOENDLSTREAM_H
