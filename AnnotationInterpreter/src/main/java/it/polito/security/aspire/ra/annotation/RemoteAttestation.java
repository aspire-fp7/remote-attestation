package it.polito.security.aspire.ra.annotation;

import java.io.File;
import java.io.FileNotFoundException;

import it.polito.security.aspire.ra.annotation.exceptions.InconsistentRemoteAttestationExceptiopn;
import it.polito.security.aspire.ra.annotation.exceptions.MalformedDiversifiedVersions;
import it.polito.security.aspire.ra.annotation.exceptions.RemoteAttestationExportException;

public abstract class RemoteAttestation implements Comparable<RemoteAttestation> {

	private static final String LABEL_DEFAUT_NAME = "default_label";
	private static final Integer FREQUENCY_DEFAULT_VALUE = 30; // in seconds

	
	private String label;
	private Integer frequency;

	public RemoteAttestation() {
		label = LABEL_DEFAUT_NAME;
		frequency = FREQUENCY_DEFAULT_VALUE;
	}

	public String getLabel() {
		return label;
	}

	public void setLabel(String label) {
		this.label = label;
	}

	public Integer getFrequency() {
		return frequency;
	}

	public void setFrequency(Integer frequency) {
		this.frequency = frequency;
	}

	@Override
	public String toString() {
		return "RA label: " + label + "\nRa frequency: " + frequency + "\n";
	}

	@Override
	public abstract int compareTo(RemoteAttestation o);

	public abstract void export(File outputFolder) throws RemoteAttestationExportException;

	public abstract void export(String outputFilepath);

	public abstract String toCompactExport(File diverisificationDescriptor) throws RemoteAttestationExportException, MalformedDiversifiedVersions, FileNotFoundException;

	public abstract void checkConsistency() throws InconsistentRemoteAttestationExceptiopn;

}