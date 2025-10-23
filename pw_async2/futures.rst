.. _module-pw_async2-futures:

=======
Futures
=======
.. pigweed-module-subpage::
   :name: pw_async2

A ``Future`` is an object that represents the value of an asynchronous operation
which may not yet be complete. Upon completion, the future produces the result
of the operation, if it has one.

Futures are the core interface to ``pw_async2`` asynchronous APIs.

.. note::

   Futures are the new model for writing ``pw_async2`` code. They are currently
   nested under an ``experimental`` namespace and are actively being developed
   as of 2025-10-14.

   Once futures are stabilized, they will be promoted to the root async2
   namespace and the information on this page will be spread across the other
   async2 documentation and guides.

-------------
Core concepts
-------------
Futures operate using the
:ref:`informed poll <module-pw_async2-informed-poll>` model on which
``pw_async2`` is built. This model is summarized below, but it is recommended to
read the full description for important background knowledge.

A ``Future<T>`` exposes two member functions:

- ``Poll<T> Pend(Context& cx)``: Drives the asynchronous operation, returning
  its result on completion. After returning ``Ready``, the future cannot be
  polled again.

- ``bool is_complete()``: Returns whether the future has already completed and
  had its result consumed. Can be called after the future returns ``Ready``.

The base ``Future<T>`` class is an abstract interface. Specific asynchronous
operations return various concrete future types.

Ownership and lifetime
======================
Futures are owned by the caller of an asynchronous operation. The task that
receives the future is responsible for storing and polling it.

The provider of a future must either outlive the future or arrange for the
future to be resolved in an error state when the provider is destroyed.

Polling
=======
Futures are lazy and do nothing on their own. The task owning a future must poll
it to drive it to completion. Calling a future's ``Pend`` function advances its
operation and returns a :cc:`Poll` containing one of two values:

* ``Pending()``: The asynchronous operation has not yet finished. The value is
  not available. The task polling the future is be scheduled to wake when the
  future can make additional progress.

  Typically, your task should propagate a ``Pending`` return upwards to notify
  the dispatcher that it is blocked and should sleep.

* ``Ready(T)``: The operation has completed, and the value is now available.

Once a future returns ``Ready``, its state is final. Attempting to poll it again
results in an assertion.

This polling model allows a single thread to manage many concurrent operations
without blocking.

Composability
=============
The power of futures is their ability to compose to construct complex
asynchronous logic from smaller building blocks.

Futures can be classified into two categories: *leaf* futures and *composite*
futures. Leaf futures represent a specific asynchronous operation, such as a
read from a channel, or waiting for a timer. They contain the required state
for their operations and manage the task waiting on them.

Composite futures are built on top of other futures, combining their results
to build advanced asynchronous execution graphs. For example, a ``Join`` future
waits for multiple other futures to complete, returning all of their results at
once. Composite futures can be used to express complex logic in a declarative
way.

Coroutine support
=================
Futures' simple ``Pend`` API makes them easy to use with async2's
:ref:`coroutine adapter <module-pw_async2-coro>`. You can ``co_await`` a
function that returns a future directly, automatically polling the future to
completion.

--------------------
Working with futures
--------------------

Calling functions that return futures
=====================================
Consider some asynchronous call which produces a simple value on completion.
Pigweed provides ``ValueFuture<T>`` for this common case. The async function has
the following signature:

.. code-block:: c++

   class NumberGenerator {
     ValueFuture<int> GetNextNumber();
   };

You would write a task that calls this operation as follows:

.. tab-set::

   .. tab-item:: Standard polling

      .. code-block:: c++

         class MyTask : public pw::async2::Task {
          private:
           pw::async2::Poll<> DoPend(pw::async2::Context& cx) override {
             // Obtain and store the future, then poll it to completion.
             if (!future_.has_value()) {
               future_.emplace(generator_.GetNextNumber());
             }

             PW_TRY_READY_ASSIGN(int number, future_->Pend(cx));
             PW_LOG_INFO("Received number: %d", number);

             return pw::async2::Ready();
           }

           NumberGenerator& generator_;

           // The future is stored in an optional so it can be lazily initialized
           // inside DoPend. Most concrete futures are not default constructible.
           std::optional<ValueFuture<int>> future_;
         };

   .. tab-item:: C++20 coroutines

      .. code-block:: c++

         pw::async2::Coro<pw::Status> MyCoroutineFunction(pw::async2::CoroContext&,
                                                          NumberGenerator& generator) {
           // Pigweed's coroutine integration allows futures to be awaited directly.
           int number = co_await generator.GetNextNumber();
           PW_LOG_INFO("Received number: %d", number);
           co_return pw::OkStatus();
         }

Writing functions that return futures
=====================================
All future-based ``pw_async2`` APIs have the signature

.. code-block::

   Future<T> DoThing(Args... args);

Where ``Future<T>`` is some concrete future implementation (e.g.
``ValueFuture``) which resolves to a value of type ``T`` and ``Args``
represents any arguments to the operation.

When defining an asynchronous API, the function should always return a
``Future`` directly --- not a ``Result<Future>`` or
``std::optional<Future>``. If the operation is fallible, that should be
expressed by the future's output, e.g. ``Future<Result<T>>``.

This is necessary for proper composability. It makes using asynchronous APIs
consistent and enables higher-level futures which compose other futures to
function cleanly. Additionally, returning a ``Future`` directly is essential to
be able to work with coroutines: ``co_await`` can be used directly and will
resolve to a ``Result<T>``.

Resolving futures
-----------------
After you vend a future from an asynchronous operation, you need a way to track
and resolve it once the operation has completed. This is the role of providers.

Initially, all leaf futures in Pigweed are listable, allowing them to be stored
in one of the following providers:

- A :cc:`ListFutureProvider` allows multiple concurrent tasks to wait on an
  operation. The provider maintains a FIFO list of futures. When the operation
  completes, you can pop one (or more) futures from the list and resolve them.

- A :cc:`SingleFutureProvider` only allows one task waiting on it at a time. It
  asserts if you vend a second future. Once the operation is complete, the
  future can be taken out and resolved.

Listable futures take their provider as a constructor argument and automatically
manage their presence in the list.

---------------------
Implementing a future
---------------------
While ``pw_async2`` provides a suite of common futures and combinators, you
may sometimes need to implement a custom leaf future to represent a specific
asynchronous operation (e.g., waiting for a hardware interrupt).

The primary tool for this is the :cc:`ListableFutureWithWaker` base class.

ListableFutureWithWaker
=======================
This class provides the essential machinery for most custom leaf futures:

- It stores the :cc:`Waker` of the task that polls it.
- It manages its membership in an intrusive list, allowing it to be tracked
  by a "provider".
- It tracks completion internally.

Waking mechanism
================
When a task polls a future and it returns ``Pending``, the future must store
the task's :cc:`Waker` from the provided :cc:`Context`. This is handled
automatically by :cc:`ListableFutureWithWaker`.

On the other side of the asynchronous operation (e.g., in an interrupt handler),
when the operation completes, the provider is used to retrieve the future, and
its ``Wake()`` function is called. This notifies the dispatcher that the task
waiting on this future is ready to make progress and should be polled again.

Example: Waiting for a GPIO interrupt
=====================================
Below is an example of a custom future that waits for a GPIO button press using
interfaces from ``pw_digital_io``.

.. literalinclude:: examples/custom_future.cc
   :language: cpp
   :linenos:
   :start-after: // DOCSTAG: [pw_async2-examples-custom-future]
   :end-before: // DOCSTAG: [pw_async2-examples-custom-future]

This example demonstrates the core mechanics of creating a custom future.
This pattern of waiting for a single value from a producer is so common that
``pw_async2`` provides ``ValueFuture`` and ``ValueProvider`` to handle it.
In practice, you would return a ``VoidFuture`` (alias for ``ValueFuture<void>``)
from ``WaitForPress`` instead of writing a custom ``ButtonFuture``.

-----------
Combinators
-----------
Combinators allow you to compose multiple futures into a single future to express
complex control flow.

Join
====
:cc:`Join` waits for multiple futures to complete and returns a tuple of their
results.

.. code-block:: cpp

   #include "pw_async2/join.h"

   ValueFuture<pw::Status> DoWork(int id);

.. tab-set::

   .. tab-item:: Standard polling

      .. code-block:: cpp

         class JoinTask : public pw::async2::Task {
          private:
           pw::async2::Poll<> DoPend(pw::async2::Context& cx) override {
             if (!future_.has_value()) {
               // Start three futures concurrently and wait for all of them
               // to complete.
               future_.emplace(
                   pw::async2::experimental::Join(DoWork(1), DoWork(2), DoWork(3)));
             }

             PW_TRY_READY_ASSIGN(auto results, future_->Pend(cx));
             auto [status1, status2, status3] = *results;

             if (!status1.ok() || !status2.ok() || !status3.ok()) {
               PW_LOG_ERROR("Operation failed");
             } else {
               PW_LOG_INFO("All operations succeeded");
             }

             return pw::async2::Ready();
           }

           std::optional<JoinFuture<ValueFuture<pw::Status>,
                                    ValueFuture<pw::Status>,
                                    ValueFuture<pw::Status>>>
               future_;
         };

   .. tab-item:: C++20 coroutines

      .. code-block:: cpp

         pw::async2::Coro<pw::Status> JoinExample(pw::async2::CoroContext&) {
           // Start three futures concurrently and wait for all of them to complete.
           auto [status1, status2, status3] =
               co_await pw::async2::experimental::Join(DoWork(1), DoWork(2), DoWork(3));

           if (!status1.ok() || !status2.ok() || !status3.ok()) {
             PW_LOG_ERROR("Operation failed");
             co_return pw::Status::Internal();
           }
           PW_LOG_INFO("All operations succeeded");
           co_return pw::OkStatus();
         }

Select
======
:cc:`Select` waits for the *first* of multiple futures to complete. It returns a
:cc:`SelectFuture` which resolves to an :cc:`OptionalTuple` containing the
result. If additional futures happen to complete between the first future
completing the task re-running, the tuple stores all of their results.

.. code-block:: cpp

   #include "pw_async2/select.h"

   ValueFuture<int> DoWork();
   ValueFuture<int> DoOtherWork();

.. tab-set::

   .. tab-item:: Standard polling

      .. code-block:: cpp

         class SelectTask : public pw::async2::Task {
          private:
           pw::async2::Poll<> DoPend(pw::async2::Context& cx) override {
             if (!future_.has_value()) {
               // Race two futures and wait for the first one to complete.
               future_.emplace(
                   pw::async2::experimental::Select(DoWork(), DoOtherWork()));
             }

             PW_TRY_READY_ASSIGN(auto results, future_->Pend(cx));

             // Check which future(s) completed.
             // In this example, we check all of them, but it's common to return
             // after the first result.
             if (results.has_value<0>()) {
               PW_LOG_INFO("DoWork completed with: %d", results.get<0>());
             }
             if (results.has_value<1>()) {
               PW_LOG_INFO("DoOtherWork completed with: %d", results.get<1>());
             }

             return pw::async2::Ready();
           }

           std::optional<SelectFuture<ValueFuture<int>, ValueFuture<int>>> future_;
         };

   .. tab-item:: C++20 coroutines

      .. code-block:: cpp

         pw::async2::Coro<int> SelectExample(pw::async2::CoroContext&) {
           // Race two futures and wait for the first one to complete.
           auto results =
               co_await pw::async2::experimental::Select(DoWork(), DoOtherWork());

           // Check which future(s) completed.
           // In this example, we check all of them, but it's common to return
           // after the first result.
           if (results.has_value<0>()) {
             int result = results.get<0>();
             PW_LOG_INFO("DoWork completed with: %d", result);
           }

           if (results.has_value<1>()) {
             int result = results.get<1>();
             PW_LOG_INFO("DoOtherWork completed with: %d", result);
           }

           co_return pw::OkStatus();
         }
