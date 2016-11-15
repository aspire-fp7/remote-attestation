package it.polito.security.aspire.ra.annotation.staticra;

public enum NonceGenearationBlock implements StaticRaBlock {

	NG_1;

	public String toMakefileVariable;

	static {
		NG_1.toMakefileVariable = "ra_nonce_generation";
	}

	private final static String MAKE_FILE_VARIABLE_NAME = "RA_NONCE_GENERATION_BLOCK_NAME";

	public String toString4Makefile() {
		return MAKE_FILE_VARIABLE_NAME + " := " + this.toMakefileVariable;
	}

	public String toStringFilenameC() {
		return this.toMakefileVariable + ".c";
	}
}