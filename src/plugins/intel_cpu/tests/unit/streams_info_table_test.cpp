// Copyright (C) 2018-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>
#include <ie_system_conf.h>

#include <common_test_utils/test_common.hpp>

#include "cpu_map_scheduling.hpp"
#include "cpu_streams_calculation.hpp"

using namespace testing;
using namespace InferenceEngine;
using namespace ov;

namespace {

struct SchedulingCoreTypeTestCase {
    ov::hint::SchedulingCoreType input_type;
    std::vector<std::vector<int>> proc_type_table;
    std::vector<std::vector<int>> result_table;
    ov::hint::SchedulingCoreType output_type;
};

class SchedulingCoreTypeTests : public CommonTestUtils::TestsCommon,
                                public testing::WithParamInterface<std::tuple<SchedulingCoreTypeTestCase>> {
public:
    void SetUp() override {
        const auto& test_data = std::get<0>(GetParam());
        auto test_input_type = test_data.input_type;

        std::vector<std::vector<int>> test_result_table =
            ov::intel_cpu::apply_scheduling_core_type(test_input_type, test_data.proc_type_table);

        ASSERT_EQ(test_data.result_table, test_result_table);
        ASSERT_EQ(test_data.output_type, test_input_type);
    }
};

SchedulingCoreTypeTestCase _2sockets_ALL = {
    ov::hint::SchedulingCoreType::ANY_CORE,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    ov::hint::SchedulingCoreType::ANY_CORE,
};

SchedulingCoreTypeTestCase _2sockets_P_CORE_ONLY = {
    ov::hint::SchedulingCoreType::PCORE_ONLY,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    ov::hint::SchedulingCoreType::PCORE_ONLY,
};

SchedulingCoreTypeTestCase _2sockets_E_CORE_ONLY = {
    ov::hint::SchedulingCoreType::ECORE_ONLY,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    ov::hint::SchedulingCoreType::ANY_CORE,
    // ov::hint::scheduling_core_type returns ANY_CORE because the platform has no Ecores available to satisfy the
    // user's request.
};

SchedulingCoreTypeTestCase _1sockets_ALL = {
    ov::hint::SchedulingCoreType::ANY_CORE,
    {{20, 6, 8, 6}},
    {{20, 6, 8, 6}},
    ov::hint::SchedulingCoreType::ANY_CORE,
};

SchedulingCoreTypeTestCase _1sockets_P_CORE_ONLY = {
    ov::hint::SchedulingCoreType::PCORE_ONLY,
    {{20, 6, 8, 6}},
    {{12, 6, 0, 6}},
    ov::hint::SchedulingCoreType::PCORE_ONLY,
};

SchedulingCoreTypeTestCase _1sockets_P_CORE_ONLY_1 = {
    ov::hint::SchedulingCoreType::PCORE_ONLY,
    {{8, 0, 8, 0}},
    {{8, 0, 8, 0}},
    ov::hint::SchedulingCoreType::ANY_CORE,
    // ov::hint::scheduling_core_type returns ANY_CORE because the platform has no Pcore available to satisfy the
    // user's request.
};

SchedulingCoreTypeTestCase _1sockets_E_CORE_ONLY = {
    ov::hint::SchedulingCoreType::ECORE_ONLY,
    {{20, 6, 8, 6}},
    {{8, 0, 8, 0}},
    ov::hint::SchedulingCoreType::ECORE_ONLY,
};

TEST_P(SchedulingCoreTypeTests, SchedulingCoreType) {}

INSTANTIATE_TEST_SUITE_P(SchedulingCoreTypeTable,
                         SchedulingCoreTypeTests,
                         testing::Values(_2sockets_ALL,
                                         _2sockets_P_CORE_ONLY,
                                         _2sockets_E_CORE_ONLY,
                                         _1sockets_ALL,
                                         _1sockets_P_CORE_ONLY,
                                         _1sockets_P_CORE_ONLY_1,
                                         _1sockets_E_CORE_ONLY));

struct UseHTTestCase {
    bool input_ht_value;
    bool input_ht_changed;
    std::string input_pm_hint;
    std::vector<std::vector<int>> proc_type_table;
    std::vector<std::vector<int>> result_table;
    bool output_ht_value;
};

class UseHTTests : public CommonTestUtils::TestsCommon, public testing::WithParamInterface<std::tuple<UseHTTestCase>> {
public:
    void SetUp() override {
        auto test_data = std::get<0>(GetParam());

        std::vector<std::vector<int>> test_result_table =
            ov::intel_cpu::apply_hyper_threading(test_data.input_ht_value,
                                                 test_data.input_ht_changed,
                                                 test_data.input_pm_hint,
                                                 test_data.proc_type_table);

        ASSERT_EQ(test_data.result_table, test_result_table);
        ASSERT_EQ(test_data.input_ht_value, test_data.output_ht_value);
    }
};

UseHTTestCase _2sockets_false_latency = {
    false,
    true,
    "LATENCY",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    false,
};

UseHTTestCase _2sockets_false_throughput = {
    false,
    true,
    "THROUGHPUT",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    false,
};

UseHTTestCase _2sockets_true_latency = {
    true,
    true,
    "LATENCY",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    true,
};

UseHTTestCase _2sockets_true_throughput = {
    true,
    true,
    "THROUGHPUT",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    true,
};

UseHTTestCase _2sockets_default_1_latency = {
    false,
    false,
    "LATENCY",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    false,
};

UseHTTestCase _2sockets_default_1_throughput = {
    false,
    false,
    "THROUGHPUT",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    false,
};

UseHTTestCase _2sockets_default_2_latency = {
    true,
    false,
    "LATENCY",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    false,
};

UseHTTestCase _2sockets_default_2_throughput = {
    true,
    false,
    "THROUGHPUT",
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    false,
};

UseHTTestCase _1sockets_1_false_latency = {
    false,
    true,
    "LATENCY",
    {{20, 6, 8, 6}},
    {{14, 6, 8, 0}},
    false,
};

UseHTTestCase _1sockets_1_false_throughput = {
    false,
    true,
    "THROUGHPUT",
    {{20, 6, 8, 6}},
    {{14, 6, 8, 0}},
    false,
};

UseHTTestCase _1sockets_1_true_latency = {
    true,
    true,
    "LATENCY",
    {{20, 6, 8, 6}},
    {{20, 6, 8, 6}},
    true,
};

UseHTTestCase _1sockets_1_true_throughput = {
    true,
    true,
    "THROUGHPUT",
    {{20, 6, 8, 6}},
    {{20, 6, 8, 6}},
    true,
};

UseHTTestCase _1sockets_1_default_1_latency = {
    false,
    false,
    "LATENCY",
    {{20, 6, 8, 6}},
    {{14, 6, 8, 0}},
    false,
};

UseHTTestCase _1sockets_1_default_1_throughput = {
    false,
    false,
    "THROUGHPUT",
    {{20, 6, 8, 6}},
    {{20, 6, 8, 6}},
    true,
};

UseHTTestCase _1sockets_1_default_2_latency = {
    true,
    false,
    "LATENCY",
    {{20, 6, 8, 6}},
    {{14, 6, 8, 0}},
    false,
};

UseHTTestCase _1sockets_1_default_2_throughput = {
    true,
    false,
    "THROUGHPUT",
    {{20, 6, 8, 6}},
    {{20, 6, 8, 6}},
    true,
};

UseHTTestCase _1sockets_2_false_latency = {
    false,
    true,
    "LATENCY",
    {{12, 6, 0, 6}},
    {{6, 6, 0, 0}},
    false,
};

UseHTTestCase _1sockets_2_false_throughput = {
    false,
    true,
    "THROUGHPUT",
    {{12, 6, 0, 6}},
    {{6, 6, 0, 0}},
    false,
};

UseHTTestCase _1sockets_2_true_latency = {
    true,
    true,
    "LATENCY",
    {{12, 6, 0, 6}},
    {{12, 6, 0, 6}},
    true,
};

UseHTTestCase _1sockets_2_true_throughput = {
    true,
    true,
    "THROUGHPUT",
    {{12, 6, 0, 6}},
    {{12, 6, 0, 6}},
    true,
};

UseHTTestCase _1sockets_2_default_1_latency = {
    false,
    false,
    "LATENCY",
    {{12, 6, 0, 6}},
    {{6, 6, 0, 0}},
    false,
};

UseHTTestCase _1sockets_2_default_1_throughput = {
    false,
    false,
    "THROUGHPUT",
    {{12, 6, 0, 6}},
    {{12, 6, 0, 6}},
    true,
};

UseHTTestCase _1sockets_2_default_2_latency = {
    true,
    false,
    "LATENCY",
    {{12, 6, 0, 6}},
    {{6, 6, 0, 0}},
    false,
};

UseHTTestCase _1sockets_2_default_2_throughput = {
    true,
    false,
    "THROUGHPUT",
    {{12, 6, 0, 6}},
    {{12, 6, 0, 6}},
    true,
};

TEST_P(UseHTTests, UseHT) {}

INSTANTIATE_TEST_SUITE_P(UseHTTable,
                         UseHTTests,
                         testing::Values(_2sockets_false_latency,
                                         _2sockets_true_latency,
                                         _2sockets_default_1_latency,
                                         _2sockets_default_2_latency,
                                         _1sockets_1_false_latency,
                                         _1sockets_1_true_latency,
                                         _1sockets_1_default_1_latency,
                                         _1sockets_1_default_2_latency,
                                         _1sockets_2_false_latency,
                                         _1sockets_2_true_latency,
                                         _1sockets_2_default_1_latency,
                                         _1sockets_2_default_2_latency,
                                         _2sockets_false_throughput,
                                         _2sockets_true_throughput,
                                         _2sockets_default_1_throughput,
                                         _2sockets_default_2_throughput,
                                         _1sockets_1_false_throughput,
                                         _1sockets_1_true_throughput,
                                         _1sockets_1_default_1_throughput,
                                         _1sockets_1_default_2_throughput,
                                         _1sockets_2_false_throughput,
                                         _1sockets_2_true_throughput,
                                         _1sockets_2_default_1_throughput,
                                         _1sockets_2_default_2_throughput));

struct StreamsCalculationTestCase {
    int input_streams;
    int input_threads;
    int input_infer_requests;
    int model_prefer_threads;
    std::vector<std::vector<int>> proc_type_table;
    std::vector<std::vector<int>> stream_info_table;
};

class StreamsCalculationTests : public CommonTestUtils::TestsCommon,
                                public testing::WithParamInterface<std::tuple<StreamsCalculationTestCase>> {
public:
    void SetUp() override {
        const auto& test_data = std::get<0>(GetParam());

        std::vector<std::vector<int>> test_stream_info_table =
            ov::intel_cpu::get_streams_info_table(test_data.input_streams,
                                                  test_data.input_threads,
                                                  test_data.input_infer_requests,
                                                  test_data.model_prefer_threads,
                                                  test_data.proc_type_table);

        ASSERT_EQ(test_data.stream_info_table, test_stream_info_table);
    }
};

StreamsCalculationTestCase _2sockets_104cores_latency_1 = {
    1,
    0,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{1, MAIN_CORE_PROC, 208}},
};

StreamsCalculationTestCase _2sockets_104cores_latency_2 = {
    1,
    20,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{1, MAIN_CORE_PROC, 20}},
};

StreamsCalculationTestCase _2sockets_104cores_latency_3 = {
    1,
    20,
    5,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{1, MAIN_CORE_PROC, 20}},
};

StreamsCalculationTestCase _2sockets_104cores_latency_4 = {
    1,
    208,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{1, MAIN_CORE_PROC, 208}},
};

StreamsCalculationTestCase _2sockets_104cores_latency_5 = {
    1,
    0,
    0,
    0,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{1, MAIN_CORE_PROC, 104}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_1 = {
    0,
    0,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{52, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_2 = {
    2,
    0,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{2, MAIN_CORE_PROC, 104}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_3 = {
    0,
    20,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{5, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_4 = {
    2,
    20,
    0,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{2, MAIN_CORE_PROC, 10}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_5 = {
    0,
    0,
    0,
    1,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{208, MAIN_CORE_PROC, 1}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_6 = {
    0,
    0,
    0,
    2,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{104, MAIN_CORE_PROC, 2}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_7 = {
    0,
    0,
    0,
    8,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{26, MAIN_CORE_PROC, 8}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_8 = {
    0,
    40,
    0,
    8,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{5, MAIN_CORE_PROC, 8}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_9 = {
    5,
    20,
    2,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{2, MAIN_CORE_PROC, 10}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_10 = {
    0,
    0,
    2,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{2, MAIN_CORE_PROC, 104}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_11 = {
    2,
    0,
    5,
    0,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{2, MAIN_CORE_PROC, 104}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_12 = {
    0,
    0,
    2,
    2,
    {{208, 104, 0, 104}, {104, 52, 0, 52}, {104, 52, 0, 52}},
    {{2, MAIN_CORE_PROC, 104}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_13 = {
    0,
    0,
    0,
    0,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{26, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_14 = {
    2,
    0,
    0,
    0,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{2, MAIN_CORE_PROC, 52}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_15 = {
    0,
    0,
    0,
    1,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{104, MAIN_CORE_PROC, 1}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_16 = {
    0,
    0,
    0,
    2,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{52, MAIN_CORE_PROC, 2}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_17 = {
    0,
    0,
    0,
    8,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{13, MAIN_CORE_PROC, 8}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_18 = {
    0,
    0,
    2,
    0,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{2, MAIN_CORE_PROC, 52}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_19 = {
    2,
    0,
    5,
    0,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{2, MAIN_CORE_PROC, 52}},
};

StreamsCalculationTestCase _2sockets_104cores_tput_20 = {
    0,
    0,
    2,
    2,
    {{104, 104, 0, 0}, {52, 52, 0, 0}, {52, 52, 0, 0}},
    {{2, MAIN_CORE_PROC, 52}},
};

StreamsCalculationTestCase _2sockets_48cores_latency_1 = {
    1,
    0,
    0,
    0,
    {{48, 48, 0, 0}, {24, 24, 0, 0}, {24, 24, 0, 0}},
    {{1, MAIN_CORE_PROC, 48}},
};

StreamsCalculationTestCase _2sockets_48cores_tput_1 = {
    0,
    0,
    0,
    0,
    {{48, 48, 0, 0}, {24, 24, 0, 0}, {24, 24, 0, 0}},
    {{12, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _2sockets_48cores_tput_2 = {
    100,
    0,
    0,
    0,
    {{48, 48, 0, 0}, {24, 24, 0, 0}, {24, 24, 0, 0}},
    {{48, MAIN_CORE_PROC, 1}},
};

StreamsCalculationTestCase _2sockets_48cores_tput_3 = {
    0,
    100,
    0,
    0,
    {{48, 48, 0, 0}, {24, 24, 0, 0}, {24, 24, 0, 0}},
    {{12, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _2sockets_48cores_tput_4 = {
    2,
    20,
    0,
    1,
    {{48, 48, 0, 0}, {24, 24, 0, 0}, {24, 24, 0, 0}},
    {{2, MAIN_CORE_PROC, 10}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_1 = {
    1,
    0,
    0,
    0,
    {{14, 6, 8, 0}},
    {{1, ALL_PROC, 14}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_2 = {
    1,
    10,
    0,
    0,
    {{20, 6, 8, 6}},
    {{1, ALL_PROC, 10}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_3 = {
    1,
    0,
    0,
    6,
    {{20, 6, 8, 6}},
    {{1, MAIN_CORE_PROC, 12}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_4 = {
    1,
    0,
    0,
    14,
    {{20, 6, 8, 6}},
    {{1, ALL_PROC, 20}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 8}, {0, HYPER_THREADING_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_5 = {
    1,
    0,
    2,
    14,
    {{20, 6, 8, 6}},
    {{1, ALL_PROC, 20}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 8}, {0, HYPER_THREADING_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_6 = {
    1,
    0,
    0,
    0,
    {{20, 6, 8, 6}},
    {{1, ALL_PROC, 20}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 8}, {0, HYPER_THREADING_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_7 = {
    1,
    0,
    0,
    6,
    {{14, 6, 8, 0}},
    {{1, MAIN_CORE_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_8 = {
    1,
    0,
    0,
    14,
    {{14, 6, 8, 0}},
    {{1, ALL_PROC, 14}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_14cores_latency_9 = {
    1,
    0,
    2,
    14,
    {{14, 6, 8, 0}},
    {{1, ALL_PROC, 14}, {0, MAIN_CORE_PROC, 6}, {0, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_1 = {
    0,
    0,
    0,
    0,
    {{20, 6, 8, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, EFFICIENT_CORE_PROC, 3}, {2, HYPER_THREADING_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_2 = {
    2,
    0,
    0,
    0,
    {{20, 6, 8, 6}},
    {{1, MAIN_CORE_PROC, 6}, {1, EFFICIENT_CORE_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_3 = {
    4,
    0,
    0,
    0,
    {{20, 6, 8, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, EFFICIENT_CORE_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_4 = {
    0,
    12,
    0,
    0,
    {{20, 6, 8, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, EFFICIENT_CORE_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_5 = {
    0,
    0,
    0,
    1,
    {{20, 6, 8, 6}},
    {{6, MAIN_CORE_PROC, 1}, {4, EFFICIENT_CORE_PROC, 2}, {6, HYPER_THREADING_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_6 = {
    0,
    0,
    0,
    2,
    {{20, 6, 8, 6}},
    {{3, MAIN_CORE_PROC, 2}, {4, EFFICIENT_CORE_PROC, 2}, {3, HYPER_THREADING_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_7 = {
    100,
    0,
    0,
    0,
    {{20, 6, 8, 6}},
    {{6, MAIN_CORE_PROC, 1}, {8, EFFICIENT_CORE_PROC, 1}, {6, HYPER_THREADING_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_8 = {
    0,
    100,
    0,
    0,
    {{20, 6, 8, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, EFFICIENT_CORE_PROC, 3}, {2, HYPER_THREADING_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_9 = {
    4,
    0,
    8,
    0,
    {{20, 6, 8, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, EFFICIENT_CORE_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_10 = {
    6,
    0,
    4,
    0,
    {{20, 6, 8, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, EFFICIENT_CORE_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_11 = {
    0,
    0,
    2,
    0,
    {{20, 6, 8, 6}},
    {{1, MAIN_CORE_PROC, 6}, {1, EFFICIENT_CORE_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_12 = {
    0,
    0,
    2,
    2,
    {{20, 6, 8, 6}},
    {{1, MAIN_CORE_PROC, 6}, {1, EFFICIENT_CORE_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_13 = {
    0,
    1,
    0,
    1,
    {{20, 6, 8, 6}},
    {{1, MAIN_CORE_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_14 = {
    0,
    9,
    0,
    1,
    {{20, 6, 8, 6}},
    {{6, MAIN_CORE_PROC, 1}, {1, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_15 = {
    0,
    12,
    0,
    1,
    {{20, 6, 8, 6}},
    {{6, MAIN_CORE_PROC, 1}, {3, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_14cores_tput_16 = {
    0,
    15,
    0,
    1,
    {{20, 6, 8, 6}},
    {{6, MAIN_CORE_PROC, 1}, {4, EFFICIENT_CORE_PROC, 2}, {1, HYPER_THREADING_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_1 = {
    1,
    0,
    0,
    0,
    {{12, 2, 8, 2}},
    {{1, ALL_PROC, 12}, {0, MAIN_CORE_PROC, 2}, {0, EFFICIENT_CORE_PROC, 8}, {0, HYPER_THREADING_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_2 = {
    1,
    8,
    0,
    0,
    {{12, 2, 8, 2}},
    {{1, ALL_PROC, 8}, {0, MAIN_CORE_PROC, 2}, {0, EFFICIENT_CORE_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_3 = {
    1,
    0,
    0,
    2,
    {{12, 2, 8, 2}},
    {{1, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_4 = {
    1,
    0,
    0,
    10,
    {{12, 2, 8, 2}},
    {{1, ALL_PROC, 12}, {0, MAIN_CORE_PROC, 2}, {0, EFFICIENT_CORE_PROC, 8}, {0, HYPER_THREADING_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_5 = {
    1,
    0,
    0,
    0,
    {{10, 2, 8, 0}},
    {{1, ALL_PROC, 10}, {0, MAIN_CORE_PROC, 2}, {0, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_6 = {
    1,
    0,
    0,
    2,
    {{10, 2, 8, 0}},
    {{1, MAIN_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_latency_7 = {
    1,
    0,
    0,
    10,
    {{10, 2, 8, 0}},
    {{1, ALL_PROC, 10}, {0, MAIN_CORE_PROC, 2}, {0, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_10cores_tput_1 = {
    0,
    0,
    0,
    0,
    {{12, 2, 8, 2}},
    {{1, MAIN_CORE_PROC, 2}, {4, EFFICIENT_CORE_PROC, 2}, {1, HYPER_THREADING_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_tput_2 = {
    2,
    0,
    0,
    0,
    {{12, 2, 8, 2}},
    {{1, MAIN_CORE_PROC, 2}, {1, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_tput_3 = {
    4,
    0,
    0,
    0,
    {{12, 2, 8, 2}},
    {{1, MAIN_CORE_PROC, 2}, {3, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_tput_4 = {
    0,
    6,
    0,
    0,
    {{12, 2, 8, 2}},
    {{1, MAIN_CORE_PROC, 2}, {2, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_10cores_tput_5 = {
    0,
    0,
    0,
    1,
    {{12, 2, 8, 2}},
    {{2, MAIN_CORE_PROC, 1}, {4, EFFICIENT_CORE_PROC, 2}, {2, HYPER_THREADING_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_10cores_tput_6 = {
    0,
    0,
    0,
    2,
    {{12, 2, 8, 2}},
    {{1, MAIN_CORE_PROC, 2}, {4, EFFICIENT_CORE_PROC, 2}, {1, HYPER_THREADING_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_1 = {
    1,
    0,
    0,
    0,
    {{12, 4, 4, 4}},
    {{1, ALL_PROC, 12}, {0, MAIN_CORE_PROC, 4}, {0, EFFICIENT_CORE_PROC, 4}, {0, HYPER_THREADING_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_2 = {
    1,
    100,
    0,
    0,
    {{12, 4, 4, 4}},
    {{1, ALL_PROC, 12}, {0, MAIN_CORE_PROC, 4}, {0, EFFICIENT_CORE_PROC, 4}, {0, HYPER_THREADING_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_3 = {
    1,
    0,
    0,
    4,
    {{12, 4, 4, 4}},
    {{1, MAIN_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_4 = {
    1,
    0,
    0,
    8,
    {{12, 4, 4, 4}},
    {{1, ALL_PROC, 12}, {0, MAIN_CORE_PROC, 4}, {0, EFFICIENT_CORE_PROC, 4}, {0, HYPER_THREADING_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_5 = {
    1,
    0,
    0,
    0,
    {{8, 4, 4, 0}},
    {{1, ALL_PROC, 8}, {0, MAIN_CORE_PROC, 4}, {0, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_6 = {
    1,
    0,
    0,
    4,
    {{8, 4, 4, 0}},
    {{1, MAIN_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_latency_7 = {
    1,
    0,
    0,
    8,
    {{8, 4, 4, 0}},
    {{1, ALL_PROC, 8}, {0, MAIN_CORE_PROC, 4}, {0, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_1 = {
    0,
    0,
    0,
    0,
    {{12, 4, 4, 4}},
    {{1, MAIN_CORE_PROC, 4}, {1, EFFICIENT_CORE_PROC, 4}, {1, HYPER_THREADING_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_2 = {
    2,
    0,
    0,
    0,
    {{12, 4, 4, 4}},
    {{1, MAIN_CORE_PROC, 4}, {1, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_3 = {
    4,
    0,
    0,
    0,
    {{12, 4, 4, 4}},
    {{2, MAIN_CORE_PROC, 2}, {2, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_4 = {
    6,
    0,
    0,
    0,
    {{12, 4, 4, 4}},
    {{2, MAIN_CORE_PROC, 2}, {2, EFFICIENT_CORE_PROC, 2}, {2, HYPER_THREADING_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_5 = {
    0,
    6,
    0,
    0,
    {{12, 4, 4, 4}},
    {{2, MAIN_CORE_PROC, 2}, {1, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_6 = {
    0,
    8,
    0,
    0,
    {{12, 4, 4, 4}},
    {{2, MAIN_CORE_PROC, 2}, {2, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_7 = {
    0,
    0,
    0,
    1,
    {{12, 4, 4, 4}},
    {{4, MAIN_CORE_PROC, 1}, {2, EFFICIENT_CORE_PROC, 2}, {4, HYPER_THREADING_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_8cores_tput_8 = {
    0,
    0,
    0,
    0,
    {{8, 4, 4, 0}},
    {{2, MAIN_CORE_PROC, 2}, {2, EFFICIENT_CORE_PROC, 2}},
};

StreamsCalculationTestCase _1sockets_6cores_latency_1 = {
    1,
    0,
    0,
    0,
    {{12, 6, 0, 6}},
    {{1, MAIN_CORE_PROC, 12}},
};

StreamsCalculationTestCase _1sockets_6cores_latency_2 = {
    1,
    100,
    0,
    0,
    {{12, 6, 0, 6}},
    {{1, MAIN_CORE_PROC, 12}},
};

StreamsCalculationTestCase _1sockets_6cores_latency_3 = {
    1,
    0,
    0,
    0,
    {{6, 6, 0, 0}},
    {{1, MAIN_CORE_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_6cores_tput_1 = {
    0,
    0,
    0,
    0,
    {{12, 6, 0, 6}},
    {{2, MAIN_CORE_PROC, 3}, {2, HYPER_THREADING_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_6cores_tput_2 = {
    2,
    0,
    0,
    0,
    {{12, 6, 0, 6}},
    {{1, MAIN_CORE_PROC, 6}, {1, HYPER_THREADING_PROC, 6}},
};

StreamsCalculationTestCase _1sockets_6cores_tput_3 = {
    0,
    8,
    0,
    0,
    {{12, 6, 0, 6}},
    {{2, MAIN_CORE_PROC, 3}},
};

StreamsCalculationTestCase _1sockets_6cores_tput_4 = {
    0,
    0,
    0,
    1,
    {{12, 6, 0, 6}},
    {{6, MAIN_CORE_PROC, 1}, {6, HYPER_THREADING_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_ecores_latency_1 = {
    1,
    0,
    0,
    0,
    {{16, 0, 16, 0}},
    {{1, EFFICIENT_CORE_PROC, 16}},
};

StreamsCalculationTestCase _1sockets_ecores_latency_2 = {
    1,
    4,
    0,
    0,
    {{16, 0, 16, 0}},
    {{1, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_ecores_latency_3 = {
    1,
    0,
    4,
    0,
    {{16, 0, 16, 0}},
    {{1, EFFICIENT_CORE_PROC, 16}},
};

StreamsCalculationTestCase _1sockets_ecores_latency_4 = {
    1,
    0,
    0,
    4,
    {{16, 0, 16, 0}},
    {{1, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_ecores_tput_1 = {
    0,
    0,
    0,
    1,
    {{16, 0, 16, 0}},
    {{16, EFFICIENT_CORE_PROC, 1}},
};

StreamsCalculationTestCase _1sockets_ecores_tput_2 = {
    0,
    0,
    0,
    4,
    {{16, 0, 16, 0}},
    {{4, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_ecores_tput_3 = {
    2,
    0,
    0,
    0,
    {{16, 0, 16, 0}},
    {{2, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_ecores_tput_4 = {
    8,
    0,
    4,
    0,
    {{16, 0, 16, 0}},
    {{4, EFFICIENT_CORE_PROC, 4}},
};

StreamsCalculationTestCase _1sockets_ecores_tput_5 = {
    2,
    0,
    0,
    4,
    {{16, 0, 16, 0}},
    {{2, EFFICIENT_CORE_PROC, 8}},
};

StreamsCalculationTestCase _1sockets_mock_tput_1 = {
    0,
    15,
    0,
    1,
    {{20, 6, 7, 6}},
    {{6, MAIN_CORE_PROC, 1}, {3, EFFICIENT_CORE_PROC, 2}, {3, HYPER_THREADING_PROC, 1}},
};

TEST_P(StreamsCalculationTests, StreamsCalculation) {}

INSTANTIATE_TEST_SUITE_P(StreamsInfoTable,
                         StreamsCalculationTests,
                         testing::Values(_2sockets_104cores_latency_1,
                                         _2sockets_104cores_latency_2,
                                         _2sockets_104cores_latency_3,
                                         _2sockets_104cores_latency_4,
                                         _2sockets_104cores_latency_5,
                                         _2sockets_104cores_tput_1,
                                         _2sockets_104cores_tput_2,
                                         _2sockets_104cores_tput_3,
                                         _2sockets_104cores_tput_4,
                                         _2sockets_104cores_tput_5,
                                         _2sockets_104cores_tput_6,
                                         _2sockets_104cores_tput_7,
                                         _2sockets_104cores_tput_8,
                                         _2sockets_104cores_tput_9,
                                         _2sockets_104cores_tput_10,
                                         _2sockets_104cores_tput_11,
                                         _2sockets_104cores_tput_12,
                                         _2sockets_104cores_tput_13,
                                         _2sockets_104cores_tput_14,
                                         _2sockets_104cores_tput_15,
                                         _2sockets_104cores_tput_16,
                                         _2sockets_104cores_tput_17,
                                         _2sockets_104cores_tput_18,
                                         _2sockets_104cores_tput_19,
                                         _2sockets_104cores_tput_20,
                                         _2sockets_48cores_latency_1,
                                         _2sockets_48cores_tput_1,
                                         _2sockets_48cores_tput_2,
                                         _2sockets_48cores_tput_3,
                                         _2sockets_48cores_tput_4,
                                         _1sockets_14cores_latency_1,
                                         _1sockets_14cores_latency_2,
                                         _1sockets_14cores_latency_3,
                                         _1sockets_14cores_latency_4,
                                         _1sockets_14cores_latency_5,
                                         _1sockets_14cores_latency_6,
                                         _1sockets_14cores_latency_7,
                                         _1sockets_14cores_latency_8,
                                         _1sockets_14cores_latency_9,
                                         _1sockets_14cores_tput_1,
                                         _1sockets_14cores_tput_2,
                                         _1sockets_14cores_tput_3,
                                         _1sockets_14cores_tput_4,
                                         _1sockets_14cores_tput_5,
                                         _1sockets_14cores_tput_6,
                                         _1sockets_14cores_tput_7,
                                         _1sockets_14cores_tput_8,
                                         _1sockets_14cores_tput_9,
                                         _1sockets_14cores_tput_10,
                                         _1sockets_14cores_tput_11,
                                         _1sockets_14cores_tput_12,
                                         _1sockets_14cores_tput_13,
                                         _1sockets_14cores_tput_14,
                                         _1sockets_14cores_tput_15,
                                         _1sockets_14cores_tput_16,
                                         _1sockets_10cores_latency_1,
                                         _1sockets_10cores_latency_2,
                                         _1sockets_10cores_latency_3,
                                         _1sockets_10cores_latency_4,
                                         _1sockets_10cores_latency_5,
                                         _1sockets_10cores_latency_6,
                                         _1sockets_10cores_latency_7,
                                         _1sockets_10cores_tput_1,
                                         _1sockets_10cores_tput_2,
                                         _1sockets_10cores_tput_3,
                                         _1sockets_10cores_tput_4,
                                         _1sockets_10cores_tput_5,
                                         _1sockets_10cores_tput_6,
                                         _1sockets_8cores_latency_1,
                                         _1sockets_8cores_latency_2,
                                         _1sockets_8cores_latency_3,
                                         _1sockets_8cores_latency_4,
                                         _1sockets_8cores_latency_5,
                                         _1sockets_8cores_latency_6,
                                         _1sockets_8cores_latency_7,
                                         _1sockets_8cores_tput_1,
                                         _1sockets_8cores_tput_2,
                                         _1sockets_8cores_tput_3,
                                         _1sockets_8cores_tput_4,
                                         _1sockets_8cores_tput_5,
                                         _1sockets_8cores_tput_6,
                                         _1sockets_8cores_tput_7,
                                         _1sockets_8cores_tput_8,
                                         _1sockets_6cores_latency_1,
                                         _1sockets_6cores_latency_2,
                                         _1sockets_6cores_latency_3,
                                         _1sockets_6cores_tput_1,
                                         _1sockets_6cores_tput_2,
                                         _1sockets_6cores_tput_3,
                                         _1sockets_6cores_tput_4,
                                         _1sockets_ecores_latency_1,
                                         _1sockets_ecores_latency_2,
                                         _1sockets_ecores_latency_3,
                                         _1sockets_ecores_latency_4,
                                         _1sockets_ecores_tput_1,
                                         _1sockets_ecores_tput_2,
                                         _1sockets_ecores_tput_3,
                                         _1sockets_ecores_tput_4,
                                         _1sockets_ecores_tput_5,
                                         _1sockets_mock_tput_1));

}  // namespace