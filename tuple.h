#include <tuple>
#include <functional>
#include <iostream>
#include <sstream>
#include "constants.h"

template<int index, typename Callback, typename... Args>
struct iterate {
    static void next(std::tuple<Args...>& t, Callback callback, std::ostream& stream) {
        iterate<index - PREV_POS, Callback, Args...>::next(t, callback, stream);
        callback(index, std::get<index>(t), stream);
    }
};

template<typename Callback, typename... Args>
struct iterate<ZERO_LEN, Callback, Args...> {
    static void next(std::tuple<Args...>& t, Callback callback, std::ostream& stream) {
        callback(ZERO_LEN, std::get<ZERO_LEN>(t), stream);
    }
};

template<typename Callback, typename... Args>
struct iterate<UNREAL_FILE_LEN, Callback, Args...> {
    static void next(std::tuple<Args...>& t, Callback callback, std::ostream& stream) {}
};

template<typename Callback, typename... Args>
void forEach(std::tuple<Args...>& t, Callback callback, std::ostream& stream) {
    int const t_size = std::tuple_size<std::tuple<Args...>>::value;
    iterate<t_size - PREV_POS, Callback, Args...>::next(t, callback, stream);
}

struct callback {
    template<typename T>
    void operator()(int index, T&& t, std::ostream& stream) {
        stream << t << " ";
    }
};

template<typename _CharT, typename _Traits, typename... Args>
inline std::basic_ostream<_CharT, _Traits>& operator<<(std::basic_ostream<_CharT, _Traits>& stream, std::tuple<Args...>& t) {
    forEach(t, callback(), stream);
    return stream;
}

namespace parse {
    template <class Dest> class LexicCast {
        Dest value;
    public:
        LexicCast(const std::string& src) {
            std::stringstream str;
            str << src;
            str >> value;
            if (!str) {
                throw std::exception();
            }
        }

        operator const Dest& () const {
            return value;
        }

        operator Dest& () {
            return value;
        }
    };

    template <> class LexicCast<std::string> {
        std::string value;
    public:
        LexicCast(const std::string& obj) : value(obj) {}

        operator const std::string& () {
            return value;
        }

        operator std::string& () {
            return value;
        }
    };

    template<int index, typename Callback, typename... Args>
    struct iterate {
        static void next(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it) {
            iterate<index - PREV_POS, Callback, Args...>::next(t, callback, it);
            callback(index - PREV_POS, std::get<index>(t), it);
        }
    };

    template<typename Callback, typename... Args>
    struct iterate<ZERO_LEN, Callback, Args...> {
        static void next(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it) {
            callback(ZERO_LEN, std::get<ZERO_LEN>(t), it);
        }
    };

    template<typename Callback, typename... Args>
    struct iterate<UNREAL_FILE_LEN, Callback, Args...> {
        static void next(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it) {}
    };


    template<typename Callback, typename... Args>
    void ForEach(std::tuple<Args...>& t, Callback callback, std::vector<std::string>::iterator& it) {
        int const t_size = std::tuple_size<std::tuple<Args...>>::value;
        iterate<t_size - PREV_POS, Callback, Args...>::next(t, callback, it);
    }

    struct callback {
        template<typename T>
        void operator()(int index, T& t, std::vector<std::string>::iterator& it) {
            t = LexicCast<T>(*it);
            ++it;
        }
    };

    template <typename ...Args>
    void Parse(std::tuple<Args...>& tuple, std::vector<std::string>::iterator& it) {
        ForEach(tuple, callback(), it);
    }
}

