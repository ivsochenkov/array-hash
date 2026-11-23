{
  lib,
  stdenv,
  cmake,
  boost,
  cereal,
  # Build and run the Boost.Test suite plus all optional serialization adapter
  # test suites. Off by default so a plain `nix build` stays dependency-light.
  withTests ? false,
}:

let
  # Keep the package version in sync with `project(tsl-array-hash VERSION x.y.z)`.
  version =
    let
      matches = map (builtins.match "project\\(tsl-array-hash VERSION ([0-9.]+) .*") (
        lib.splitString "\n" (builtins.readFile ../CMakeLists.txt)
      );
    in
    builtins.head (builtins.head (lib.filter (m: m != null) matches));

  # Keep static Boost libraries available to the optional test build; CMake's
  # FindBoost module discovers the components provided by this package.
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

  # Nix obtains optional serialization dependencies from nixpkgs rather than
  # initializing the Cereal Git submodules.
  buildInputs = lib.optionals withTests [ boostWithStatic cereal ];

  cmakeFlags = [
    (lib.cmakeBool "BUILD_TESTS" withTests)
    (lib.cmakeBool "BUILD_CEREAL_TESTS" withTests)
    (lib.cmakeBool "BUILD_BOOST_SERIALIZATION_TESTS" withTests)
  ];

  doCheck = withTests;

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