// ----------------------------------------------------------------------------
// -                        Open3D: www.open3d.org                            -
// ----------------------------------------------------------------------------
// The MIT License (MIT)
//
// Copyright (c) 2021 www.open3d.org
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

#pragma once
#include <condition_variable>
#include <mutex>

#include "open3d/core/Tensor.h"
#include "open3d/t/io/ImageIO.h"

namespace open3d {
namespace visualization {
namespace webrtc_server {

class GlobalBuffer {
public:
    static GlobalBuffer& GetInstance() {
        static GlobalBuffer instance;
        return instance;
    }

    void Read(core::Tensor& out_frame) {
        std::unique_lock<std::mutex> ul(g_mutex);
        g_cv.wait(ul, [this]() { return this->g_ready; });
        out_frame = rgb_buffer_.Clone();
        g_ready = false;
    }

    void Write(const core::Tensor& rgb_buffer) {
        rgb_buffer.AssertShape(
                {rgb_buffer_.GetShape(0), rgb_buffer_.GetShape(1), 3});
        rgb_buffer.AssertDtype(rgb_buffer_.GetDtype());
        rgb_buffer.AssertDevice(rgb_buffer_.GetDevice());

        // Updating buffer is not protected by mutex, but it is fine since
        // partial frame is acceptable to minimize latency.
        rgb_buffer_.AsRvalue() = rgb_buffer;

        std::unique_lock<std::mutex> ul(g_mutex);
        g_ready = true;
        ul.unlock();
        g_cv.notify_one();
    }

private:
    GlobalBuffer() {
        core::Tensor two_fifty_five =
                core::Tensor::Ones({}, core::Dtype::UInt8) * 255;
        rgb_buffer_ = core::Tensor::Zeros({480, 640, 3}, core::Dtype::UInt8);
        rgb_buffer_.Slice(0, 0, 160, 1).Slice(2, 0, 1, 1) = two_fifty_five;
        rgb_buffer_.Slice(0, 160, 320, 1).Slice(2, 1, 2, 1) = two_fifty_five;
        rgb_buffer_.Slice(0, 320, 480, 1).Slice(2, 2, 3, 1) = two_fifty_five;
    }

    virtual ~GlobalBuffer() {}

    core::Tensor rgb_buffer_;
    std::mutex g_mutex;
    std::condition_variable g_cv;
    bool g_ready = false;
};

}  // namespace webrtc_server
}  // namespace visualization
}  // namespace open3d
