package it.polito.security.aspire.ra.annotation.staticra;

public enum NonceInterpretationBlock implements StaticRaBlock {

	NI_1,
	NI_2,
	NI_3,
	NI_4;

	public String toMakefileVariable;

	static {
		NI_1.toMakefileVariable = "ra_nonce_interpretation_1";
		NI_2.toMakefileVariable = "ra_nonce_interpretation_2";
		NI_3.toMakefileVariable = "ra_nonce_interpretation_3";
		NI_4.toMakefileVariable = "ra_nonce_interpretation_4";
	}

	private final static String MAKE_FILE_VARIABLE_NAME = "RA_NONCE_INTERPRETATION_BLOCK_NAME";

	public String toString4Makefile() {
		return MAKE_FILE_VARIABLE_NAME + " := " + this.toMakefileVariable;
	}

	public String toStringFilenameC() {
		return this.toMakefileVariable + ".c";
	}

}