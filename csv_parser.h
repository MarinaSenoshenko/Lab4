#include <fstream>
#include <typeinfo>
#include <regex>
#include "tuple.h"
#include "constants.h"

using namespace std;

template<class ... Args>

class CSVParser {
private:
    ifstream& input;
    size_t offset;
    int file_length = UNREAL_FILE_LEN;
    char field_delimiter = '\"', column_delimiter = ',', line_delimiter = '\n';

    enum class ParsingState {
        simple_read, read_delimiter
    };

    template<typename _CharT, typename _Traits, typename _Alloc>
    void GetLine(basic_istream<_CharT, _Traits>& is, basic_string<_CharT, _Traits, _Alloc>& str) {
        char symb;
        str.clear();
        while (is.get(symb)) {
            if (symb == line_delimiter) {
                break;
            }
            str.push_back(symb);
        }
    }

    bool EmptyTailToDelimiter(string str, int position) {
        for (int i = position; i < str.size(); ++i) {
            if (str[i] == column_delimiter) {
                break;
            }
            else {
                if (str[i] != ' ') {
                    return false;
                }
            }
        }
        return true;
    }

    string l_trim(const string& str) {
        return regex_replace(str, regex("^\\s+"), string(""));
    }

    string r_trim(const string& str) {
        return regex_replace(str, regex("\\s+$"), string(""));
    }

    string trim(const string& str) {
        return ltrim(rtrim(str));
    }


    int GetLength() {
        string line;
        if (file_length == UNREAL_FILE_LEN) {
            input.clear();
            input.seekg(ZERO_LEN, ios::beg);
            for (file_length = ZERO_LEN; Getline(input, line); file_length++);

            input.clear();
            input.seekg(ZERO_LEN, ios::beg);
        }
        return file_length;
    }

    class CSVIterator {
    private:
        string str_buf;
        ifstream& input;
        size_t index;
        CSVParser<Args...>& parent;
        bool last = false;


    public:
        CSVIterator(ifstream& ifs, size_t index, CSVParser<Args...>& parent) : index(index), parent(parent), input(ifs) {
            for (int i = ZERO_LEN; i < index - PREV_POS; i++, parent.GetLine(input, str_buf));

            parent.GetLine(input, str_buf);
            if (!input) {
                throw logic_error("Bad file!");
            }
        }

        CSVIterator operator++() {
            if (index < parent.file_length) {
                index++;
                input.clear();
                input.seekg(ZERO_LEN, ios::beg);
                for (int i = ZERO_LEN; i < index - PREV_POS; ++i, parent.GetLine(input, str_buf));

                parent.GetLine(input, str_buf);
            }
            else {
                str_buf = "";
                last = true;
            }

            return *this;
        }

        bool operator==(const CSVIterator& other) const {
            return this->last == other.last && this->index == other.index && this->str_buf == other.str_buf;
        }

        bool operator!=(const CSVIterator& other) {
            return !(*this == other);
        }

        tuple<Args...> operator*() {
            return parent.ParseLine(str_buf, index);
        }
    };

public:
    explicit CSVParser(ifstream& f, size_t offset) : input(f), offset(offset) {
        if (!f.is_open()) {
            throw std::invalid_argument("Can't open file");
        }
        if (offset >= GetLength()) {
            throw logic_error("Bad file offset! It is more than file len");
        }
        if (offset < BELOW_ZERO_OFFSET) {
            throw logic_error("Bad file offset! It is below zero");
        }
    }

    void setDelimiters(char new_field_delimiter, char new_column_delimiter, char new_line_delimiter) {
        line_delimiter = new_line_delimiter;
        field_delimiter = new_field_delimiter;
        column_delimiter = new_column_delimiter;
    }

    void reset() {
        input.clear();
        input.seekg(ZERO_LEN, ios::beg);
    }

    CSVIterator begin() {
        CSVIterator obj(input, offset + NEXT_POS, *this);
        return obj;
    }

    CSVIterator end() {
        int t = GetLength();

        CSVIterator csv_it(input, NEXT_POS, *this);
        csv_it.last = true;
        csv_it.str_buf = "";
        csv_it.index = GetLength();
        return csv_it;
    }

    void CheckSymb(bool delim, int line_position, ParsingState& state) {
        if (!delim && line.size() > line_position + NEXT_POS && line[line_position + NEXT_POS] != field_delimiter) {
            if (!EmptyTailToDelimiter(line, line_position + NEXT_POS)) {
                throw invalid_argument("Bad " + to_string(counter) + "th field at line " + to_string(line_num) + ": symbols after delimiter in field!");
            }
            state = ParsingState::simple_read;
        }
    }

    vector<string> read_string(string& line, int line_num) {
        vector<string> fields{ "" };
        ParsingState state = ParsingState::simple_read;
        int counter = line_position = BEGINING_SIZE;
        bool filled = false, access_write_delimiter = false;
        line = trim(line);
        for (char symb : line) {
            if (state == ParsingState::simple_read) {
                if (symb == column_delimiter) {
                    fields[counter] = trim(fields[counter]);
                    fields.emplace_back("");
                    counter++;
                    filled = false;
                }
                else if (symb == field_delimiter) {
                    if (line_position > ZERO_LEN && filled) {
                        throw invalid_argument("Bad " + to_string(counter) + "th field at line " + to_string(line_num) + ": field delimiter not first!");
                    }
                    fields[counter] = trim(fields[counter]);
                    state = ParsingState::read_delimiter;
                    access_write_delimiter = false;
                }
                else {
                    if (symb != ' ') {
                        filled = true;
                    }
                    fields[counter].push_back(c);
                }
            }
            else {
                if (symb == field_delimiter) {
                    CheckSymb(access_write_delimiter, line_position, state);
                    if (!access_write_delimiter && line_position == line.size() - PREV_POS) {
                        state = ParsingState::simple_read;
                    }
                    else if (!access_write_delimiter) {
                        access_write_delimiter = true;
                    }
                    else {
                        fields[counter].push_back(symb);
                        access_write_delimiter = false;
                    }
                }
                else {
                    fields[counter].push_back(symb);
                }
            }
            line_position++;
        }
        if (state != ParsingState::simple_read) {
            throw invalid_argument("Bad " + to_string(counter) + "th field at line " + to_string(line_num) + ": don't closed field!");
        }
        return fields;
    }

    tuple<Args...> ParseLine(string& line, int number){
        size_t size = sizeof...(Args);

        if (line.empty()) {
            throw invalid_argument("Line " + to_string(number) + " is empty!");
        }
        tuple<Args...> table_str;
        vector<string> fields = read_string(line, number);

        if (fields.size() != size) {
            throw invalid_argument("Wrong number of fields in line " + to_string(number) + "!");
        }
        auto iter = fields.begin();
        try {
            Parse(table_str, iter);
        }
        catch (exception & ex) {
            throw invalid_argument("Line " + to_string(number) + " contains bad types!");
        }
        return table_str;
    }
    friend class CSVParser;
};

