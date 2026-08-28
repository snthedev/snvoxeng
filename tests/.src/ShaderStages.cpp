#include <tests/tests/ShaderStages.hpp>

#include <snvoxeng/snvoxeng/ShaderStages.hpp>

#include <filesystem>

// === Tests Context ===

void Test_ShaderStages::SetUp() {}
void Test_ShaderStages::TearDown() {}

// === Tests ===

TEST_F(Test_ShaderStages, getShaderStageFlag_expectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageFlag;

    EXPECT_EQ(f(eShaderStage::eVertex), eShaderStageFlag::eNone);
    EXPECT_EQ(f(eShaderStage::eGeometry), eShaderStageFlag::eNone);
    EXPECT_EQ(f(eShaderStage::eFragment), eShaderStageFlag::eNone);
    EXPECT_EQ(f(eShaderStage::eCompute), eShaderStageFlag::eNone);
    EXPECT_EQ(f(eShaderStage::eTessControl), eShaderStageFlag::eNone);
    EXPECT_EQ(f(eShaderStage::eTessEvaluation), eShaderStageFlag::eNone);

    EXPECT_EQ(f(eShaderStage::eRaygen), eShaderStageFlag::eKHR);
    EXPECT_EQ(f(eShaderStage::eAnyHit), eShaderStageFlag::eKHR);
    EXPECT_EQ(f(eShaderStage::eClosestHit), eShaderStageFlag::eKHR);
    EXPECT_EQ(f(eShaderStage::eMiss), eShaderStageFlag::eKHR);
    EXPECT_EQ(f(eShaderStage::eIntersection), eShaderStageFlag::eKHR);
    EXPECT_EQ(f(eShaderStage::eCallable), eShaderStageFlag::eKHR);

    EXPECT_EQ(f(eShaderStage::eTask), eShaderStageFlag::eEXT);
    EXPECT_EQ(f(eShaderStage::eMesh), eShaderStageFlag::eEXT);
}
TEST_F(Test_ShaderStages, getShaderStageVkType_expectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageVkType;

    EXPECT_EQ(f(eShaderStage::eVertex), VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
    EXPECT_EQ(f(eShaderStage::eGeometry), VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT);
    EXPECT_EQ(f(eShaderStage::eFragment), VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
    EXPECT_EQ(f(eShaderStage::eCompute), VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT);
    EXPECT_EQ(f(eShaderStage::eTessControl), VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
    EXPECT_EQ(f(eShaderStage::eTessEvaluation), VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);

    EXPECT_EQ(f(eShaderStage::eRaygen), VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR);
    EXPECT_EQ(f(eShaderStage::eAnyHit), VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
    EXPECT_EQ(f(eShaderStage::eClosestHit), VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
    EXPECT_EQ(f(eShaderStage::eMiss), VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR);
    EXPECT_EQ(f(eShaderStage::eIntersection), VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
    EXPECT_EQ(f(eShaderStage::eCallable), VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR);

    EXPECT_EQ(f(eShaderStage::eTask), VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT);
    EXPECT_EQ(f(eShaderStage::eMesh), VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT);
}
TEST_F(Test_ShaderStages, getShaderStageShadercType_expectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageShadercType;

    EXPECT_EQ(f(eShaderStage::eVertex), shaderc_shader_kind::shaderc_vertex_shader);
    EXPECT_EQ(f(eShaderStage::eGeometry), shaderc_shader_kind::shaderc_geometry_shader);
    EXPECT_EQ(f(eShaderStage::eFragment), shaderc_shader_kind::shaderc_fragment_shader);
    EXPECT_EQ(f(eShaderStage::eCompute), shaderc_shader_kind::shaderc_compute_shader);
    EXPECT_EQ(f(eShaderStage::eTessControl), shaderc_shader_kind::shaderc_tess_control_shader);
    EXPECT_EQ(f(eShaderStage::eTessEvaluation), shaderc_shader_kind::shaderc_tess_evaluation_shader);

    EXPECT_EQ(f(eShaderStage::eRaygen), shaderc_shader_kind::shaderc_raygen_shader);
    EXPECT_EQ(f(eShaderStage::eAnyHit), shaderc_shader_kind::shaderc_anyhit_shader);
    EXPECT_EQ(f(eShaderStage::eClosestHit), shaderc_shader_kind::shaderc_closesthit_shader);
    EXPECT_EQ(f(eShaderStage::eMiss), shaderc_shader_kind::shaderc_miss_shader);
    EXPECT_EQ(f(eShaderStage::eIntersection), shaderc_shader_kind::shaderc_intersection_shader);
    EXPECT_EQ(f(eShaderStage::eCallable), shaderc_shader_kind::shaderc_callable_shader);

    EXPECT_EQ(f(eShaderStage::eTask), shaderc_shader_kind::shaderc_task_shader);
    EXPECT_EQ(f(eShaderStage::eMesh), shaderc_shader_kind::shaderc_mesh_shader);
}

/*
getShaderStage_name
getShaderStageName
getShaderStageExt
- for debug only, skip
*/

TEST_F(Test_ShaderStages, getShaderStageByVkType_expectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageByVkType;

    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT), eShaderStage::eVertex);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT), eShaderStage::eGeometry);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT), eShaderStage::eFragment);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT), eShaderStage::eCompute);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT), eShaderStage::eTessControl);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT), eShaderStage::eTessEvaluation);

    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR), eShaderStage::eRaygen);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR), eShaderStage::eAnyHit);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR), eShaderStage::eClosestHit);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_MISS_BIT_KHR), eShaderStage::eMiss);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_INTERSECTION_BIT_KHR), eShaderStage::eIntersection);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_CALLABLE_BIT_KHR), eShaderStage::eCallable);

    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT), eShaderStage::eTask);
    EXPECT_EQ(f(VkShaderStageFlagBits::VK_SHADER_STAGE_MESH_BIT_EXT), eShaderStage::eMesh);
}
TEST_F(Test_ShaderStages, getShaderStageByVkType_unexpectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageByVkType;

    // Combined flags is not allowed

    EXPECT_ANY_THROW(f(static_cast<VkShaderStageFlagBits>(
        VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
        | VkShaderStageFlagBits::VK_SHADER_STAGE_GEOMETRY_BIT)
    ));
    EXPECT_ANY_THROW(f(static_cast<VkShaderStageFlagBits>(
        VkShaderStageFlagBits::VK_SHADER_STAGE_TASK_BIT_EXT
        | VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR)
    ));
    EXPECT_ANY_THROW(f(static_cast<VkShaderStageFlagBits>(
        VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
        | VkShaderStageFlagBits::VK_SHADER_STAGE_RAYGEN_BIT_KHR
        | VkShaderStageFlagBits::VK_SHADER_STAGE_ANY_HIT_BIT_KHR)
    ));

    EXPECT_ANY_THROW(f(VkShaderStageFlagBits::VK_SHADER_STAGE_ALL_GRAPHICS));
    EXPECT_ANY_THROW(f(VkShaderStageFlagBits::VK_SHADER_STAGE_ALL));

    // Unsupported stages

    EXPECT_ANY_THROW(f(VkShaderStageFlagBits::VK_SHADER_STAGE_SUBPASS_SHADING_BIT_HUAWEI));
    EXPECT_ANY_THROW(f(VkShaderStageFlagBits::VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI));

    // Invalid values

    EXPECT_ANY_THROW(f(VkShaderStageFlagBits::VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM));
}

TEST_F(Test_ShaderStages, getShaderStageByShadercType_expectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageByShadercType;

    EXPECT_EQ(f(shaderc_shader_kind::shaderc_vertex_shader), eShaderStage::eVertex);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_geometry_shader), eShaderStage::eGeometry);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_fragment_shader), eShaderStage::eFragment);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_compute_shader), eShaderStage::eCompute);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_tess_control_shader), eShaderStage::eTessControl);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_tess_evaluation_shader), eShaderStage::eTessEvaluation);

    EXPECT_EQ(f(shaderc_shader_kind::shaderc_raygen_shader), eShaderStage::eRaygen);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_anyhit_shader), eShaderStage::eAnyHit);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_closesthit_shader), eShaderStage::eClosestHit);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_miss_shader), eShaderStage::eMiss);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_intersection_shader), eShaderStage::eIntersection);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_callable_shader), eShaderStage::eCallable);

    EXPECT_EQ(f(shaderc_shader_kind::shaderc_task_shader), eShaderStage::eTask);
    EXPECT_EQ(f(shaderc_shader_kind::shaderc_mesh_shader), eShaderStage::eMesh);
}
TEST_F(Test_ShaderStages, getShaderStageByShadercType_unexpectedVals)
{
    using namespace sn::voxeng;
    const auto f = ShaderStageInfosContainer::getShaderStageByShadercType;

    // Unsupported stages

    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_infer_from_source));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_vertex_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_fragment_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_compute_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_geometry_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_tess_control_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_tess_evaluation_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_spirv_assembly));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_raygen_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_anyhit_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_closesthit_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_miss_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_intersection_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_callable_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_task_shader));
    EXPECT_ANY_THROW(f(shaderc_shader_kind::shaderc_glsl_default_mesh_shader));
}

TEST_F(Test_ShaderStages, getShaderStageByFileExtension_expectedVals)
{
    using namespace sn::voxeng;

    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension("vert"), eShaderStage::eVertex);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(".vert"), eShaderStage::eVertex);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension("hgyuvikrhjgsdfbfvgdvzs.vert"), eShaderStage::eVertex);

    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension("frag"), eShaderStage::eFragment);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(".frag"), eShaderStage::eFragment);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension("ghbnsdrtyfnx.frag"), eShaderStage::eFragment);

    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension("tesc"), eShaderStage::eTessControl);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(".tesc"), eShaderStage::eTessControl);

    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L"vert"), eShaderStage::eVertex);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L".vert"), eShaderStage::eVertex);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L"hgyuvikrhjgsdfbfvgdvzs.vert"), eShaderStage::eVertex);

    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L"frag"), eShaderStage::eFragment);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L".frag"), eShaderStage::eFragment);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L"ghbnsdrtyfnx.frag"), eShaderStage::eFragment);

    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L"tesc"), eShaderStage::eTessControl);
    EXPECT_EQ(ShaderStageInfosContainer::getShaderStageByFileExtension(L".tesc"), eShaderStage::eTessControl);
}
TEST_F(Test_ShaderStages, getShaderStageByFileExtension_unexpectedVals)
{
    using namespace sn::voxeng;

    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension("kjasdhdak"));
    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension("vert "));
    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension(".vert "));
    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension("fgdhdf.vert "));

    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension(L"kjasdhdak"));
    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension(L"vert "));
    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension(L".vert "));
    EXPECT_ANY_THROW(ShaderStageInfosContainer::getShaderStageByFileExtension(L"fgdhdf.vert "));
}

TEST_F(Test_ShaderStages, getShaderStageByFilePath_expectedVals)
{
    using namespace sn::voxeng;

    const auto f = ShaderStageInfosContainer::getShaderStageByFilePath;

    // some filename
    EXPECT_EQ(f(std::filesystem::path("base") / "hgyuvikrhjgsdfbfvgdvzs.vert"), eShaderStage::eVertex);
}
TEST_F(Test_ShaderStages, getShaderStageByFilePath_unexpectedVals)
{
    using namespace sn::voxeng;

    const auto f = ShaderStageInfosContainer::getShaderStageByFilePath;

    // not file
    EXPECT_ANY_THROW(f(std::filesystem::path("base") / "vert"));

    // empty filename
    EXPECT_ANY_THROW(f(std::filesystem::path("base") / ".vert"));

    // empty filepath
    EXPECT_ANY_THROW(f(std::filesystem::path()));

    // unknown ext
    EXPECT_ANY_THROW(f(std::filesystem::path("base") / "main.cpp"));
}
