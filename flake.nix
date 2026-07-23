{
  description = "Instance Sim Engine - OpenGL instanced rendering library";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs =
    { self, nixpkgs }:
    let
      supportedSystems = [
        "x86_64-linux"
        "aarch64-linux"
        "x86_64-darwin"
        "aarch64-darwin"
      ];
      forAllSystems = nixpkgs.lib.genAttrs supportedSystems;

    in
    {
      packages = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          engine = pkgs.stdenv.mkDerivation {
            pname = "instance-sim-engine";
            version = "1.0";
            src = self;
            cmakeFlags = [ "-DBUILD_APPS=OFF" ];
            buildInputs = [
              pkgs.glfw
              pkgs.libGL
              pkgs.glm
            ];
            nativeBuildInputs = [
              pkgs.cmake
              pkgs.pkg-config
            ];
          };

          default = self.packages.${system}.engine;
        }
      );

      devShells = forAllSystems (
        system:
        let
          pkgs = nixpkgs.legacyPackages.${system};
        in
        {
          default = pkgs.mkShell {
            buildInputs = with pkgs; [
              glfw
              libGL
              glm
            ];
            nativeBuildInputs = with pkgs; [
              cmake
              pkg-config

              # lsps
              clang-tools
              basedpyright

              (python3.withPackages (
                ps: with ps; [
                  pandas
                  matplotlib
                  numpy
                ]
              ))
            ];
          };
        }
      );
    };
}
