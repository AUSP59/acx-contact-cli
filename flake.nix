{
  description = "ACX Enterprise Contact CLI dev shell";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.05";
  outputs = { self, nixpkgs }:
  let pkgs = nixpkgs.legacyPackages.x86_64-linux; in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = with pkgs; [ cmake ninja gcc gdb clang-tools gcovr doxygen graphviz cppcheck python3 ];
    };
  };
}
