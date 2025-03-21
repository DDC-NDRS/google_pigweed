.. _module-pw_blob_store:

=============
pw_blob_store
=============
.. pigweed-module::
   :name: pw_blob_store

``pw_blob_store`` is a storage container library for storing a single blob of
data. ``BlobStore`` is a flash-backed persistent storage system with integrated
data integrity checking that serves as a lightweight alternative to a file
system.

-----
Usage
-----
Most operations on a ``BlobStore`` are done using ``BlobReader`` and
``BlobWriter`` objects that have been constructed using a ``BlobStore``. Though
a ``BlobStore`` may have multiple open ``BlobReader`` objects, no other
readers/writers may be active if a ``BlobWriter`` is opened on a blob store.

The data state of a blob can be checked using the ``HasData()`` method.
The method returns true if the blob is currenty valid and has at least one data
byte. This allows checking if a blob has stored data without needing to
instantiate and open a reader or writer.

Write buffer
============

BlobStore uses a write buffer to allow writes smaller than and/or unaligned to
the flash write aligment. BlobStore also supports using the write buffer for
deferred writes that can be enqueued and written to flash at a later time or by
a different thread/context.

BlobStore can be used with a zero-size write buffer to reduce memory
requirements. When using zero-size write buffer, the user is required to write
maintain write sizes that are a multiple of the flash write size the blob is
configured for.

If a non-zero sized write buffer is used, the write buffer size must be a
multiple of the flash write size.

Writing to a BlobStore
----------------------
``BlobWriter`` objects are ``pw::stream::Writer`` compatible, but do not support
reading any of the blob's contents. Opening a ``BlobWriter`` on a ``BlobStore``
that contains data will discard any existing data if ``Discard()``, ``Write
()``, or ``Erase()`` are called. Partially written blobs can be resumed using
``Resume()``. There is currently no mechanism to allow appending to completed
data write.

.. code-block:: cpp

   BlobStore::BlobWriterWithBuffer writer(my_blob_store);
   writer.Open();
   writer.Write(my_data);

   // ...

   // A close is implied when a BlobWriter is destroyed. Manually closing a
   // BlobWriter enables error handling on Close() failure.
   writer.Close();

Resuming a BlobStore write
--------------------------
``BlobWriter::Resume()`` supports resuming writes for blobs that have not been
completed (``writer.Close()``). Supported resume situations are after ``Abandon()``
has been called on a write or after a crash/reboot with a fresh ``BlobStore``
instance.

``Resume()`` opens the ``BlobWriter`` at the most recent safe resume point.
``Resume()`` finds the furthest written point in the flash partition and then backs
up by erasing any partially written sector plus a full sector. This backing up is to
try to avoid any corrupted or otherwise wrong data that might have resulted from the
previous write failing. ``Resume()`` returns the current number of valid bytes in the
resumed write.

If the blob is using a ChecksumAlgorithm, the checksum of the resumed blob write instance
calculated from the content of the already written data. If it is desired to check the
integrity of the already written data, ``BlobWriter::CurrentChecksum()`` can be used to
check against the incoming data.

Once ``Resume()`` has successfully completed, the writer is ready to continue writing
as normal.

Erasing a BlobStore
===================
There are two distinctly different mechanisms to "erase" the contents of a BlobStore:

#. ``Discard()``: Discards any ongoing writes and ensures ``BlobReader`` objects
   see the ``BlobStore`` as empty. This is the fastest way to logically erase a
   ``BlobStore``.
#. ``Erase()``: Performs an explicit flash erase of the ``BlobStore``'s
   underlying partition. This is useful for manually controlling when a flash
   erase is performed before a ``BlobWriter`` starts to write data (as flash
   erase operations may be time-consuming).

Naming a BlobStore's contents
=============================
Data in a ``BlobStore`` May be named similarly to a file. This enables
identification of a BlobStore's contents in cases where different data may be
stored to a shared blob store. This requires an additional RAM buffer that can
be used to encode the BlobStore's KVS metadata entry. Calling
``MaxFileNameLength()`` on a ``BlobWriter`` will provide the max file name
length based on the ``BlobWriter``'s metadata encode buffer size.

``SetFileName()`` performs a copy of the provided file name, meaning it's safe
for the ``std::string_view`` to be invalidated after the function returns.

.. code-block:: cpp

   constexpr size_t kMaxFileNameLength = 48;
   BlobStore::BlobWriterWithBuffer<kMaxFileNameLength> writer(my_blob_store);
   writer.Open();
   writer.SetFileName("stonks.jpg");
   writer.Write(my_data);
   // ...
   writer.Close();

Reading from a BlobStore
------------------------
A ``BlobStore`` may have multiple open ``BlobReader`` objects. No other
readers/writers may be open/active if a ``BlobWriter`` is opened on a blob
store.

0) Create BlobReader instance
1) BlobReader::Open()
2) Read data using BlobReader::Read() or
   BlobReader::GetMemoryMappedBlob(). BlobReader is seekable. Use
   BlobReader::Seek() to read from a desired offset.
3) BlobReader::Close()

--------------------------
FileSystem RPC integration
--------------------------
``pw_blob_store`` provides an optional ``FileSystemEntry`` implementation for
use with ``pw_file``'s ``FlatFileSystemService``. This simplifies the process of
enumerating ``BlobStore`` objects as files via ``pw_file``'s ``FileSystem`` RPC
service.

-----------
Size report
-----------
The following size report showcases the memory usage of the blob store.

.. TODO: b/388905812 - Re-enable the size report.
.. .. include:: blob_size
.. include:: ../size_report_notice

.. note::
  The documentation for this module is currently incomplete.
