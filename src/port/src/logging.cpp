#include "port.h"
#include "tinyfiledialogs.h"
#include <chrono>
#include <cstring>
#include <iostream>

namespace lcs {
#define F_BOLD "\033[1m"
#define F_UNDERLINE "\033[4m"
#define F_RED "\033[31m"
#define F_GREEN "\033[32m"
#define F_BLUE "\033[34m"
#define F_RESET "\033[0m"

constexpr int LINE_SIZE = 200;
static std::array<Line, LINE_SIZE> _buffer {};
// next item slot to write
static size_t _next = 0;
static size_t _size = 0;

static const auto app_start_time = std::chrono::steady_clock::now();
void Line::_set_time(void)
{
    using namespace std::chrono;
    uint64_t total
        = duration_cast<milliseconds>(steady_clock::now() - app_start_time)
              .count();
    uint32_t hour = static_cast<uint32_t>(total / 3'600'000ULL); // 60*60*1000
    uint32_t min  = static_cast<uint32_t>((total % 3'600'000ULL) / 60'000ULL);
    uint32_t sec  = static_cast<uint32_t>((total % 60'000ULL) / 1'000ULL);
    uint32_t ms   = static_cast<uint32_t>(total % 1'000ULL / 10);
    if (hour == 0) {
        std::snprintf(time_str.data(), time_str.max_size() - 1,
            "%02d:%02d:%02d", min, sec, ms);
    } else {
        std::snprintf(time_str.data(), time_str.max_size() - 1,
            "%02d:%02d:%02d", hour, min, sec);
    }
}

void l_iterate(std::function<void(size_t, const Line& l)> fn)
{
    if (_size < LINE_SIZE) {
        for (size_t i = 0; i < _size; i++) {
            fn(i, _buffer[i]);
        }
    } else {
        // if the buffer is full. The item we're about the write into is
        // the oldest
        size_t idx = (_next) % _size;
        while (idx != (_next - 1) % LINE_SIZE) {
            fn(idx, _buffer[idx]);
            idx = (idx + 1) % _size;
        }
    }
}

void Line::_fn_parse(std::string fnname)
{
    size_t class_begin  = fnname.find_first_of(' ') + 1;
    size_t fn_end       = fnname.find_first_of('(', class_begin);
    std::string fnrange = fnname.substr(class_begin, fn_end - class_begin);

    class_begin = fnrange.find_first_of("lcs::") != std::string::npos ? 5 : 0;
    size_t fn_begin  = fnrange.find_last_of("::") + 1;
    fn_end           = fnrange.size() - 1;
    size_t class_end = fnrange.find_first_of("::", class_begin);

    std::strncpy(fn.data(), fnrange.data() + fn_begin,
        std::min(fn.max_size() - 1, fn_end - fn_begin + 1));
    if (class_begin != fn_begin) {
        std::strncpy(obj.data(), fnrange.data() + class_begin,
            std::min(obj.max_size() - 1, class_end - class_begin));
    }
}

inline static void _log_pre(const Line& l)
{
    std::ostringstream oss2 {};
    printf(F_BLUE "[%s] " F_BOLD "%s%-6s" F_RESET F_GREEN "|" F_RESET
                  "%-15s" F_GREEN " |%-18s|" F_RESET F_BLUE
                  "%-25s" F_RESET F_GREEN "|" F_RESET "%s\r\n",
        l.time_str.begin(), (l.severity == ERROR ? F_RED : F_GREEN),
        l.log_level_str.begin(), l.file_line.begin(), l.obj.begin(),
        l.fn.begin(), l.expr.begin());
    if (is_testing) {
        __TEST_LOG__ << l.log_level_str.begin() << '\t' << l.file_line.begin()
                     << '\t' << l.obj.begin() << "\t" << l.fn.begin() << '\t'
                     << l.expr.begin() << std::endl;
    }
}

void l_push(Line&& line)
{
    // DEBUG logs are ignored on release
#ifdef NDEBUG
    if (LogLevel::DEBUG == line.severity) {
        return;
    }
#endif

    if (_size < LINE_SIZE) {
        _buffer[_next] = std::move(line);
        _log_pre(_buffer[_next]);
        _next++;
        _size++;
    } else {
        _next          = (_next + 1) % _size;
        _buffer[_next] = std::move(line);
        _log_pre(_buffer[_next]);
    }
}

void l_clear(void)
{
    _next = 0;
    _size = 0;
}

int __expect(std::function<bool(void)> expr, const char* function,
    const char* file, int line, const char* str_expr) noexcept
{
    static const char* _title = "Logic Circuit Simulator";
    std::stringstream s {};
    s << "ERROR " << file << " : " << line << "\t" << function
      << "(...) Assertion " << str_expr << " failed!" << std::endl;
    try {
        if (expr()) {
            return 0;
        }
        tinyfd_messageBox(_title, s.str().c_str(), "ok", "error", 1);

    } catch (const std::exception& ex) {
        tinyfd_messageBox(_title, ex.what(), "ok", "error", 0);
    } catch (const std::string& ex) {
        tinyfd_messageBox(_title, ex.c_str(), "ok", "error", 0);
    }

    l_push(Line { LogLevel::ERROR, file, line, function, "Assertion %s failed!",
        str_expr });
    return 1;
}

} // namespace lcs
