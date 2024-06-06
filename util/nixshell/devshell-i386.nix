with import <nixpkgs> {};

pkgs.mkShell {
	name = "coreboot-devshell-i386";

	packages = [
		cacert
		gdb
		git
		qemu
		# EDK2
		python3
	];

	buildInputs = [
		ncurses
		openssl
	];

	nativeBuildInputs = [
		coreboot-toolchain.i386
		pkg-config
		openssh
		# U-Boot
		bison
		flex
		# EDK2
		libuuid
		imagemagick
		gnat
	];

	shellHook = ''
		# In Nix, stdenv sets a STRIP environment variable, which has conflict
		# with libpayload/Makefile.payload. Unset the variable.
		unset STRIP
	'';
}
