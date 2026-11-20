{
  lib,
  stdenv,
  cmake,
  boost,
  # Build and run the Boost.Test suite (tests/). Off by default so that a plain
  # `nix build` is fast and does not need Boost; `nix flake check` enables it.
  withTests ? false,
}:

let
  # Keep the package version in sync with `project(tsl-array-hash VERSION x.y.z)`.
  version =
    let
      matches = map (builtins.match "project\\(tsl-array-hash VERSION ([0-9.]+)\\).*") (
        lib.splitString "\n" (builtins.readFile ../CMakeLists.txt)
      );
    in
    builtins.head (builtins.head (lib.filter (m: m != null) matches));

  # tests/CMakeLists.txt sets Boost_USE_STATIC_LIBS, and nixpkgs' boost only
  # ships shared libraries by default.
  boostWithStatic = boost.override { enableStatic = true; };
in
stdenv.mkDerivation {
  pname = "tsl-array-hash";
  inherit version;

  src = lib.fileset.toSource {
    root = ../.;
    fileset = lib.fileset.unions (
      [
        ../CMakeLists.txt
        ../cmake
        ../include
      ]
      ++ lib.optional withTests ../tests
    );
  };

  strictDeps = true;

  nativeBuildInputs = [ cmake ];

  buildInputs = lib.optionals withTests [ boostWithStatic ];

  cmakeFlags = [
    (lib.cmakeBool "BUILD_TESTS" withTests)
  ];

  doCheck = withTests;

  # The top-level CMakeLists.txt never calls enable_testing(), so `ctest` would
  # find nothing. Run the test binary directly instead.
  checkPhase = ''
    runHook preCheck
    ./tests/tsl_array_hash_tests
    runHook postCheck
  '';

  meta = {
    description = "C++ implementation of a fast and memory efficient hash map and hash set specialized for strings";
    homepage = "https://github.com/ivsochenkov/array-hash";
    license = lib.licenses.mit;
    platforms = lib.platforms.all;
  };
}