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

load("@bazel_skylib//rules/directory:directory.bzl", "directory")
load("@bazel_skylib//rules/directory:subdirectory.bzl", "subdirectory")
load("@rules_cc//cc/toolchains/args:sysroot.bzl", "cc_sysroot")

package(default_visibility = ["//visibility:public"])

cc_sysroot(
    name = "sysroot",
    sysroot = ":root",
    data = [":root"],
)

directory(
    name = "root",
    srcs = glob(["**/*"]),
)

subdirectory(
    name = "usr-include-arm-linux-gnueabihf",
    parent = ":root",
    path = "usr/include/arm-linux-gnueabihf",
)

subdirectory(
    name = "usr-include-x86_64-linux-gnu",
    parent = ":root",
    path = "usr/include/x86_64-linux-gnu",
)

subdirectory(
    name = "usr-include",
    parent = ":root",
    path = "usr/include",
)
