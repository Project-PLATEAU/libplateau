#include <plateau/polygon_mesh/mesh_extractor.h>
#include <plateau/polygon_mesh/primary_city_object_types.h>
#include "citygml/vecs.hpp"
#include "citygml/texture.h"
#include "../src/polygon_mesh/grid_merger.h"
#include "citygml/cityobject.h"
#include <plateau/polygon_mesh/mesh_merger.h>
#include <plateau/polygon_mesh/polygon_mesh_utils.h>
#include <plateau/mesh_writer/obj_writer.h>

namespace plateau::polygonMesh {
    namespace {
        bool shouldSkipCityObj(const citygml::CityObject& city_obj, const MeshExtractOptions& options){
            return options.exclude_city_object_outside_extent && !options.extent.contains(city_obj);
        }

        void extractInner(Model& out_model, const citygml::CityModel& city_model,
                          const MeshExtractOptions& options) {
            if (options.max_lod < options.min_lod) throw std::logic_error("Invalid LOD range.");

            const auto geo_reference = geometry::GeoReference(options.coordinate_zone_id, options.reference_point, options.unit_scale, options.mesh_axes);

            // rootNode として LODノード を作ります。
            for (unsigned lod = options.min_lod; lod <= options.max_lod; lod++) {
                auto lod_node = Node("LOD" + std::to_string(lod));

                // LODノードの下にメッシュ配置用ノードを作ります。
                switch (options.mesh_granularity) {
                    case MeshGranularity::PerCityModelArea: {
                        // 次のような階層構造を作ります:
                        // model -> LODノード -> グループごとのノード

                        // 都市をグループに分け、グループごとにメッシュをマージします。
                        auto result = GridMerger::gridMerge(city_model, options, lod, geo_reference);
                        // グループごとのノードを追加します。
                        for (auto& pair: result) {
                            unsigned group_id = pair.first;
                            auto& mesh = pair.second;
                            auto node = Node("group" + std::to_string(group_id), std::move(mesh));
                            lod_node.addChildNode(std::move(node));
                        }
                    }
                        break;
                    case MeshGranularity::PerPrimaryFeatureObject: {
                        // 次のような階層構造を作ります：
                        // model -> LODノード -> 主要地物ごとのノード

                        auto& primary_city_objs = city_model.getAllCityObjectsOfType(
                                PrimaryCityObjectTypes::getPrimaryTypeMask());

                        // 主要地物ごとにメッシュを結合します。
                        for (auto primary_obj: primary_city_objs) {
                            // 範囲外ならスキップします。
                            if (shouldSkipCityObj(*primary_obj, options)) continue;
                            // 主要地物のメッシュを作ります。
                            Mesh mesh;
                            if (MeshExtractor::shouldContainPrimaryMesh(lod, *primary_obj)) {
                                MeshMerger::mergePolygonsInCityObject(mesh, *primary_obj, lod, options, geo_reference,
                                                                      TVec2f{0, 0},
                                                                      TVec2f{0, 0}, city_model.getGmlPath());
                            }
                            if (lod >= 2) {
                                // 主要地物の子である各最小地物をメッシュに加えます。
                                auto atomic_objs = PolygonMeshUtils::getChildCityObjectsRecursive(*primary_obj);
                                MeshMerger::mergePolygonsInCityObjects(mesh, atomic_objs, lod, options, geo_reference,
                                                                       TVec2f{0, 0},
                                                                       TVec2f{0, 0}, city_model.getGmlPath());
                            }
                            // 主要地物ごとのノードを追加します。
                            lod_node.addChildNode(Node(primary_obj->getId(), std::move(mesh)));
                        }
                    }
                        break;
                    case MeshGranularity::PerAtomicFeatureObject: {
                        // 次のような階層構造を作ります：
                        // model -> LODノード -> 主要地物ごとのノード -> その子の最小地物ごとのノード
                        auto& primary_city_objs = city_model.getAllCityObjectsOfType(
                                PrimaryCityObjectTypes::getPrimaryTypeMask());
                        for (auto primary_obj: primary_city_objs) {
                            // 範囲外ならスキップします。
                            if(shouldSkipCityObj(*primary_obj, options)) continue;
                            // 主要地物のノードを作成します。
                            auto primary_mesh = std::optional<Mesh>(std::nullopt);

                            if (MeshExtractor::shouldContainPrimaryMesh(lod, *primary_obj)) {
                                primary_mesh = Mesh();
                                MeshMerger::mergePolygonsInCityObject(primary_mesh.value(), *primary_obj, lod, options,
                                                                      geo_reference,
                                                                      TVec2f{0, 0},
                                                                      TVec2f{0, 0}, city_model.getGmlPath());
                            }
                            auto primary_node = Node(primary_obj->getId(), std::move(primary_mesh));
                            // 最小地物ごとにノードを作成します。
                            auto atomic_objs = PolygonMeshUtils::getChildCityObjectsRecursive(*primary_obj);
                            for (auto atomic_obj: atomic_objs) {
                                // 最小地物のノードを作成
                                auto atomic_mesh = Mesh();
                                MeshMerger::mergePolygonsInCityObject(atomic_mesh, *atomic_obj, lod, options,
                                                                      geo_reference,
                                                                      TVec2f{0, 0},
                                                                      TVec2f{0, 0}, city_model.getGmlPath());
                                auto atomic_node = Node(atomic_obj->getId(), std::move(atomic_mesh));
                                primary_node.addChildNode(std::move(atomic_node));
                            }
                            lod_node.addChildNode(std::move(primary_node));
                        }

                    }
                        break;
                    default:
                        throw std::logic_error("Unknown enum type of options.mesh_granularity .");
                }

                out_model.addNode(std::move(lod_node));
            }
            out_model.eraseEmptyNodes();
        }
    }

    std::shared_ptr<Model> MeshExtractor::extract(const citygml::CityModel& city_model,
                                                  const MeshExtractOptions& options) {
        auto result = std::make_shared<Model>();
        extract(*result, city_model, options);
        return result;
    }

    void MeshExtractor::extract(Model& out_model, const citygml::CityModel& city_model,
                                const MeshExtractOptions& options) {
        extractInner(out_model, city_model, options);
    }

    bool MeshExtractor::shouldContainPrimaryMesh(unsigned lod, const citygml::CityObject& primary_obj) {
        return !(lod >= 2 && // ( LODが2以上 かつ
                 (primary_obj.getType() & citygml::CityObject::CityObjectsType::COT_Building) !=
                 (citygml::CityObject::CityObjectsType) 0); // ビット演算で、フラグに Building を含む)の NOT
    }
}
