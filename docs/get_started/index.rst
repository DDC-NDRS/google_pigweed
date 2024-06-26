.. _docs-get-started:

===========
Quickstarts
===========
.. _docs-first-time-setup:

--------------------------------------------------
Configure your workstation for Pigweed development
--------------------------------------------------
Pigweed does its best to bundle all its dependencies into an isolated
environment using :ref:`module-pw_env_setup`. While this doesn't eliminate
all prerequisites, it greatly accelerates new developer onboarding.

The first-time setup guides below are required for any Pigweed-based project,
but only need to be done once per machine.

.. grid:: 2

   .. grid-item-card:: First-time setup
      :link: docs-first-time-setup-guide
      :link-type: ref
      :class-item: sales-pitch-cta-primary

      Prerequisites, first-time setup, and support notes for Linux, macOS, and
      Windows.

   .. grid-item-card:: Install Bazel
      :link: docs-install-bazel
      :link-type: ref
      :class-item: sales-pitch-cta-secondary

      Recommendations on how to install Bazel.

------------------------------
Create a Pigweed-based project
------------------------------
.. grid:: 1

   .. grid-item-card:: :octicon:`rocket` Bazel
      :link: docs-get-started-bazel
      :link-type: ref
      :class-item: sales-pitch-cta-primary

      Fork our minimal, Bazel-based starter code. Bazel is the recommended
      build system for new projects using Pigweed.

.. grid:: 1

   .. grid-item-card:: :octicon:`rocket` Zephyr
      :link: docs-quickstart-zephyr
      :link-type: ref
      :class-item: sales-pitch-cta-secondary

      Learn how to set up a C++-based Zephyr project that is ready to use
      Pigweed and then build the app with Zephyr's ``native_sim`` board.

.. grid:: 1

   .. grid-item-card:: :octicon:`code` Examples
      :link: https://pigweed.dev/examples/index.html
      :link-type: url
      :class-item: sales-pitch-cta-secondary

      Check out the examples repo, a repository that outlines the recommended
      way to use Pigweed in a broader GN-based project. Note that Bazel is the
      recommended build system for new projects using Pigweed, whereas the
      examples repo uses GN.


.. grid:: 2

   .. grid-item-card:: :octicon:`code` Kudzu
      :link: docs-kudzu
      :link-type: ref
      :class-item: sales-pitch-cta-secondary

      Study the code of Kudzu, a just-for-fun Maker Faire 2023 project that
      demonstrates complex Pigweed usage. This project also uses GN.

   .. grid-item-card:: :octicon:`list-ordered` Upstream Pigweed
      :link: docs-get-started-upstream
      :link-type: ref
      :class-item: sales-pitch-cta-secondary

      Get set up to contribute to upstream Pigweed.

------------------------------------------
Use Pigweed modules in an existing project
------------------------------------------
Pigweed is modular: you can use as much or as little of it as you need.

.. grid:: 2

   .. grid-item-card:: :octicon:`rocket` Bazel
      :link: docs-bazel-integration
      :link-type: ref
      :class-item: sales-pitch-cta-primary

      Instructions for how to use a Pigweed module in an existing Bazel
      project.

   .. grid-item-card:: :octicon:`list-ordered` GitHub Actions
      :link: docs-github-actions
      :link-type: ref
      :class-item: sales-pitch-cta-secondary

      How to set up GitHub Actions to build and test your Bazel-based
      Pigweed project.

.. toctree::
   :maxdepth: 1
   :hidden:

   First-time setup <first_time_setup>
   Install Bazel <install_bazel>
   Bazel quickstart <bazel>
   Bazel integration <bazel_integration>
   GitHub Actions <github_actions>
   Zephyr quickstart <zephyr>
   Upstream Pigweed <upstream>
