#pragma once
#include <vector>
#include <memory>
#include <string>
#include <xstring>
#include <unordered_map>
#include <fstream>

#include <DirectXMath.h>

enum channel_t
{
    ANIMATION_CHANNEL_XPOS,
    ANIMATION_CHANNEL_YPOS,
    ANIMATION_CHANNEL_ZPOS,
    ANIMATION_CHANNEL_XROT,
    ANIMATION_CHANNEL_YROT,
    ANIMATION_CHANNEL_ZROT
};

class AnimatedObjectClass
{
public:
    using FrameShape = std::unordered_map<std::string, DirectX::XMMATRIX>;
    
    struct AnimationNode
    {
        float offset_x, offset_y, offset_z;

        std::string name;
        std::vector<channel_t> channels;

        std::weak_ptr<AnimationNode> parent;
        std::vector<std::shared_ptr<AnimationNode>> children;

        DirectX::XMMATRIX link_matrix;
        DirectX::XMMATRIX shape_transform;
        DirectX::XMMATRIX global_transform;

        AnimationNode(std::string name, std::weak_ptr<AnimationNode> parent = std::weak_ptr<AnimationNode>())
            : name(name), parent(parent)
        {
            global_transform = link_matrix = DirectX::XMMatrixIdentity();
        }

        ~AnimationNode() = default;
    };

public:
    AnimatedObjectClass(const char* filename, size_t start_frame = 0, DirectX::XMFLOAT3 scaling = { 1.f, 1.f, 1.f }, DirectX::XMFLOAT3 offset = {0, 0, 0});
    ~AnimatedObjectClass();

    /// @brief Merges two frame shapes with linear interpolation.
    /// @param first The first frame shape.
    /// @param second The second frame shape.
    /// @param alpha The interpolation factor (0.0 to 1.0).
    ///              Alpha = 0.0 returns the first shape, alpha = 1.0 returns the second shape.
    /// @return The merged frame shape.
    static FrameShape MergeFrameShapes(
        const FrameShape& first,
        const FrameShape& second,
        float alpha
    );

    FrameShape UpdateAndGetShapeMatrix(const int frame, DirectX::XMMATRIX transform_of_root);
    FrameShape GetJointMatrix(const int frame) const;

private:
    std::shared_ptr<AnimationNode> create_hierarchy(
        std::shared_ptr<AnimationNode> curr_node, std::ifstream& fin);

private:
    std::wstring filename_;
    int channels_num_, frames_num_;
    float frame_time_;

	DirectX::XMFLOAT3 scaling_, offset_;


    std::vector<std::shared_ptr<AnimationNode>> nodes_;
    std::vector<float> frame_info_;
};
