/*
Copyright (c) 2022 Advanced Micro Devices, Inc. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
/*
Testcase Scenarios :
Unit_hipMemcpy2DToArrayAsync_Positive_Default - Test basic async memcpy between
host/device and 2D array with hipMemcpy2DToArrayAsync api
Unit_hipMemcpy2DToArrayAsync_Positive_Synchronization_Behavior - Test
synchronization behavior for hipMemcpy2DToArrayAsync api
Unit_hipMemcpy2DToArrayAsync_Positive_ZeroWidthHeight - Test that no data is
copied when width/height is set to 0
Unit_hipMemcpy2DToArrayAsync_Negative_Parameters - Test unsuccessful execution
of hipMemcpy2DToArrayAsync api when parameters are invalid
*/
#include "array_memcpy_tests_common.hh"

#include <hip/hip_runtime_api.h>
#include <hip_test_common.hh>
#include <resource_guards.hh>
#include <utils.hh>


TEST_CASE("Unit_hipMemcpy2DToArrayAsync_Positive_Default") {
  using namespace std::placeholders;

  const auto stream_type = GENERATE(Streams::nullstream, Streams::perThread, Streams::created);
  const StreamGuard stream_guard(stream_type);
  const hipStream_t stream = stream_guard.stream();

  const auto width = GENERATE(16, 32, 48);
  const auto height = GENERATE(1, 16, 32, 48);

  SECTION("Host to Array") {
    Memcpy2DHosttoAShell<true, int>(
        std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), height,
                  hipMemcpyHostToDevice, stream),
        width, height, stream);
  }

  SECTION("Host to Array with default kind") {
    Memcpy2DHosttoAShell<true, int>(
        std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), height,
                  hipMemcpyDefault, stream),
        width, height, stream);
  }
#if HT_NVIDIA  // EXSWHTEC-213
  SECTION("Device to Array") {
    SECTION("Peer access disabled") {
      Memcpy2DDevicetoAShell<true, false, int>(
          std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), height,
                    hipMemcpyDeviceToDevice, stream),
          width, height, stream);
    }
    SECTION("Peer access enabled") {
      Memcpy2DDevicetoAShell<true, true, int>(
          std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), height,
                    hipMemcpyDeviceToDevice, stream),
          width, height, stream);
    }
  }

  SECTION("Device to Array with default kind") {
    SECTION("Peer access disabled") {
      Memcpy2DDevicetoAShell<true, false, int>(
          std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), height,
                    hipMemcpyDefault, stream),
          width, height, stream);
    }
    SECTION("Peer access enabled") {
      Memcpy2DDevicetoAShell<true, true, int>(
          std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), height,
                    hipMemcpyDefault, stream),
          width, height, stream);
    }
  }
#endif
}

TEST_CASE("Unit_hipMemcpy2DToArrayAsync_Positive_Synchronization_Behavior") {
  using namespace std::placeholders;
  HIP_CHECK(hipDeviceSynchronize());

  SECTION("Host to Array") {
    const auto width = GENERATE(16, 32, 48);
    const auto height = GENERATE(16, 32, 48);

    MemcpyHtoASyncBehavior(std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, width * sizeof(int),
                                     width * sizeof(int), height, hipMemcpyHostToDevice, nullptr),
                           width, height, false);
  }

  SECTION("Device to Array") {
    const auto width = GENERATE(16, 32, 48);
    const auto height = GENERATE(16, 32, 48);

    MemcpyDtoASyncBehavior(std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int),
                                     height, hipMemcpyDeviceToDevice, nullptr),
                           width, height, false);
  }
}

TEST_CASE("Unit_hipMemcpy2DToArrayAsync_Positive_ZeroWidthHeight") {
  using namespace std::placeholders;
  const auto width = 16;
  const auto height = 16;

  const auto stream_type = GENERATE(Streams::nullstream, Streams::perThread, Streams::created);
  const StreamGuard stream_guard(stream_type);
  const hipStream_t stream = stream_guard.stream();

  SECTION("Array to host") {
    SECTION("Height is 0") {
      Memcpy2DToArrayZeroWidthHeight<false>(
          std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), 0,
                    hipMemcpyHostToDevice, stream),
          width, height, stream);
    }
    SECTION("Width is 0") {
      Memcpy2DToArrayZeroWidthHeight<false>(std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, 0,
                                                      height, hipMemcpyHostToDevice, stream),
                                            width, height, stream);
    }
  }
  SECTION("Array to device") {
    SECTION("Height is 0") {
      Memcpy2DToArrayZeroWidthHeight<false>(
          std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, width * sizeof(int), 0,
                    hipMemcpyDeviceToDevice, stream),
          width, height, stream);
    }
    SECTION("Width is 0") {
      Memcpy2DToArrayZeroWidthHeight<false>(std::bind(hipMemcpy2DToArrayAsync, _1, 0, 0, _2, _3, 0,
                                                      height, hipMemcpyDeviceToDevice, stream),
                                            width, height, stream);
    }
  }
}

TEST_CASE("Unit_hipMemcpy2DToArrayAsync_Negative_Parameters") {
  using namespace std::placeholders;

  const auto width = 32;
  const auto height = 32;
  const auto allocation_size = 2 * width * height * sizeof(int);

  const unsigned int flag = hipArrayDefault;

  constexpr auto InvalidStream = [] {
    StreamGuard sg(Streams::created);
    return sg.stream();
  };

  ArrayAllocGuard<int> array_alloc(make_hipExtent(width, height, 0), flag);
  LinearAllocGuard2D<int> device_alloc(width, height);
  LinearAllocGuard<int> host_alloc(LinearAllocs::hipHostMalloc, allocation_size);

  SECTION("Host to Array") {
    SECTION("dst == nullptr") {
      HIP_CHECK_ERROR(
          hipMemcpy2DToArrayAsync(nullptr, 0, 0, host_alloc.ptr(), 2 * width * sizeof(int),
                                  width * sizeof(int), height, hipMemcpyHostToDevice, nullptr),
          hipErrorInvalidHandle);
    }
    SECTION("src == nullptr") {
      HIP_CHECK_ERROR(
          hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, nullptr, 2 * width * sizeof(int),
                                  width * sizeof(int), height, hipMemcpyHostToDevice, nullptr),
          hipErrorInvalidValue);
    }
#if HT_NVIDIA  // EXSWHTEC-212
    SECTION("spitch < width") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, host_alloc.ptr(),
                                              width * sizeof(int) - 10, width * sizeof(int), height,
                                              hipMemcpyHostToDevice, nullptr),
                      hipErrorInvalidPitchValue);
    }
    SECTION("Offset + width/height overflows") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 1, 0, host_alloc.ptr(),
                                              2 * width * sizeof(int), width * sizeof(int), height,
                                              hipMemcpyHostToDevice, nullptr),
                      hipErrorInvalidValue);
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 1, host_alloc.ptr(),
                                              2 * width * sizeof(int), width * sizeof(int), height,
                                              hipMemcpyHostToDevice, nullptr),
                      hipErrorInvalidValue);
    }
    SECTION("Width/height overflows") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, host_alloc.ptr(),
                                              2 * width * sizeof(int), width * sizeof(int) + 1,
                                              height, hipMemcpyHostToDevice, nullptr),
                      hipErrorInvalidValue);
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, host_alloc.ptr(),
                                              2 * width * sizeof(int), width * sizeof(int),
                                              height + 1, hipMemcpyHostToDevice, nullptr),
                      hipErrorInvalidValue);
    }
    SECTION("Memcpy kind is invalid") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, host_alloc.ptr(),
                                              2 * width * sizeof(int), width * sizeof(int), height,
                                              static_cast<hipMemcpyKind>(-1), nullptr),
                      hipErrorInvalidMemcpyDirection);
    }
    SECTION("Invalid stream") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, host_alloc.ptr(),
                                              2 * width * sizeof(int), width * sizeof(int), height,
                                              hipMemcpyHostToDevice, InvalidStream()),
                      hipErrorContextIsDestroyed);
    }
#endif
  }
  SECTION("Device to Array") {
    SECTION("dst == nullptr") {
      HIP_CHECK_ERROR(
          hipMemcpy2DToArrayAsync(nullptr, 0, 0, device_alloc.ptr(), device_alloc.pitch(),
                                  width * sizeof(int), height, hipMemcpyDeviceToDevice, nullptr),
          hipErrorInvalidHandle);
    }
    SECTION("src == nullptr") {
      HIP_CHECK_ERROR(
          hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, nullptr, device_alloc.pitch(),
                                  width * sizeof(int), height, hipMemcpyDeviceToDevice, nullptr),
          hipErrorInvalidValue);
    }
#if HT_NVIDIA  // EXSWHTEC-212
    SECTION("spitch < width") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, device_alloc.ptr(),
                                              width * sizeof(int) - 10, width * sizeof(int), height,
                                              hipMemcpyDeviceToDevice, nullptr),
                      hipErrorInvalidPitchValue);
    }
    SECTION("Offset + width/height overflows") {
      HIP_CHECK_ERROR(
          hipMemcpy2DToArrayAsync(array_alloc.ptr(), 1, 0, device_alloc.ptr(), device_alloc.pitch(),
                                  width * sizeof(int), height, hipMemcpyDeviceToDevice, nullptr),
          hipErrorInvalidValue);
      HIP_CHECK_ERROR(
          hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 1, device_alloc.ptr(), device_alloc.pitch(),
                                  width * sizeof(int), height, hipMemcpyDeviceToDevice, nullptr),
          hipErrorInvalidValue);
    }
    SECTION("Width/height overflows") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, device_alloc.ptr(),
                                              device_alloc.pitch(), width * sizeof(int) + 1, height,
                                              hipMemcpyDeviceToDevice, nullptr),
                      hipErrorInvalidValue);
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, device_alloc.ptr(),
                                              device_alloc.pitch(), width * sizeof(int), height + 1,
                                              hipMemcpyDeviceToDevice, nullptr),
                      hipErrorInvalidValue);
    }
    SECTION("Memcpy kind is invalid") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, device_alloc.ptr(),
                                              device_alloc.pitch(), width * sizeof(int), height,
                                              static_cast<hipMemcpyKind>(-1), nullptr),
                      hipErrorInvalidMemcpyDirection);
    }
    SECTION("Invalid stream") {
      HIP_CHECK_ERROR(hipMemcpy2DToArrayAsync(array_alloc.ptr(), 0, 0, device_alloc.ptr(),
                                              device_alloc.pitch(), width * sizeof(int), height,
                                              hipMemcpyDeviceToDevice, InvalidStream()),
                      hipErrorContextIsDestroyed);
    }
#endif
  }
}
