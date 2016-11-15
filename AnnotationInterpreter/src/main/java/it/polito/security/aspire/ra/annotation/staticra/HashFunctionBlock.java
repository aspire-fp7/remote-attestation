package it.polito.security.aspire.ra.annotation.staticra;

public enum HashFunctionBlock implements StaticRaBlock {

	HF_BLAKE2,
	HF_MD5,
	HF_SHA1,
	HF_SHA256,
	HF_RIPEMD160;

	public String toMakefileVariable;

	static {
		HF_BLAKE2.toMakefileVariable = "ra_do_hash_blake2";
		HF_MD5.toMakefileVariable = "ra_do_hash_md5";
		HF_SHA1.toMakefileVariable = "ra_do_hash_sha1";
		HF_SHA256.toMakefileVariable = "ra_do_hash_sha256";
		HF_RIPEMD160.toMakefileVariable = "ra_do_hash_ripemd160";
	}

	private final static String MAKE_FILE_VARIABLE_NAME = "RA_DO_HASH_BLOCK_NAME";

	public String toString4Makefile() {
		return MAKE_FILE_VARIABLE_NAME + " := " + this.toMakefileVariable;
	}

	public String toStringFilenameC() {
		return this.toMakefileVariable + ".c";
	}

}
