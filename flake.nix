{

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-25.11";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
    in
  {
    devShells.${system}.default = pkgs.mkShell {
      buildInputs = with pkgs; [
        gcc
      ];
      shellHook = ''
        echo -e "\033[32mDone\033[0m"
      '';
    };
  };

}
