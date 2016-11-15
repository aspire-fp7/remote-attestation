package it.polito.security.aspire.ra.annotation.staticra;

public class StaticRaRegion {

	String attestatorName;
	private boolean attestAtStartUp;

	public String getAttestatorName() {
		return attestatorName;
	}

	public void setAttestatorName(String attestatorName) {
		this.attestatorName = attestatorName;
	}

	public boolean isAttestAtStartUp() {
		return attestAtStartUp;
	}

	public void setAttestAtStartUp(boolean attestaAtStartUp) {
		this.attestAtStartUp = attestaAtStartUp;
	}
	
}
