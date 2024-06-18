load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
_HCC_COMMIT = "d008f00d54ba46acf879385d12ffdd7494bf1ba5"
    
http_archive(
    name = "hedron_compile_commands",
    # Replace the commit hash in both places (below) with the latest, rather than using the stale one here.
    # Even better, set up Renovate and let it do the work for you (see "Suggestion: Updates" in the README).
    url = "https://github.com/hedronvision/bazel-compile-commands-extractor/archive/%s.tar.gz"%_HCC_COMMIT,
    strip_prefix = "bazel-compile-commands-extractor-" + _HCC_COMMIT,
    sha256 = "0f90fd65d832cd19a8feb7a9848dc3762e3a6ea11265abbcebe35d14a1c10ec9",
    # When you first run this tool, it'll recommend a sha256 hash to put here with a message like: 
    # "DEBUG: Rule 'hedron_compile_commands' indicated that a canonical reproducible form 
    # can be obtained by modifying arguments sha256 = ..."
   
)
load("@hedron_compile_commands//:workspace_setup.bzl", "hedron_compile_commands_setup")
hedron_compile_commands_setup()