#include <exception>
#include <iostream>
#include <memory>

#include <citygml/citygml.h>

#include "libplateau_c.h"
#include "city_model_c.h"

extern "C" {
    struct plateau_citygml_parser_params {
        int optimize;

        plateau_citygml_parser_params()
            : optimize(0) {
        }
    };

    LIBPLATEAU_C_EXPORT CityModelHandle* LIBPLATEAU_C_API plateau_load_citygml(const char* gml_path, const plateau_citygml_parser_params params) {
        API_TRY
            citygml::ParserParams parser_params;
            parser_params.optimize = params.optimize;
            return new CityModelHandle(citygml::load(gml_path, parser_params, nullptr));
        API_CATCH
            return nullptr;
    }
}
