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

      # nix flake check: builds and runs the unit and serialization adapter tests.
      checks = forAllSystems (pkgs: {
        tests = pkgs.callPackage ./nix/default.nix { withTests = true; };
      });

      # nix develop
      devShells = forAllSystems (pkgs: {
        default = pkgs.mkShell {
          # Brings in cmake, Boost and Cereal used by optional adapter tests.
          inputsFrom = [ (pkgs.callPackage ./nix/default.nix { withTests = true; }) ];

          packages = [
            pkgs.ccls
            pkgs.direnv
            pkgs.clang-tools # clangd, clang-format, clang-tidy
            pkgs.doxygen # doxygen.conf
          ]
          ++ pkgs.lib.optionals pkgs.stdenv.hostPlatform.isLinux [ pkgs.gdb ];

          # Typical workflow inside the shell:
          #   cmake -S . -B build -DBUILD_TESTS=ON \
          #     -DBUILD_CEREAL_TESTS=ON -DBUILD_BOOST_SERIALIZATION_TESTS=ON \
          #   cmake --build build && ctest --test-dir build --output-on-failure
        };
      });
    };
}
