#include "input_reader.h"
#include "stat_reader.h"

using namespace transport_catalogue::input_parser_reader;
using namespace transport_catalogue::stat_reader_printer;
using namespace std;

int main() {

    TransportCatalogue tc;

    ParseQuerys(ReadQuerys(cin), tc);

    StatHandler(ReadQuerys(cin), tc);
}