# Copyright 2021 The Pigweed Authors
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
"""Bazel rules for downloading CIPD packages."""

load(
    "//pw_env_setup/bazel/cipd_setup/internal:cipd_internal.bzl",
    _cipd_client_impl = "cipd_client_impl",
    _cipd_deps_impl = "cipd_deps_impl",
    _cipd_repository_impl = "cipd_repository_impl",
)

_cipd_client_repository = repository_rule(
    _cipd_client_impl,
    attrs = {
        "_cipd_digest_file": attr.label(default = str(Label("//pw_env_setup:py/pw_env_setup/cipd_setup/.cipd_version.digests"))),
        "_cipd_version_file": attr.label(default = str(Label("//pw_env_setup:py/pw_env_setup/cipd_setup/.cipd_version"))),
    },
    doc = """
Fetches the cipd client.

This rule should not be used directly and instead should be called via
the cipd_client_repository macro.
""",
)

def cipd_client_repository():
    """Fetches the cipd client.

    Fetches the cipd client to the prescribed remote repository target
    prefix 'cipd_client'. This rule should be called before a
    cipd_repository rule is instantiated.
    """
    _cipd_client_repository(
        name = "cipd_client",
    )

cipd_repository = repository_rule(
    _cipd_repository_impl,
    attrs = {
        "build_file": attr.label(
            allow_single_file = True,
            doc = "Override the BUILD file in the new CIPD repository.",
        ),
        "patch_args": attr.string_list(
            doc = "Arguments to pass to the patch tool. List of strings.",
            default = [],
        ),
        "patches": attr.label_list(
            doc = "A list of patches to apply to the CIPD package after downloading it",
            default = [],
        ),
        "path": attr.string(
            doc = "Path within CIPD where this repository lives.",
        ),
        "tag": attr.string(
            doc = "Tag specifying which version of the repository to fetch.",
        ),
        "tag_by_os": attr.string_dict(
            doc = "Dict from OS name (mac, linux, windows) to tag. Incompatible with the 'tag' attribute.",
        ),
        "_cipd_client": attr.label(
            default = "@cipd_client//:cipd",
            doc = "Location of the CIPD client binary (internal).",
        ),
    },
    doc = """
Downloads a singular CIPD dependency to the root of a remote repository.

Example:

    load(
        "@pigweed//pw_env_setup/bazel/cipd_setup:cipd_rules.bzl",
        "cipd_client_repository",
        "cipd_repository",
    )

    # Must be called before cipd_repository
    cipd_client_repository()

    cipd_repository(
        name = "bloaty",
        path = "fuchsia/third_party/bloaty/${os=linux,mac}-${arch=amd64}",
        tag = "git_revision:c057ba4f43db0506d4ba8c096925b054b02a8bd3",
    )
""",
)

_pigweed_deps = repository_rule(
    _cipd_deps_impl,
    attrs = {
        "_pigweed_packages_json": attr.label(
            default = str(Label("//pw_env_setup:py/pw_env_setup/cipd_setup/pigweed.json")),
        ),
        "_upstream_testing_packages_json": attr.label(
            default = str(Label("//pw_env_setup:py/pw_env_setup/cipd_setup/testing.json")),
        ),
    },
)

def pigweed_deps():
    """Configures Pigweeds Bazel dependencies

    Example:
        load("@pigweed//pw_env_setup:pigweed_deps.bzl", "pigweed_deps")

        pigweed_deps()

        load("@cipd_deps//:cipd_init.bzl", "cipd_init")

        cipd_init()
"""
    _pigweed_deps(
        name = "cipd_deps",
    )
