{
  description = "Renderer for Tromp Lambda diagrams";
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs";
    flake-utils.url = "github:numtide/flake-utils";

    nob = {
      url = "github:tsoding/nob.h";
      flake = false;
    };
  };

  outputs = inputs@{
    nixpkgs,
    flake-utils,
    ...
  }: flake-utils.lib.eachDefaultSystem (system:
    let
      pkgs = nixpkgs.legacyPackages.${system};
      nob = pkgs.stdenv.mkDerivation {
        name = "nob.h";
        src = inputs.nob;
        doBuild = false;
        doCheck = false;

        installPhase = ''
          mkdir -p $out/include/
          cp nob.h $out/include/
        '';
      };
    in rec {
      packages.default = pkgs.clangStdenv.mkDerivation {
        name = "tromp-diagrams";
        src = ./.;

        buildInputs = with pkgs; [
          raylib
        ];

        nativeBuildInputs = with pkgs; [
          bear
          nob
        ];

        installPhase = ''
          mkdir -p $out/bin/
          cp tromp-diagrams $out/bin/
        '';
      };

      devShell = pkgs.mkShell {
        inputsFrom = [ packages.default ];
        packages = with pkgs; [ clang-tools_19 clang gdb ];
      };
    }
  );
}
