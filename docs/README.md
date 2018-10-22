# Perfetto - Performance instrumentation and tracing

Perfetto is an open-source project for performance instrumentation and tracing
of Linux/Android/Chrome platforms and user-space apps.  
It consists of:

**A portable, high efficiency, user-space tracing library**  
designed for tracing of multi-process systems, based on zero-alloc zero-copy
zero-syscall (on fast-paths) writing of protobufs over shared memory.

**OS-wide Linux/Android probes for platform debugging**
* Kernel tracing: a daemon that converts Kernel [Ftrace][ftrace] events into
  API-stable protobufs, on device, with low overhead.
* I/O tracing
* Many new probes coming soon: heap profiling, perf sampling, syscall tracing.

**Web-based frontend**  
A UI for inspection and analysis of traces (coming soon).

**Batch processing of traces**  
A python / C++ (TBD) library for trace-based metrics (coming soon).


![Perfetto Stack](https://storage.googleapis.com/perfetto/markdown_img/perfetto-stack.png)

Goals
-----
Perfetto is building the next-gen unified tracing ecosystem for:
- Android platform tracing ([Systrace][systrace])
- Chrome platform tracing ([chrome://tracing][chrome-tracing])
- App-defined user-space tracing (including support for non-Android apps).

The goal is to create an open, portable and developer friendly tracing ecosystem
for app and platform performance debugging.

Key features
------------
**Designed for production**  
Perfetto's tracing library and daemons are designed for use in production.
Privilege isolation is a key design goal:
* The interface for writing trace events are decoupled from the interface for
  read-back and control and can be subjected to different ACLs.
* Despite being based on shared memory, Perfetto is designed to prevent
  cross-talk between data sources, even in case of arbitrary code execution
  (memory is shared point-to-point, memory is never shared between processes).
* Perfetto daemons are designed following to the principle of least privilege,
  in order to allow strong sandboxing (via SELinux on Android).

See [security-model.md](security-model.md) for more details.

**Long traces**  
Pefetto aims at supporting hours-long / O(100GB) traces, both in terms of
recording backend and UI frontend.

**Interoperability**  
Perfetto traces (output) and configuration (input) consists of protobuf
messages, in order to allow interoperability with several languages.

See [trace-format.md](trace-format.md) for more details.

**Composability**  
As Perfetto is designed both for OS-level tracing and app-level tracing, its
design allows to compose several instances of the Perfetto tracing library,
allowing to nest multiple layers of tracing and drive then with the same
frontend. This allows powerful blending of app-specific and OS-wide trace
events.
See [multi-layer-tracing.md](multi-layer-tracing.md) for more details.

**Portability**  
The only dependencies of Perfetto's tracing libraries are C++11 and [Protobuf lite][protobuf] (plus google-test, google-benchmark, libprotobuf-full for testing).

**Extensibility**  
Perfetto allows third parties to defined their own protobufs for:
* [(input) Configuration](/protos/perfetto/config/data_source_config.proto#52)
* [(output) Trace packets](/protos/perfetto/trace/trace_packet.proto#36)

Allowing apps to define their own strongly-typed input and output schema.
See [trace-format.md](trace-format.md) for more details.


Docs
----
* [Contributing](contributing.md)
* [Build instructions](build-instructions.md)
* [Running tests](testing.md)
* [Running Perfetto](running.md)
* [Capturing long traces](long-traces.md)
* [Trace processor](trace-processor.md)
* [Key concepts and architecture](architecture.md)
* [Life of a tracing session](life-of-a-tracing-session.md)
* [Ftrace interop](ftrace.md)
* [Performance benchmarks](benchmarks.md)
* [Trace config](trace-config.md)
* [Trace format](trace-format.md)
* [Multi-layer tracing](multi-layer-tracing.md)
* [Security model](security-model.md)
* [Embedding Perfetto in your own project](embedder-guide.md)
* [ProtoZero internals](protozero.md)
* [IPC internals](ipc.md)


Bugs
----
* For bugs affecting Android or the tracing internals use the internal
bug tracker ([go/perfetto-bugs](http://goto.google.com/perfetto-bugs)).
* For bugs affecting Chrome use http://crbug.com, Component:Speed>Tracing
label:Perfetto.


[ftrace]: https://www.kernel.org/doc/Documentation/trace/ftrace.txt
[systrace]: https://developer.android.com/studio/command-line/systrace.html
[chrome-tracing]: https://www.chromium.org/developers/how-tos/trace-event-profiling-tool
[protobuf]: https://developers.google.com/protocol-buffers/