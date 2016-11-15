package it.polito.security.aspire.ra.annotation.staticra;

public enum RandomWalkBlock implements StaticRaBlock{

	/*
	 * Normal
	 */
	RW_NORMAL,

	/*
	 * Goldbach
	 */
	RW_GOLDBACH;

	public String toMakefileVariable;

	static {
		RW_NORMAL.toMakefileVariable = "ra_data_preparation";
		RW_GOLDBACH.toMakefileVariable = "ra_data_preparation_goldbach";
	}

	private final static String MAKE_FILE_VARIABLE_NAME = "RA_DATA_PREPARATION_BLOCK_NAME";
	
	public String toString4Makefile() {
		return MAKE_FILE_VARIABLE_NAME + " := " + this.toMakefileVariable;
	}
	public String toStringFilenameC() {
		return this.toMakefileVariable + ".c";
	}
}
