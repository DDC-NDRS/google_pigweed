# Copyright 2023 The Pigweed Authors
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
"""Install and check status of Emboss."""

import pathlib
from typing import Sequence

import pw_package.git_repo
import pw_package.package_manager


class Emboss(pw_package.git_repo.GitRepo):
    """Install and check status of Emboss."""

    def __init__(self, *args, **kwargs):
        super().__init__(
            *args,
            name="emboss",
            # LINT.IfChange(emboss)
            url=(
                "https://pigweed.googlesource.com"
                "/third_party/github/google/emboss"
            ),
            tag="v2024.1017.203246",
            # LINT.ThenChange(/MODULE.bazel:emboss)
            **kwargs,
        )

    def info(self, path: pathlib.Path) -> Sequence[str]:
        return (
            f"{self.name} installed in: {path}",
            "Enable by running 'gn args out' and adding this line:",
            f'  dir_pw_third_party_emboss = "{path}"',
        )


pw_package.package_manager.register(Emboss)
