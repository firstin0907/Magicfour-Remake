#pragma once

#include <directxmath.h>
#include <DirectXCollision.h>

#include <array>
#include <variant>

struct FrustumCuller
{
    FrustumCuller(const DirectX::XMMATRIX& vp_matrix) : frustum(vp_matrix) {};

    bool IsInFrustum(const std::variant<DirectX::BoundingBox, DirectX::BoundingSphere> bv,
                        const DirectX::XMMATRIX& world_matrix = DirectX::XMMatrixIdentity()) const
    {
        switch(bv.index())
        {
            case 0: // BoundingBox
            {
                DirectX::BoundingBox transformed_box;
                std::get<DirectX::BoundingBox>(bv).Transform(transformed_box, world_matrix);
                return frustum.Intersects(transformed_box);
            }

            case 1: // BoundingSphere
            {
                DirectX::BoundingSphere transformed_sphere;
                std::get<DirectX::BoundingSphere>(bv).Transform(transformed_sphere, world_matrix);
                return frustum.Intersects(transformed_sphere);
            }
        }
        return false;
    }

    DirectX::BoundingFrustum frustum;
};
