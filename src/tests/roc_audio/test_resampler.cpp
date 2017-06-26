/*
 * Copyright (c) 2015 Mikhail Baranov
 * Copyright (c) 2015 Victor Gaydov
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <math.h>
#include <stdio.h>
#include <CppUTest/TestHarness.h>

#include "roc_audio/resampler.h"
#include "roc_config/config.h"
#include "roc_core/scoped_ptr.h"
#include "fft.h"
#include "test_stream_reader.h"
#include "roc_core/helpers.h"

namespace roc {
namespace test {

using namespace audio;

namespace {

enum { FrameSize = ROC_CONFIG_DEFAULT_RESAMPLER_FRAME_SAMPLES * 2 - 1 };

enum { OutSamples = FrameSize * 100 + 1, InSamples = OutSamples + (FrameSize * 3) };

} // namespace

TEST_GROUP(resampler) {
    TestStreamReader<InSamples> reader;

    core::ScopedPtr<Resampler> resampler;

    void setup() {
        resampler.reset(new Resampler(reader, default_buffer_composer(), FrameSize));
    }

    void expect_buffers(size_t num_buffers, size_t sz, int value) {
        read_buffers<InSamples>(*resampler, num_buffers, sz, value);
    }
};

TEST(resampler, invalid_scaling) {
    enum { InvalidScaling = FrameSize };

    CHECK(!resampler->set_scaling(InvalidScaling));
}

TEST(resampler, no_scaling_one_read) {
    CHECK(resampler->set_scaling(1.0f));

    reader.add(InSamples, 333);

    expect_buffers(1, OutSamples, 333);
}

TEST(resampler, no_scaling_multiple_reads) {
    CHECK(resampler->set_scaling(1.0f));

    for (int n = 0; n < InSamples; n++) {
        reader.add(1, n);
    }

    for (int n = 0; n < OutSamples; n++) {
        expect_buffers(1, 1, FrameSize + n);
    }
}

TEST(resampler, upscaling_twice) {
    CHECK(resampler->set_scaling(2.0f));
    double buff[1024*2];

    FILE *fout = fopen("/tmp/resampler.out", "w+");
    CHECK(fout);

    for (int n = 0; n < InSamples; n++) {
        const float s = sin(M_PI/4 * double(n));
        // const float s = sin(M_PI/4 * double(n)) + cos(M_PI/32 * double(n));
        // float s = 0;
        // if (n == 100)
            // s = 1;
        reader.add(1, s);
        if (n*2 < ROC_ARRAY_SIZE(buff)) {
            buff[n*2] = s;
            buff[n*2+1] = 0;
            fprintf(fout, "%f, ", s);
        }
    }
    FFT(buff, ROC_ARRAY_SIZE(buff)/2);
    fprintf(fout, "\n");
    for (size_t i = 0; i < ROC_ARRAY_SIZE(buff); i += 2) {
        fprintf(fout, "%f, ", buff[i]);
    }
    fprintf(fout, "\n");
    for (size_t i = 1; i < ROC_ARRAY_SIZE(buff); i += 2) {
        fprintf(fout, "%f, ", buff[i]);
    }
    fprintf(fout, "\n");

    fclose(fout);
}

} // namespace test
} // namespace roc
