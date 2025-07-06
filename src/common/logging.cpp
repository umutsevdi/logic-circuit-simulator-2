#include "common.h"
#include "tinyfiledialogs.h"

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

inline static void _log_pre(const Line& l)
{
    std::ostringstream oss2 {};

    printf(F_BOLD "%s%-6s" F_RESET F_GREEN " | " F_RESET "%-35s" F_GREEN
                  " | " F_RESET F_BLUE "%-35s" F_RESET F_GREEN " | " F_RESET
                  "%s\r\n",
        (l.severity == ERROR ? F_RED : F_GREEN), l.log_level_str.begin(),
        l.file_line.begin(), l.fn.begin(), l.expr.begin());
    if (is_testing) {
        __TEST_LOG__ << l.log_level_str.begin() << '\t' << l.file_line.begin()
                     << '\t' << l.fn.begin() << '\t' << l.expr.begin()
                     << std::endl;
    }
}

void l_push(Line&& line)
{
    // DEBUG logs are ignored on release
#ifdef NDEBUG
    if constexpr (LogLevel::DEBUG == line.severity) {
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

    l_push(Line { LogLevel::ERROR, file, line, function, 0,
        "Assertion %s failed!", str_expr });
    return 1;
}

} // namespace lcs
