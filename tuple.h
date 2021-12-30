#include <tuple>
#include <functional>
#include <iostream>
#include <vector>
#include <sstream>
#include "constants.h"

using namespace std;

struct callback {
    template<typename T>
    void operator()(int index, T&& t, ostream& stream) {
        stream << t << " ";
    }
};

template<int index, typename CallBack, typename... Args> 
struct iterate {
    static void next(tuple<Args...>& t, CallBack callback, ostream& stream) {
        iterate<index - PREV_POS, Callback, Args...>::next(t, callback, stream);
        callback(index, get<index>(t), stream);
    }
};

template<typename CallBack, typename... Args> 
struct iterate<ZERO_LEN, CallBack, Args...> {
    static void next(tuple<Args...>& t, CallBack callback, ostream& stream) {
        callback(ZERO_LEN, get<ZERO_LEN>(t), stream);
    }

};

template<typename CallBack, typename... Args> 
struct iterate<UNREAL_FILE_LEN, CallBack, Args...> {
    static void next(tuple<Args...>& t, CallBack callback, ostream& stream) {}
};

template<typename CallBack, typename... Args> 
void forEach(tuple<Args...>& t, CallBack callback, ostream& stream) {
    int const t_size = tuple_size<tuple<Args...>>::value;
    iterate<t_size - PREV_POS, Callback, Args...>::next(t, callback, stream);
}



template<typename _CharT, typename _Traits, typename... Args>
inline basic_ostream<_CharT, _Traits>& operator<<(basic_ostream<_CharT, _Traits>& stream, tuple<Args...>& t) {
    forEach(t, callback(), stream);
    return stream;
}

