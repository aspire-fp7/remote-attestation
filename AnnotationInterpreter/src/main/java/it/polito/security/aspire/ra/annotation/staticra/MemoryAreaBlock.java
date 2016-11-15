package it.polito.security.aspire.ra.annotation.staticra;

public enum MemoryAreaBlock implements StaticRaBlock {

	MA_1;

	public String toMakefileVariable;

	static {
		MA_1.toMakefileVariable = "ra_memory";
	}

	private final static String MAKE_FILE_VARIABLE_NAME = "RA_MEMORY_BLOCK_NAME";

	public String toString4Makefile() {
		return MAKE_FILE_VARIABLE_NAME + " := " + this.toMakefileVariable;
	}

	public String toStringFilenameC() {
		return this.toMakefileVariable + ".c";
	}
}