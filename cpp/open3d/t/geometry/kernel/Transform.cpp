// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2018-2021 www.open3d.org
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// ----------------------------------------------------------------------------

#include "open3d/t/geometry/kernel/Transform.h"

#include "open3d/core/CUDAUtils.h"
#include "open3d/core/ShapeUtil.h"

namespace open3d {
namespace t {
namespace geometry {
namespace kernel {
namespace transform {

void TransformPoints(const core::Tensor& transformation, core::Tensor& points) {
    points.AssertShape({utility::nullopt, 3});
    transformation.AssertShape({4, 4});
    core::Dtype dtype = points.GetDtype();
    transformation.AssertDtype(dtype);
    core::Device device = points.GetDevice();
    transformation.AssertDevice(device);

    core::Tensor points_contiguous = points.Contiguous();
    core::Tensor transformation_contiguous = transformation.Contiguous();

    core::Device::DeviceType device_type = device.GetType();
    if (device_type == core::Device::DeviceType::CPU) {
        TransformPointsCPU(transformation_contiguous, points_contiguous);
    } else if (device_type == core::Device::DeviceType::CUDA) {
        CUDA_CALL(TransformPointsCUDA, transformation_contiguous,
                  points_contiguous);
    } else {
        utility::LogError("Unimplemented device");
    }

    points = points_contiguous;
}

void TransformNormals(const core::Tensor& transformation,
                      core::Tensor& normals) {
    normals.AssertShape({utility::nullopt, 3});
    transformation.AssertShape({4, 4});
    core::Dtype dtype = normals.GetDtype();
    transformation.AssertDtype(dtype);
    core::Device device = normals.GetDevice();
    transformation.AssertDevice(device);

    core::Tensor normals_contiguous = normals.Contiguous();
    core::Tensor transformation_contiguous = transformation.Contiguous();

    core::Device::DeviceType device_type = device.GetType();
    if (device_type == core::Device::DeviceType::CPU) {
        TransformNormalsCPU(transformation_contiguous, normals_contiguous);
    } else if (device_type == core::Device::DeviceType::CUDA) {
        CUDA_CALL(TransformNormalsCUDA, transformation_contiguous,
                  normals_contiguous);
    } else {
        utility::LogError("Unimplemented device");
    }

    normals = normals_contiguous;
}

void RotatePoints(const core::Tensor& R,
                  core::Tensor& points,
                  const core::Tensor& center) {
    points.AssertShape({utility::nullopt, 3});
    R.AssertShape({3, 3});
    center.AssertShape({3});
    core::Dtype dtype = points.GetDtype();
    R.AssertDtype(dtype);
    center.AssertDtype(dtype);
    core::Device device = points.GetDevice();
    R.AssertDevice(device);
    center.AssertDevice(device);

    core::Tensor points_contiguous = points.Contiguous();
    core::Tensor R_contiguous = R.Contiguous();
    core::Tensor center_contiguous = center.Contiguous();

    core::Device::DeviceType device_type = device.GetType();
    if (device_type == core::Device::DeviceType::CPU) {
        RotatePointsCPU(R_contiguous, points_contiguous, center_contiguous);
    } else if (device_type == core::Device::DeviceType::CUDA) {
        CUDA_CALL(RotatePointsCUDA, R_contiguous, points_contiguous,
                  center_contiguous);
    } else {
        utility::LogError("Unimplemented device");
    }

    points = points_contiguous;
}

void RotateNormals(const core::Tensor& R, core::Tensor& normals) {
    normals.AssertShape({utility::nullopt, 3});
    R.AssertShape({3, 3});
    core::Dtype dtype = normals.GetDtype();
    R.AssertDtype(dtype);
    core::Device device = normals.GetDevice();
    R.AssertDevice(device);

    core::Tensor normals_contiguous = normals.Contiguous();
    core::Tensor R_contiguous = R.Contiguous();

    core::Device::DeviceType device_type = device.GetType();
    if (device_type == core::Device::DeviceType::CPU) {
        RotateNormalsCPU(R_contiguous, normals_contiguous);
    } else if (device_type == core::Device::DeviceType::CUDA) {
        CUDA_CALL(RotateNormalsCUDA, R_contiguous, normals_contiguous);
    } else {
        utility::LogError("Unimplemented device");
    }

    normals = normals_contiguous;
}

}  // namespace transform
}  // namespace kernel
}  // namespace geometry
}  // namespace t
}  // namespace open3d
