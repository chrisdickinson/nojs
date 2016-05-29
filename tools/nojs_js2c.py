#!/usr/bin/env python

import os
import sys
import optparse

directory = os.path.dirname(__file__)
sys.path.append(
    os.path.realpath(os.path.join(directory, '..', 'third_party', 'v8', 'tools'))
)

import js2c

js2c.GET_SCRIPT_SOURCE_CASE = """\
    if (index == %(i)i) return std::vector<const char>(sources + %(offset)i, sources + %(offset)i + %(source_length)i);
"""

js2c.GET_SCRIPT_NAME_CASE = """\
    if (index == %(i)i) { const char* name {"%(name)s"}; return std::vector<const char>(name, name + %(length)i); }
"""

js2c.HEADER_TEMPLATE = """\
// This file was generated from .js source files by GYP.  If you
// want to make changes to this file you should either change the
// javascript source files or the GYP script.
#include "src/nojs_natives.h"
#include <vector>

namespace NoJS {

%(sources_declaration)s\

  int NativesCollection::GetBuiltinsCount() {
    return %(builtin_count)i;
  }

  int NativesCollection::GetDebuggerCount() {
    return %(debugger_count)i;
  }

  int NativesCollection::GetIndex(const char* name) {
%(get_index_cases)s\
    return -1;
  }

  std::vector<const char> NativesCollection::GetScriptSource(int index) {
%(get_script_source_cases)s\
    return std::vector<const char>();
  }

  std::vector<const char> NativesCollection::GetScriptName(int index) {
%(get_script_name_cases)s\
    return std::vector<const char>();
  }

  std::vector<const char> NativesCollection::GetScriptsSource() {
    return std::vector<const char>(sources, sources + %(total_length)i);
  }

}
"""

def _JS2C(sources, target, native_type, raw_file, startup_blob, emit_js):
    prepared_sources = js2c.PrepareSources(sources, native_type, emit_js)
    sources_output = "".join(prepared_sources.modules)
    metadata = js2c.BuildMetadata(prepared_sources, sources_output, native_type)

    # Optionally emit raw file.
    if raw_file:
        output = open(raw_file, "w")
        output.write(sources_output)
        output.close()

    if startup_blob:
        js2c.WriteStartupBlob(prepared_sources, startup_blob)

    # Emit resulting source file.
    output = open(target, "w")
    if emit_js:
        output.write(sources_output)
    else:
        output.write(js2c.HEADER_TEMPLATE % metadata)
    output.close()


def _main():
    parser = optparse.OptionParser()
    parser.add_option(
        "--raw",
        help="file to write the processed sources array to."
    )
    parser.add_option(
        "--startup_blob",
        help="file to write the startup blob to."
    )
    parser.add_option(
        "--js",
        help="writes a JS file output instead of a C file",
        action="store_true",
        default=False,
        dest='js'
    )
    parser.add_option(
        "--nojs",
        action="store_false",
        default=False,
        dest='js'
    )
    parser.set_usage("""js2c out.cc type sources.js ...
    out.cc: C code to be generated.
    type: type parameter for NativesCollection template.
    sources.js: JS internal sources or macros.py.""")
    (options, args) = parser.parse_args()
    JS2C(
        args[2:],
        args[0],
        args[1],
        options.raw,
        options.startup_blob,
        options.js
    )

if __name__ == "__main__":
    js2c.main()
