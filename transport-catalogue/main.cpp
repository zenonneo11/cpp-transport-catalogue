#include <sstream>
#include "input_reader.h"
#include "stat_reader.h"

using namespace transport_catalogue::input_parser_reader;
using namespace transport_catalogue::stat_reader_printer;
using namespace std;

int main() {

    TransportCatalogue tc;

    ParseQuerys(ReadQuerys(cin), tc);

    for (auto&& responce : ParseStatQuerys(ReadQuerys(cin), tc)) {
        switch (responce.r_type)            
        {
        case ResponseType::BUS_RESPONSE:
            PrintBusResponce(responce, std::cout);
            break;
        case ResponseType::STOP_RESPONSE:
            PrintStopResponce(responce, std::cout);
            break;
        }
    }       
}