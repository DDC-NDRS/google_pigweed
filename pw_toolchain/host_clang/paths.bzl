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

"""Private helper function for bzlmod compatibility."""

visibility(["//pw_toolchain/host_clang/..."])

# TODO: https://pwbug.dev/346388161 - Remove this once we migrate to rules_cc.
LINUX_SYSROOT = "external/" + Label("@linux_sysroot").repo_name
LLVM_TOOLCHAIN = "external/" + Label("@llvm_toolchain_macos").repo_name
