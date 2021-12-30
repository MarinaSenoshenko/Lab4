#include "csv_parser.h"
#include "constants.h"

using namespace std;

int main(int argc, char** argv)
{
    ifstream file("test.csv");

    CSVParser<string, double> parser(file, ZERO_LEN);
    parser.setDelimiters('\"', ';', '/');

    for (tuple<string, double> rs : parser) {
        cout << rs << endl;
    }
    return EXIT;
}
