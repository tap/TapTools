/// @file
/// @brief      The kernel tests run as a host runs the kernels: flush-to-zero set for every test case.
/// @details    The oversampling chain's Butterworth stage (taptools/butterworth.h) states the host's
///             flush-to-zero as a debug-build precondition (tap/dsp/subnormals.h), so a Debug build of
///             this battery stops at the first oversampled kernel unless the thread that runs it
///             flushes subnormals. This listener sets the bits for each test case and restores the
///             register after it (tools/support/flush_to_zero.h), the Catch2 counterpart of DspTap's
///             flush_to_zero_fixture (tests/support/iir_support.h). Catch2 runs every test case on
///             the thread that calls these hooks; a test that runs a recursive kernel on a thread of
///             its own sets the scope there itself.
/// @author     Timothy Place
// SPDX-License-Identifier: MIT
// Copyright 2026 Timothy Place and the TapTools contributors

#include <optional>

#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

#include "../../tools/support/flush_to_zero.h"

namespace {

    class flush_to_zero_listener : public Catch::EventListenerBase {
      public:
        using Catch::EventListenerBase::EventListenerBase;

        void testCaseStarting(const Catch::TestCaseInfo& /*info*/) override { m_scope.emplace(); }
        void testCaseEnded(const Catch::TestCaseStats& /*stats*/) override { m_scope.reset(); }

      private:
        std::optional<tap::tools::support::flush_to_zero_scope> m_scope;
    };

} // namespace

CATCH_REGISTER_LISTENER(flush_to_zero_listener)
