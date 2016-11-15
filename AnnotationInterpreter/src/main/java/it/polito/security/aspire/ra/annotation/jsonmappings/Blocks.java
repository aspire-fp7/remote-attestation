package it.polito.security.aspire.ra.annotation.jsonmappings;

import com.google.gson.annotations.Expose;
import com.google.gson.annotations.SerializedName;

public class Blocks {

	@SerializedName("RW")
	@Expose
	private Integer RW;
	@SerializedName("HF")
	@Expose
	private Integer HF;
	@SerializedName("NI")
	@Expose
	private Integer NI;
	@SerializedName("NG")
	@Expose
	private Integer NG;
	@SerializedName("MA")
	@Expose
	private Integer MA;
	@SerializedName("DS")
	@Expose
	private Integer DS;

	/**
	 *
	 * @return The RW
	 */
	public Integer getRW() {
		return RW;
	}

	/**
	 *
	 * @param RW
	 *            The RW
	 */
	public void setRW(Integer RW) {
		this.RW = RW;
	}

	/**
	 *
	 * @return The HF
	 */
	public Integer getHF() {
		return HF;
	}

	/**
	 *
	 * @param HF
	 *            The HF
	 */
	public void setHF(Integer HF) {
		this.HF = HF;
	}

	/**
	 *
	 * @return The NI
	 */
	public Integer getNI() {
		return NI;
	}

	/**
	 *
	 * @param NI
	 *            The NI
	 */
	public void setNI(Integer NI) {
		this.NI = NI;
	}

	/**
	 *
	 * @return The NG
	 */
	public Integer getNG() {
		return NG;
	}

	/**
	 *
	 * @param NG
	 *            The NG
	 */
	public void setNG(Integer NG) {
		this.NG = NG;
	}

	/**
	 *
	 * @return The MA
	 */
	public Integer getMA() {
		return MA;
	}

	/**
	 *
	 * @param MA
	 *            The MA
	 */
	public void setMA(Integer MA) {
		this.MA = MA;
	}

	/**
	 *
	 * @return The DS
	 */
	public Integer getDS() {
		return DS;
	}

	/**
	 *
	 * @param DS
	 *            The DS
	 */
	public void setDS(Integer DS) {
		this.DS = DS;
	}

}