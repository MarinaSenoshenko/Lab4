#include "tuple.h"
#include "constants.h"

using namespace std;

template <class Dest> class LexicCast {
    Dest value;
public:
    LexicCast(const string& src) {
        stringstream str;
        str << src;
        str >> value;
        if (!str) {
            throw exception();
        }
    }
    operator const Dest& () {
        return value;
    }

    operator Dest& () {
        return value;
    }
};

template <> class LexicCast<string> {
    string value;
public:
    LexicCast(const string& obj) : value(obj) {}
    operator const string& () {
        return value;
    }
    operator string& () {
        return value;
    }
};

template<int index, typename Callback, typename... Args>
struct iterate {
    static void next(tuple<Args...>& t, Callback callback, vector<string>::iterator& it) {
        iterate<index - PREV_POS, Callback, Args...>::next(t, callback, it);
        callback(index - PREV_POS, get<index>(t), it);
    }
};

template<typename Callback, typename... Args>
struct iterate<ZERO_LEN, Callback, Args...> {
    static void next(tuple<Args...>& t, Callback callback, vector<string>::iterator& it) {
        callback(ZERO_LEN, get<ZERO_LEN>(t), it);
    }
};

template<typename Callback, typename... Args>
struct iterate<UNREAL_FILE_LEN, Callback, Args...> {
    static void next(tuple<Args...>& t, Callback callback, vector<string>::iterator& it) {}
};


template<typename Callback, typename... Args>
void ForEach(tuple<Args...>& t, Callback callback, vector<string>::iterator& it) {
    int const t_size = tuple_size<tuple<Args...>>::value;
    iterate<t_size - PREV_POS, Callback, Args...>::next(t, callback, it);
}

struct callback {
    template<typename T>
    void operator()(int index, T& t, vector<string>::iterator& it) {
        t = LexicCast<T>(*it);
        ++it;
    }
};

template <typename ...Args>
void Parse(tuple<Args...>& tuple, vector<string>::iterator& it) {
    ForEach(tuple, callback(), it);
}
