#include "json_reader.h"
#include "request_handler.h"
#include <sstream>

using namespace transport_catalogue;
using namespace transport_catalogue::input_parser_reader;
using namespace transport_catalogue::renderer;

using namespace std;
int main() {

    TransportCatalogue tc;

    Document doc(Load(cin));
    MapRenderer map_rdr;
    ParseRenderSettings(doc, map_rdr);

    ParseBaseRequests(doc, tc);

    RequestHandler req_hndlr(tc, map_rdr);


    Print(HandleStatRequests(doc, req_hndlr), std::cout);
}