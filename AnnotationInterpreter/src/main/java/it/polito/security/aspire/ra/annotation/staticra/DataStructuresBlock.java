package it.polito.security.aspire.ra.annotation.staticra;

public enum DataStructuresBlock implements StaticRaBlock {

	DS_1;

	public String toMakefileVariable;

	static {
		DS_1.toMakefileVariable = "ra_data_table";
	}

	private final static String MAKE_FILE_VARIABLE_NAME = "RA_DATA_TABLE_BLOCK_NAME";

	public String toString4Makefile() {
		return MAKE_FILE_VARIABLE_NAME + " := " + this.toMakefileVariable;
	}

	public String toStringFilenameC() {
		return this.toMakefileVariable + ".c";
	}
}
