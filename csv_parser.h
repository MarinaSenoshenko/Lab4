#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <fstream>
#include <typeinfo>
#include <regex>
#include "tuple.h"
#include "constants.h"


template<class ... Args>

class CSVParser {
private:
    std::ifstream& input;
    size_t offset;
    int file_length = UNREAL_FILE_LEN;
    char field_delimiter = '\"', column_delimiter = ',', line_delimiter = '\n';

    enum class ParsingState {
        simple_read, read_delimiter
    };

    template<typename _CharT, typename _Traits, typename _Alloc>
    void GetLine(std::basic_istream<_CharT, _Traits>& is, std::basic_string<_CharT, _Traits, _Alloc>& str) {
        char symb;
        str.clear();
        while (is.get(symb)) {
            if (symb == line_delimiter) {
                break;
            }
            str.push_back(symb);
        }
    }

    bool EmptyTailToDelimiter(std::string str, int position) {
        for (int i = position; i < str.size(); ++i) {
            if (str[i] == column_delimiter) {
                break;
            }
            else if (str[i] != ' ') {
                return false;
            }
        }
        return true;
    }

    std::string l_trim(const std::string& str) {
        return regex_replace(str, std::regex("^\\s+"), std::string(""));
    }

    std::string r_trim(const std::string& str) {
        return regex_replace(str, std::regex("\\s+$"), std::string(""));
    }

    std::string trim(const std::string& str) {
        return l_trim(r_trim(str));
    }


    int GetLength() {
        
        if (file_length == UNREAL_FILE_LEN) {
            input.clear();
            input.seekg(ZERO_LEN, std::ios::beg);

            std::string line;
            for (file_length = ZERO_LEN; getline(input, line); file_length++);

            input.clear();
            input.seekg(ZERO_LEN, std::ios::beg);
        }
        return file_length;
    }

    class CSVIterator {
    private:
        std::string str_buf;
        std::ifstream& input;
        size_t index;
        CSVParser<Args...>& parent;
        bool last = false;
        friend class CSVParser;

    public:
        CSVIterator(std::ifstream& ifs, size_t index, CSVParser<Args...>& parent) : index(index), parent(parent), input(ifs) {
            for (int i = ZERO_LEN; i < index - PREV_POS; i++, parent.GetLine(input, str_buf));

            parent.GetLine(input, str_buf);
            if (!input) {
                throw std::logic_error("Bad file!");
            }
        }

        CSVIterator operator++() {
            if (index < parent.file_length) {
                index++;
                input.clear();
                input.seekg(ZERO_LEN, std::ios::beg);
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

        std::tuple<Args...> operator*() {
            return parent.ParseLine(str_buf, index);
        }
    };

public:
    explicit CSVParser(std::ifstream& f, size_t offset) : input(f), offset(offset) {
        if (!f.is_open()) {
            throw std::invalid_argument("Can't open file");
        }
        if (offset >= GetLength()) {
            throw std::logic_error("Bad file offset! It is more than file len");
        }
        if (offset < BELOW_ZERO_OFFSET) {
            throw std::logic_error("Bad file offset! It is below zero");
        }
    }

    void SetDelimiters(char new_field_delimiter, char new_column_delimiter, char new_line_delimiter) {
        line_delimiter = new_line_delimiter;
        field_delimiter = new_field_delimiter;
        column_delimiter = new_column_delimiter;
    }

    void reset() {
        input.clear();
        input.seekg(ZERO_LEN, std::ios::beg);
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

    void CheckSymb(bool delim, int line_position, ParsingState& state, std::string& line, int line_num, int counter) {
        if (!delim && line.size() > line_position + NEXT_POS && line[line_position + NEXT_POS] != field_delimiter) {
            if (!EmptyTailToDelimiter(line, line_position + NEXT_POS)) {
                throw std::invalid_argument("Bad " + std::to_string(counter) + "th field at line " + std::to_string(line_num) + ": symbols after delimiter in field!");
            }
            state = ParsingState::simple_read;
        }
    }

    std::vector<std::string> read_string(std::string& line, int line_num) {
        std::vector<std::string> fields{ "" };
        ParsingState state = ParsingState::simple_read;
        int counter = BEGINING_SIZE;
        int line_position = BEGINING_SIZE;
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
                    if (line_position > ZERO_LEN&& filled) {
                        throw std::invalid_argument("Bad " + std::to_string(counter) + "th field at line " + std::to_string(line_num) + ": field delimiter not first!");
                    }
                    fields[counter] = trim(fields[counter]);
                    state = ParsingState::read_delimiter;
                    access_write_delimiter = false;
                }
                else {
                    if (symb != ' ') {
                        filled = true;
                    }
                    fields[counter].push_back(symb);
                }
            }
            else {
                if (symb == field_delimiter) {
                    CheckSymb(access_write_delimiter, line_position, state, line, counter, line_num);
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
            throw std::invalid_argument("Bad " + std::to_string(counter) + "th field at line " + std::to_string(line_num) + ": don't closed field!");
        }
        return fields;
    }


    std::tuple<Args...> ParseLine(std::string& line, int number) {
        size_t size = sizeof...(Args);

        if (line.empty()) {
            throw std::invalid_argument("Line " + std::to_string(number) + " is empty!");
        }
        std::tuple<Args...> table_str;
        std::vector<std::string> fields = read_string(line, number);

        if (fields.size() != size) {
            throw std::invalid_argument("Wrong number of fields in line " + std::to_string(number) + "!");
        }
        auto a = fields.begin();
        try {
            parse::Parse(table_str, a);
        }
        catch (std::exception & ex) {
            throw std::invalid_argument("Line " + std::to_string(number) + " contains bad types!");
        }

        return table_str;
    }
};

#endif


