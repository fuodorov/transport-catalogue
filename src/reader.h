#pragma once
#include "catalogue.h"
#include "json/json.h"
#include "renderer.h"
#include "router.h"
#include "serializer.h"

namespace transport_catalogue::json {

class Parser {
 public:
  Parser() = default;
  Parser(Document doc);
  Parser(std::istream &input);

  void ProcessNodeTransportCatalogue(const Node &root, TransportCatalogue &catalogue);
  void ProcessNodeStatisticRequest(const Node &root,
                       std::vector<StatisticRequest> &stat_request);
  void ProcessNodeRenderSettings(const Node &node,
                         map_renderer::RenderSettings &render_settings);
  void ProcessNodeRoutingSettings(const Node &node, router::RoutingSettings &route_set);
  void ProcessNodeSerializationSettings(
      const Node &node,
      serialization::SerializationSettings &serialization_set);

  void ProcessTransportCatalogue(
      TransportCatalogue &catalogue,
      map_renderer::RenderSettings &render_settings,
      router::RoutingSettings &routing_settings,
      serialization::SerializationSettings &serialization_settings);

  void ProcessRequests(
      std::vector<StatisticRequest> &stat_request,
      serialization::SerializationSettings &serialization_settings);

  Stop ProcessNodeStop(Node &node);
  Bus ProcessNodeBus(Node &node, TransportCatalogue &catalogue);
  std::vector<Distance> ProcessNodeDistances(Node &node,
                                             TransportCatalogue &catalogue);

  const Document &GetDocument() const;

 private:
  Document document;
};

}  // namespace transport_catalogue::json