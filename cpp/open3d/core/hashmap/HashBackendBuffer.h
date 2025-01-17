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

#pragma once

#include <assert.h>

#include <atomic>
#include <memory>
#include <vector>

#include "open3d/core/MemoryManager.h"
#include "open3d/core/Tensor.h"

namespace open3d {
namespace core {

void CPUResetHeap(Tensor &heap);

#ifdef BUILD_CUDA_MODULE
void CUDAResetHeap(Tensor &heap);
#endif

// The heap array stores the indices of the key/values buffers. It is not
// injective.
// During Allocate, an buffer index (buf_index) is extracted from the
// heap; During Free, a buf_index is put back to the top of the heap.
// ---------------------------------------------------------------------
// heap  ---Malloc-->  heap  ---Malloc-->  heap  ---Free(0)-->  heap
// N-1                 N-1                  N-1                  N-1   |
//  .                   .                    .                    .    |
//  .                   .                    .                    .    |
//  .                   .                    .                    .    |
//  3                   3                    3                    3    |
//  2                   2                    2 <-                 2    |
//  1                   1 <-                 1                    0 <- |
//  0 <- heap_top       0                    0                    0

// Buffer index type for the internal heap.
using buf_index_t = uint32_t;

class HashBackendBuffer {
public:
    struct HeapTop {
        Tensor cuda;
        std::atomic<int> cpu = {0};
    };

    HashBackendBuffer(int64_t capacity,
                      int64_t key_dsize,
                      std::vector<int64_t> value_dsizes,
                      const Device &device);

    /// Reset the heap and heap top.
    void ResetHeap();

    /// Return device of the buffer.
    Device GetDevice() const;

    /// Return capacity of the buffer.
    int64_t GetCapacity() const;

    /// Return key's data size in bytes.
    int64_t GetKeyDsize() const;

    /// Return value's data sizes in bytes.
    std::vector<int64_t> GetValueDsizes() const;

    /// Return the index heap tensor.
    Tensor GetIndexHeap() const;

    /// Return the heap top structure. To be dispatched accordingly in C++/CUDA
    /// accessors.
    HeapTop &GetHeapTop();

    /// Return the current heap top.
    int GetHeapTopIndex() const;

    /// Return the key buffer tensor.
    Tensor GetKeyBuffer() const;

    /// Return the value buffer tensors.
    std::vector<Tensor> GetValueBuffers() const;

    /// Return the selected value buffer tensor at index i.
    Tensor GetValueBuffer(size_t i = 0) const;

protected:
    Tensor heap_;
    HeapTop heap_top_;

    Tensor key_buffer_;
    std::vector<Tensor> value_buffers_;
};
}  // namespace core
}  // namespace open3d
