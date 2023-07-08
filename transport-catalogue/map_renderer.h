#pragma once

#include "geo.h"
#include "svg.h"
#include "json.h"
#include "domain.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>
#include <array>
#include <set>

namespace transport_catalogue {

    namespace renderer {

        inline const double EPSILON = 1e-6;

        inline bool IsZero(double value) {
            return std::abs(value) < transport_catalogue::renderer::EPSILON;
        }

        class SphereProjector {
        public:
            // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
            template <typename PointInputIt>
            SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                double max_width, double max_height, double padding)
                : padding_(padding) 
            {
                // Если точки поверхности сферы не заданы, вычислять нечего
                if (points_begin == points_end) {
                    return;
                }

                // Находим точки с минимальной и максимальной долготой
                const auto [left_it, right_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
                min_lon_ = left_it->lng;
                const double max_lon = right_it->lng;

                // Находим точки с минимальной и максимальной широтой
                const auto [bottom_it, top_it] = std::minmax_element(
                    points_begin, points_end,
                    [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
                const double min_lat = bottom_it->lat;
                max_lat_ = top_it->lat;

                // Вычисляем коэффициент масштабирования вдоль координаты x
                std::optional<double> width_zoom;
                if (!IsZero(max_lon - min_lon_)) {
                    width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
                }

                // Вычисляем коэффициент масштабирования вдоль координаты y
                std::optional<double> height_zoom;
                if (!IsZero(max_lat_ - min_lat)) {
                    height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
                }

                if (width_zoom && height_zoom) {
                    // Коэффициенты масштабирования по ширине и высоте ненулевые,
                    // берём минимальный из них
                    zoom_coeff_ = std::min(*width_zoom, *height_zoom);
                }
                else if (width_zoom) {
                    // Коэффициент масштабирования по ширине ненулевой, используем его
                    zoom_coeff_ = *width_zoom;
                }
                else if (height_zoom) {
                    // Коэффициент масштабирования по высоте ненулевой, используем его
                    zoom_coeff_ = *height_zoom;
                }
            }



            // Проецирует широту и долготу в координаты внутри SVG-изображения
            svg::Point operator()(transport_catalogue::geo::Coordinates coords) const {
                return {
                    (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                    (max_lat_ - coords.lat) * zoom_coeff_ + padding_
                };
            }

        private:
            double padding_;
            double min_lon_ = 0;
            double max_lat_ = 0;
            double zoom_coeff_ = 0;
        };


        struct MapRenderer {

            void RenderLines(svg::Document& map, std::vector<std::vector<const Stop*>> buses_stops, const SphereProjector& proj) const {
                for (const auto& stops : buses_stops) {
                    svg::Polyline bus_line;

                    for (const auto& stop : stops) {
                        bus_line.AddPoint(proj(stop->coordinates));
                    }
                    bus_line.SetStrokeColor(GetColor(map.Size())).
                        SetFillColor("none").SetStrokeWidth(props.line_width).
                        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND).
                        SetStrokeLineCap(svg::StrokeLineCap::ROUND);
                    map.Add(bus_line);
                }

            }

            void RenderBusNames(svg::Document& map, std::vector<const Bus*> all_buses, const SphereProjector& proj) const {
                int color_count = 0;
                for (auto bus : all_buses) {
                    if (bus->is_loop) {
                        
                        svg::Text bus_label;
                        geo::Coordinates geo_label_coord = bus->stops.front()->coordinates;
                        bus_label.
                            SetFillColor(GetColor(color_count++)).
                            SetPosition(proj(geo_label_coord)).
                            SetOffset({props.bus_label_offset[0], props.bus_label_offset[1]}).
                            SetFontSize(props.bus_label_font_size).
                            SetFontFamily("Verdana").
                            SetFontWeight("bold").
                            SetData(bus->name);
                        
                        svg::Text bus_label_underlayer = bus_label;     
                        bus_label_underlayer.
                            SetFillColor(props.underlayer_color).
                            SetStrokeColor(props.underlayer_color).
                            SetStrokeWidth(props.underlayer_width).
                            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                        map.Add(bus_label_underlayer);
                        map.Add(bus_label);
                    }
                    else
                    { 
                        svg::Text bus_label;
                        geo::Coordinates geo_label_coord = bus->stops.front()->coordinates;
                        bus_label.
                            SetFillColor(GetColor(color_count++)).
                            SetPosition(proj(geo_label_coord)).
                            SetOffset({ props.bus_label_offset[0], props.bus_label_offset[1] }).
                            SetFontSize(props.bus_label_font_size).
                            SetFontFamily("Verdana").
                            SetFontWeight("bold").
                            SetData(bus->name);

                        svg::Text bus_label_underlayer = bus_label;
                        bus_label_underlayer.
                            SetFillColor(props.underlayer_color).
                            SetStrokeColor(props.underlayer_color).
                            SetStrokeWidth(props.underlayer_width).
                            SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                            SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

                        map.Add(bus_label_underlayer);
                        map.Add(bus_label);

                        if (bus->stops.front() != bus->stops.at(bus->stops.size() / 2)) {                           
                        geo_label_coord = bus->stops.at(bus->stops.size()/2)->coordinates;

                        bus_label.SetPosition(proj(geo_label_coord));
                        bus_label_underlayer.SetPosition(proj(geo_label_coord));

                        map.Add(bus_label_underlayer);
                        map.Add(bus_label);
                        } 
                        

                    }
                }
            }

            void RenderStops(svg::Document& map, std::vector<const Bus*> all_buses, const SphereProjector& proj) const {
                auto Pr = [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name; };
                std::set<const Stop*, decltype(Pr)> stops(Pr);
                for (const Bus* bus : all_buses) {
                    stops.insert(bus->stops.begin(), bus->stops.end());
                }

                for (const auto& stop : stops) {
                    svg::Circle stop_circle;
                    stop_circle.
                        SetCenter(proj(stop->coordinates)).
                        SetRadius(props.stop_radius).
                        SetFillColor("white");
                    map.Add(stop_circle);
                }
            }

            void RenderStopNames(svg::Document& map, std::vector<const Bus*> all_buses, const SphereProjector& proj) const {
                auto Pr = [](const Stop* lhs, const Stop* rhs) { return lhs->name < rhs->name; };
                std::set<const Stop*, decltype(Pr)> stops(Pr);
                for (const Bus* bus : all_buses) {
                    stops.insert(bus->stops.begin(), bus->stops.end());
                }

                for (const auto& stop : stops) {
                    svg::Text stop_label;

                    stop_label.
                        SetFillColor("black").
                        SetPosition(proj(stop->coordinates)).
                        SetOffset({ props.stop_label_offset[0], props.stop_label_offset[1] }).
                        SetFontSize(props.stop_label_font_size).
                        SetFontFamily("Verdana").
                        SetData(stop->name);

                    svg::Text stop_label_underlayer = stop_label;
                    stop_label_underlayer.
                        SetFillColor(props.underlayer_color).
                        SetStrokeColor(props.underlayer_color).
                        SetStrokeWidth(props.underlayer_width).
                        SetStrokeLineCap(svg::StrokeLineCap::ROUND).
                        SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
                    map.Add(stop_label_underlayer);
                    map.Add(stop_label); 
                }           
            }

            svg::Color GetColor(int num) const {
                return props.color_palette[num % props.color_palette.size()];
            }
            struct VisualiseProps {

                //width и height — ширина и высота изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
                double width, height;

                //padding — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше min(width, height)/2.
                double padding;

                //line_width — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
                double line_width;

                //stop_radius — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
                double stop_radius;

                //bus_label_font_size — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
                int bus_label_font_size;

                //bus_label_offset — смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Элементы массива — числа в диапазоне от –100000 до 100000.
                std::array<double, 2> bus_label_offset;

                //stop_label_font_size — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
                int stop_label_font_size;

                //stop_label_offset — смещение названия остановки относительно её координат на карте.
                //Массив из двух элементов типа double. Задаёт значения свойств dx и dy SVG-элемента <text>. Числа в диапазоне от –100000 до 100000.
                std::array<double, 2> stop_label_offset;

                //underlayer_color — цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
                svg::Color underlayer_color;

                //underlayer_width — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута stroke-width элемента <text>.
                //Вещественное число в диапазоне от 0 до 100000.
                double underlayer_width;

                //color_palette - цветовая палитра. Непустой массив.
                std::vector<svg::Color> color_palette;
            };
            VisualiseProps props;
        };

    }
}