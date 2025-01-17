#include <plateau/polygon_mesh/mesh_merger.h>
#include <cassert>
#include "libplateau_c.h"

using namespace libplateau;
using namespace plateau::polygonMesh;
using namespace plateau::geometry;

extern "C" {
    LIBPLATEAU_C_EXPORT APIResult LIBPLATEAU_C_API plateau_mesh_merger_merge_mesh(
            Mesh* mesh,
            Mesh* other_mesh,
            bool invert_mesh_front_back,
            bool include_texture
    ) {
        API_TRY {
            MeshMerger::mergeMesh(
                    *mesh, *other_mesh, invert_mesh_front_back, include_texture,
                    TVec2f(0, 0), TVec2f(0, 0)
            );
            return APIResult::Success;
        } API_CATCH;
        return APIResult::ErrorUnknown;
    }

    LIBPLATEAU_C_EXPORT APIResult LIBPLATEAU_C_API plateau_mesh_merger_mesh_info(
            Mesh* mesh,
            TVec3d* vertices_array,
            int vertices_count,
            unsigned* indices_array,
            int indices_count,
            TVec2f* uv_1_array,
            int uv_1_count,
            SubMesh** sub_mesh_pointers_array,
            int sub_mesh_count,
            CoordinateSystem mesh_axis_convert_from,
            CoordinateSystem mesh_axis_convert_to,
            bool include_texture
    ) {
        API_TRY {
            auto vertices = std::vector<TVec3d>(vertices_array, vertices_array + vertices_count);
            auto indices = std::vector<unsigned>(indices_array, indices_array + indices_count);
            auto uv_1 = std::vector<TVec2f>(uv_1_array, uv_1_array + uv_1_count);

            auto sub_meshes = std::vector<SubMesh>();
            for (int i = 0; i < sub_mesh_count; i++) {
                SubMesh* sub_mesh_ptr = *(sub_mesh_pointers_array + i);
                sub_meshes.push_back(*(sub_mesh_ptr));
            }

            MeshMerger::mergeMeshInfo(*mesh,
                                      std::move(vertices), std::move(indices), std::move(uv_1),
                                      std::move(sub_meshes),
                                      mesh_axis_convert_from, mesh_axis_convert_to, include_texture);
            return APIResult::Success;
        } API_CATCH;
        return APIResult::ErrorUnknown;
    }
}
