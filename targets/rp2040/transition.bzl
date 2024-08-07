# Copyright 2024 The Pigweed Authors
#
# Licensed under the Apache License, Version 2.0 (the "License"); you may not
# use this file except in compliance with the License. You may obtain a copy of
# the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
# WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
# License for the specific language governing permissions and limitations under
# the License.

"""Bazel transitions for the rp2xxx series."""

load("//pw_build:merge_flags.bzl", "merge_flags_for_transition_impl", "merge_flags_for_transition_outputs")
load("//third_party/freertos:flags.bzl", "FREERTOS_FLAGS")

# LINT.IfChange
# Typical RP2040 pw_system backends and other platform configuration flags.
RP2_SYSTEM_FLAGS = FREERTOS_FLAGS | {
    "@freertos//:freertos_config": str(Label("//targets/rp2040:freertos_config")),
    "@pico-sdk//bazel/config:PICO_CLIB": "llvm_libc",
    "@pico-sdk//bazel/config:PICO_STDIO_UART": True,
    "@pico-sdk//bazel/config:PICO_STDIO_USB": True,
    "@pico-sdk//bazel/config:PICO_TOOLCHAIN": "clang",
    "@pigweed//pw_assert:assert_backend": str(Label("//pw_assert_trap")),
    "@pigweed//pw_assert:assert_backend_impl": str(Label("//pw_assert_trap:impl")),
    "@pigweed//pw_assert:check_backend": str(Label("//pw_assert_trap")),
    "@pigweed//pw_assert:check_backend_impl": str(Label("//pw_assert_trap:impl")),
    "@pigweed//pw_build:default_module_config": str(Label("//targets/rp2040:pigweed_module_config")),
    "@pigweed//pw_cpu_exception:entry_backend": str(Label("//pw_cpu_exception_cortex_m:cpu_exception")),
    "@pigweed//pw_cpu_exception:entry_backend_impl": str(Label("//pw_cpu_exception_cortex_m:cpu_exception_impl")),
    "@pigweed//pw_cpu_exception:handler_backend": str(Label("//pw_cpu_exception:basic_handler")),
    "@pigweed//pw_cpu_exception:support_backend": str(Label("//pw_cpu_exception_cortex_m:support")),
    "@pigweed//pw_interrupt:backend": str(Label("//pw_interrupt_cortex_m:context")),
    "@pigweed//pw_log:backend": str(Label("//pw_log_tokenized")),
    "@pigweed//pw_log:backend_impl": str(Label("//pw_log_tokenized:impl")),
    "@pigweed//pw_log_tokenized:handler_backend": str(Label("//pw_system:log_backend")),
    "@pigweed//pw_sys_io:backend": str(Label("//pw_sys_io_rp2040")),
    "@pigweed//pw_system:device_handler_backend": str(Label("//targets/rp2040:device_handler")),
    "@pigweed//pw_system:extra_platform_libs": str(Label("//targets/rp2040:extra_platform_libs")),
    "@pigweed//pw_toolchain:cortex-m_toolchain_kind": "clang",
    "@pigweed//pw_trace:backend": str(Label("//pw_trace_tokenized:pw_trace_tokenized")),
    "@pigweed//pw_unit_test:backend": str(Label("//pw_unit_test:light")),
    "@pigweed//pw_unit_test:main": str(Label("//targets/rp2040:unit_test_app")),
}
# LINT.ThenChange(//.bazelrc)

# Additional flags specific to the upstream Pigweed RP2040 platform.
_rp2040_flags = {
    "//command_line_option:platforms": str(Label("//targets/rp2040:rp2040")),
}

_rp2350_flags = {
    "//command_line_option:platforms": str(Label("//targets/rp2040:rp2350")),
}

def _rp2_transition(device_specific_flags):
    def _rp2_transition_impl(settings, attr):
        # buildifier: disable=unused-variable
        _ignore = settings, attr
        return merge_flags_for_transition_impl(
            base = RP2_SYSTEM_FLAGS,
            override = device_specific_flags,
        )

    return transition(
        implementation = _rp2_transition_impl,
        inputs = [],
        outputs = merge_flags_for_transition_outputs(
            base = RP2_SYSTEM_FLAGS,
            override = device_specific_flags,
        ),
    )

def _rp2_binary_impl(ctx):
    out = ctx.actions.declare_file(ctx.label.name)
    ctx.actions.symlink(output = out, is_executable = True, target_file = ctx.executable.binary)
    return [DefaultInfo(files = depset([out]), executable = out)]

rp2040_binary = rule(
    _rp2_binary_impl,
    attrs = {
        "binary": attr.label(
            doc = "cc_binary to build for the rp2040",
            cfg = _rp2_transition(_rp2040_flags),
            executable = True,
            mandatory = True,
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    doc = "Builds the specified binary for the rp2040 platform",
    # This target is for rp2040 and can't be run on host.
    executable = False,
)

rp2350_binary = rule(
    _rp2_binary_impl,
    attrs = {
        "binary": attr.label(
            doc = "cc_binary to build for the rp2040",
            cfg = _rp2_transition(_rp2350_flags),
            executable = True,
            mandatory = True,
        ),
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
    doc = "Builds the specified binary for the rp2350 platform",
    # This target is for rp2350 and can't be run on host.
    executable = False,
)
