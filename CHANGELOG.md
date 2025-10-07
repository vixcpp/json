Changelog – vixcpp/json

All notable changes to this module will be documented in this file.
The format is based on Keep a Changelog

and this module adheres to Semantic Versioning
.

[0.1.0] – 2025-10-07
Added

Initial release of the json module for vix.cpp.

Header-only, zero-dependency JSON handling using nlohmann::json.

Serialization of native C++ structures to JSON.

Deserialization of JSON into strongly-typed C++ models.

Automatic integration with vix.cpp model and request/response system.

Support for pretty-printing and compact output.

Helper macros for custom (de)serialization logic.

UTF-8 encoding support.

Changed

Improved internal abstraction over nlohmann::json for future replacement flexibility.

Fixed

Minor issues with nested object serialization in edge cases.

[0.0.1] – Draft

Project structure for the json module initialized.

Basic integration tests with vix.cpp::App and response rendering.

CMake targets and dependencies configured.
