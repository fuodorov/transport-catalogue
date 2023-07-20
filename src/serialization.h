#pragma once

#include "catalogue.h"
#include "catalogue.pb.h"

#include "svg.pb.h"

#include "renderer.h"
#include "renderer.pb.h"

#include "router.h"
#include "router.pb.h"

#include <iostream>

namespace serialization {
    
struct SerializationSettings {std::string file_name;};
    
struct Catalogue {
    transport_catalogue::TransportCatalogue transport_catalogue_;
    map_renderer::RenderSettings render_settings_;
    domain::RoutingSettings routing_settings_;
};
    
template <typename It>
uint32_t calculate_id(It start, It end, std::string_view name);
    
transport_catalogue_protobuf::TransportCatalogue transport_catalogue_serialization(const transport_catalogue::TransportCatalogue& transport_catalogue);
transport_catalogue::TransportCatalogue transport_catalogue_deserialization(const transport_catalogue_protobuf::TransportCatalogue& transport_catalogue_proto);

transport_catalogue_protobuf::Color color_serialization(const svg::Color& tc_color);
svg::Color color_deserialization(const transport_catalogue_protobuf::Color& color_proto);
transport_catalogue_protobuf::RenderSettings render_settings_serialization(const map_renderer::RenderSettings& render_settings);
map_renderer::RenderSettings render_settings_deserialization(const transport_catalogue_protobuf::RenderSettings& render_settings_proto);
    
transport_catalogue_protobuf::RoutingSettings routing_settings_serialization(const domain::RoutingSettings& routing_settings);
domain::RoutingSettings routing_settings_deserialization(const transport_catalogue_protobuf::RoutingSettings& routing_settings_proto);

void catalogue_serialization(const transport_catalogue::TransportCatalogue& transport_catalogue, 
                             const map_renderer::RenderSettings& render_settings,
                             const domain::RoutingSettings& routing_settings,
                             std::ostream& out); 
    
Catalogue catalogue_deserialization(std::istream& in);
    
}//end namespace serialization