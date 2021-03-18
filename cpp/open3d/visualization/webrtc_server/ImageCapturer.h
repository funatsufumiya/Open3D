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

#include <api/video/i420_buffer.h>
#include <libyuv/convert.h>
#include <libyuv/video_common.h>
#include <media/base/video_broadcaster.h>
#include <media/base/video_common.h>
#include <modules/desktop_capture/desktop_capture_options.h>
#include <modules/desktop_capture/desktop_capturer.h>
#include <pc/video_track_source.h>
#include <rtc_base/logging.h>

#include <chrono>
#include <iostream>
#include <memory>
#include <thread>

#include "open3d/core/Tensor.h"
#include "open3d/t/io/ImageIO.h"
#include "open3d/utility/Console.h"
#include "open3d/visualization/webrtc_server/CustomTrackSource.h"
#include "open3d/visualization/webrtc_server/GlobalBuffer.h"

namespace open3d {
namespace visualization {
namespace webrtc_server {

class ImageCapturer : public rtc::VideoSourceInterface<webrtc::VideoFrame> {
public:
    ImageCapturer(const std::string& url_,
                  const std::map<std::string, std::string>& opts);
    virtual ~ImageCapturer();

    static ImageCapturer* Create(
            const std::string& url,
            const std::map<std::string, std::string>& opts);

    ImageCapturer(const std::map<std::string, std::string>& opts);

    // Overide rtc::VideoSourceInterface<webrtc::VideoFrame>.
    virtual void AddOrUpdateSink(
            rtc::VideoSinkInterface<webrtc::VideoFrame>* sink,
            const rtc::VideoSinkWants& wants) override;

    virtual void RemoveSink(
            rtc::VideoSinkInterface<webrtc::VideoFrame>* sink) override;

protected:
    bool Init();
    void CaptureThread();
    bool Start();
    void Stop();
    bool IsRunning();
    void OnCaptureResult(const std::shared_ptr<core::Tensor>& frame);
    std::thread capture_thread_;
    int width_;
    int height_;
    bool is_running_;
    rtc::VideoBroadcaster broadcaster_;
};

class ImageCapturerTrackSource : public webrtc::VideoTrackSource {
public:
    static rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> Create(
            const std::string& video_url,
            const std::map<std::string, std::string>& opts) {
        // TODO: remove this check after standarizing the track names.
        if (video_url.find("image://") != 0) {
            utility::LogError(
                    "ImageCapturerTrackSource::Create failed for video_url: {}",
                    video_url);
        }
        std::unique_ptr<ImageCapturer> capturer =
                absl::WrapUnique(ImageCapturer::Create(video_url, opts));
        if (!capturer) {
            return nullptr;
        }
        rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> video_source =
                new rtc::RefCountedObject<ImageCapturerTrackSource>(
                        std::move(capturer));
        return video_source;
    }

protected:
    explicit ImageCapturerTrackSource(std::unique_ptr<ImageCapturer> capturer)
        : webrtc::VideoTrackSource(/*remote=*/false),
          capturer_(std::move(capturer)) {}

private:
    rtc::VideoSourceInterface<webrtc::VideoFrame>* source() override {
        return capturer_.get();
    }
    std::unique_ptr<ImageCapturer> capturer_;
};

}  // namespace webrtc_server
}  // namespace visualization
}  // namespace open3d