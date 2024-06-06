with import <nixpkgs> {};

pkgs.mkShellNoCC {
	name = "coreboot-devshell-i386";

	packages = [
		cacert
		gdb
		git
		qemu
	];

	buildInputs = [
		ncurses
		openssl
		libuuid # EDK2
	];

	nativeBuildInputs = [
		coreboot-toolchain.i386
		pkg-config
		openssh
		bison flex # U-Boot
		gnat # Fix https://github.com/NixOS/nixpkgs/issues/142943 for EDK2
	];

	shellHook = ''
		# In Nix, stdenv sets a STRIP environment variable, which has conflict
		# with libpayload/Makefile.payload. Unset the variable.
		unset STRIP
	'';
}
