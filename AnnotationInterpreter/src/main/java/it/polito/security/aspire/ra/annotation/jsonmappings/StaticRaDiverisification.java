package it.polito.security.aspire.ra.annotation.jsonmappings;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class StaticRaDiverisification {

	@SerializedName("version")
	@Expose
	private Integer version;
	@SerializedName("label")
	@Expose
	private String label;
	@SerializedName("blocks")
	@Expose
	private Blocks blocks;

	/**
	 *
	 * @return The version
	 */
	public Integer getVersion() {
		return version;
	}

	/**
	 *
	 * @param version
	 *            The version
	 */
	public void setVersion(Integer version) {
		this.version = version;
	}

	/**
	 *
	 * @return The label
	 */
	public String getLabel() {
		return label;
	}

	/**
	 *
	 * @param label
	 *            The label
	 */
	public void setLabel(String label) {
		this.label = label;
	}

	/**
	 *
	 * @return The blocks
	 */
	public Blocks getBlocks() {
		return blocks;
	}

	/**
	 *
	 * @param blocks
	 *            The blocks
	 */
	public void setBlocks(Blocks blocks) {
		this.blocks = blocks;
	}

}