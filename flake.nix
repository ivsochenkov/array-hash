{
  description = "tsl::array_hash - header-only C++ hash map/set specialized for strings";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";

  outputs =
    { self, nixpkgs }:
    let
      systems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      # Use as `nixpkgs.overlays = [ array-hash.overlays.default ];`
      # to get `pkgs.tsl-array-hash`.
      overlays.default = final: _prev: {
        tsl-array-hash = final.callPackage ./nix/default.nix { };
      };

      # nix build  /  nix build .#tsl-array-hash
      packages = forAllSystems (pkgs: rec {
        tsl-array-hash = pkgs.callPackage ./nix/default.nix { };
        default = tsl-array-hash;
      });

      # nix flake check: builds the Boost.Test suite and runs it.
      checks = forAllSystems (pkgs: {
        tests = pkgs.callPackage ./nix/default.nix { withTests = true; };
      });

      # nix develop
      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          # Brings in cmake and Boost (with static libs) used by the test suite.
          inputsFrom = [ (pkgs.callPackage ./nix/default.nix { withTests = true; }) ];

          packages = [
            pkgs.ccls
            pkgs.direnv
            pkgs.clang-tools # clangd, clang-format, clang-tidy
            pkgs.doxygen # doxygen.conf
          ]
          ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isLinux [ pkgs.gdb ];

          # Typical workflow inside the shell:
          #   cmake -S . -B build -G Ninja -DBUILD_TESTS=ON
          #   cmake --build build && ./build/tests/tsl_array_hash_tests
        };
      });
    };
}
