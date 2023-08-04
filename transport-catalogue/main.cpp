#include "json_reader.h"
#include "request_handler.h"
#include "transport_router.h"
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
  
    ParseRoutingSettings(doc);

    TransportRouter tr(tc);

    RequestHandler req_hndlr( tc, map_rdr, tr );


    Print(HandleStatRequests(doc, req_hndlr), std::cout);
}