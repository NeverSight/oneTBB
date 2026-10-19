/*
    Copyright (c) 2026 NeverSight contributors
    SPDX-License-Identifier: Apache-2.0
*/

#include "common/test.h"
#include "oneapi/tbb/tbb_allocator.h"
#include "oneapi/tbb/cache_aligned_allocator.h"
#include "oneapi/tbb/global_control.h"
#include "oneapi/tbb/parallel_for.h"
#include <cstdio>
#include <atomic>
#include <cstdint>

TEST_CASE("Process allocator remains selected with a discoverable tbbmalloc library") {
    // Build the real sidecar beside tbb: absence must not make this test pass.
    auto* library = std::fopen(AVAILABLE_MALLOC_LIBRARY, "rb");
    REQUIRE(library != nullptr);
    std::fclose(library);
    using allocator = oneapi::tbb::tbb_allocator<std::uint64_t>;
    REQUIRE(allocator::allocator_type() == allocator::standard);
    allocator memory;
    auto* values = memory.allocate(127);
    oneapi::tbb::cache_aligned_allocator<std::uint64_t> aligned;
    auto* aligned_values = aligned.allocate(127);
    oneapi::tbb::global_control limit(oneapi::tbb::global_control::max_allowed_parallelism, 4);
    std::atomic<unsigned> completed{0};
    oneapi::tbb::parallel_for(0u, 127u, [&](unsigned i) {
        values[i] = i * i;
        aligned_values[i] = values[i] + 1;
        ++completed;
    });
    REQUIRE(completed == 127);
    for (unsigned i = 0; i != 127; ++i)
        REQUIRE(aligned_values[i] == i * i + 1);
    aligned.deallocate(aligned_values, 127);
    memory.deallocate(values, 127);
    REQUIRE(allocator::allocator_type() == allocator::standard);
}
