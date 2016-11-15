package it.polito.security.aspire.ra.annotation;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeMap;
import java.util.TreeSet;

import com.google.gson.Gson;
import com.google.gson.JsonIOException;
import com.google.gson.JsonSyntaxException;
import com.google.gson.stream.MalformedJsonException;

import it.polito.security.aspire.ra.annotation.exceptions.InconsistentRemoteAttestationExceptiopn;
import it.polito.security.aspire.ra.annotation.exceptions.MalformedDiversifiedVersions;
import it.polito.security.aspire.ra.annotation.exceptions.MalformedRaAnnotation;
import it.polito.security.aspire.ra.annotation.exceptions.RemoteAttestationExportException;
import it.polito.security.aspire.ra.annotation.exceptions.UnknownParameter;
import it.polito.security.aspire.ra.annotation.jsonmappings.Annotation;
import it.polito.security.aspire.ra.annotation.jsonmappings.StaticRaDiverisification;
import it.polito.security.aspire.ra.annotation.staticra.DataStructuresBlock;
import it.polito.security.aspire.ra.annotation.staticra.HashFunctionBlock;
import it.polito.security.aspire.ra.annotation.staticra.MemoryAreaBlock;
import it.polito.security.aspire.ra.annotation.staticra.NonceGenearationBlock;
import it.polito.security.aspire.ra.annotation.staticra.NonceInterpretationBlock;
import it.polito.security.aspire.ra.annotation.staticra.RandomWalkBlock;
import it.polito.security.aspire.ra.annotation.staticra.StaticRa;
import it.polito.security.aspire.ra.annotation.staticra.StaticRaCommand;
import it.polito.security.aspire.ra.annotation.staticra.StaticRaRegion;
import it.polito.security.aspire.ra.annotation.staticra.StaticRaRegionCommand;

public class RaAnnotationInterpreter {

	private static final String RA_PROTECTION_NAME = "remote_attestation";

	public static void main(String[] args) {

		// Logger log = Logger.getLogger(Parser.class.getName());

		if (args.length == 0) {
			usage();
			System.exit(1);
		}

		String command = args[0];
		try {

			String inputFile;
			String outputFile;
			if (command.equals("parseAnnotations")) {
				if (args.length < 3) {
					usage();
					System.exit(1);
				}
				inputFile = args[1];
				outputFile = args[2];
				annotationsProcessor(inputFile, outputFile, false, null);
			} else if (command.equals("staticRaDiverisify")) {
				if (args.length < 3) {
					usage();
					System.exit(1);
				}
				inputFile = args[1];
				outputFile = args[2];
				diverisifyStaticRa(inputFile, outputFile);
			} else if (command.equals("parseAnnotationsCompact")) {
				if (args.length < 4) {
					usage();
					System.exit(1);
				}
				inputFile = args[1];
				outputFile = args[2];
				String divesificationDescriptorPath = args[3];
				annotationsProcessor(inputFile, outputFile, true, new File(divesificationDescriptorPath));

			} else if (command.equals("staticAttestator2BlocksFile")) {
				if (args.length < 3) {
					usage();
					System.exit(1);
				}
				String attestatorName = args[1];
				outputFile = args[2];
				StaticRa ra = new StaticRa();
				try {
					ra.ExpandUniqueId(Integer.parseInt(attestatorName.replace("attestator_", "")));
				} catch (NumberFormatException e) {
					Utils.getLogger().severe("Attestator name unknown: " + attestatorName);
					System.exit(1);
				}
				ra.export(outputFile);

			} else {
				Utils.getLogger().severe("Unknown command: " + command);
				usage();
				System.exit(1);
			}
		} catch (MalformedRaAnnotation e) {
			Utils.getLogger().severe("Malformed annotation " + e.getMessage());
			System.exit(1);
		} catch (FileNotFoundException e) {
			Utils.getLogger().severe("File not found: " + e.getMessage());
			System.exit(1);
		} catch (RemoteAttestationExportException e) {
			Utils.getLogger().severe("Error parsing JSON annotation file " + e.getMessage());
			System.exit(1);
		} catch (MalformedDiversifiedVersions e) {
			Utils.getLogger().severe("Export error " + e.getMessage());
			System.exit(1);
		} catch (InconsistentRemoteAttestationExceptiopn e) {
			Utils.getLogger().severe("Annotations are inconsistent " + e.getMessage());
			System.exit(1);
		}
	}

	private static void diverisifyStaticRa(String inputFile, String outputFile) throws RemoteAttestationExportException, FileNotFoundException, MalformedDiversifiedVersions {

		try {
			StaticRaDiverisification[] versions = (new Gson()).fromJson(new FileReader(inputFile), StaticRaDiverisification[].class);
			for (StaticRaDiverisification version : versions) {

				StaticRa ra = new StaticRa(version);
				Utils.getLogger().info(ra.toString());

				ra.exportDiversified(new File(outputFile));

			}
		} catch (JsonIOException e) {
			throw new MalformedDiversifiedVersions();
		} catch (JsonSyntaxException e) {
			throw new MalformedDiversifiedVersions();
		}
	}

	private static void annotationsProcessor(String inputFile, String outputFile, boolean isCompactRequired, File divesificationDescriptor)
			throws FileNotFoundException, MalformedRaAnnotation, RemoteAttestationExportException, MalformedDiversifiedVersions, InconsistentRemoteAttestationExceptiopn {

		Map<String, RemoteAttestation> neededRas = new TreeMap<String, RemoteAttestation>();
		List<StaticRaRegion> staticRaRegions = new ArrayList<StaticRaRegion>();

		Utils.getLogger().info("Annotation processor started");

		try {
			Annotation[] annotations = (new Gson()).fromJson(new FileReader(inputFile), Annotation[].class);
			for (Annotation annotation : annotations) {

				String[] protections = annotation.getAnnotationContent().replaceAll(" ", "").split("(,)*protection(\\()*");
				
				for (String protection : protections) {
					if (protection.isEmpty() || !protection.startsWith(RA_PROTECTION_NAME))
						continue;

					Utils.getLogger().info("Remote attestation protection annotation found: " + protection);
					String parameters = protection.replaceFirst(RA_PROTECTION_NAME + ",", "");

					RemoteAttestation actualRa = null;

					int placeBracket = parameters.indexOf("(");
					int placeComma = parameters.indexOf(",");
					int place;

					if (placeBracket == -1 && placeComma == -1) {
						throw new MalformedRaAnnotation("Malformed parameters string: " + parameters);
					}

					if (placeBracket != -1 && placeComma != -1)
						place = placeBracket < placeComma ? placeBracket : placeComma;
					else {
						place = placeBracket;
						if (place != -1)
							place = placeComma;
					}

					RaMainParameter raMainParameter;
					try {
						raMainParameter = RaMainParameter.valueOf(parameters.substring(0, place));
					} catch (IllegalArgumentException e) {
						throw new MalformedRaAnnotation("Unknown RA main parameter: " + parameters.substring(0, place));
					}

					switch (raMainParameter) {

					case static_ra:
						actualRa = processStaticRaAnnotation(parameters);
						break;
					case static_ra_region:
						staticRaRegions.add(processStaticRaRegionAnnotation(parameters));

					case SWATT_based_ra:
					case dynamic_ra:
					case hashBased_ra:
					case implicit_ra:
					case invariants_monitoring:
					case temporal_ra:
						break;
					}

					if (actualRa != null) {
						if (neededRas.put(actualRa.getLabel(), actualRa) != null)
							throw new MalformedRaAnnotation("Multiple definition of same Remote Attestation annotation");
					}

				}
			}

		} catch (JsonIOException e) {
			throw new MalformedRaAnnotation(e.getMessage());
		} catch (JsonSyntaxException e) {
			throw new MalformedRaAnnotation(e.getMessage());
		}

		for (StaticRaRegion currentRaRegion : staticRaRegions) {
			RemoteAttestation ra = neededRas.get(currentRaRegion.getAttestatorName());
			if (ra == null) {
				throw new MalformedRaAnnotation("Static RA region associated to a non defined attestator label: " + currentRaRegion.getAttestatorName());
			}
			if (!(ra instanceof StaticRa)) {
				throw new MalformedRaAnnotation("Static RA region associated to a non StaticRA attestator: " + currentRaRegion.getAttestatorName());
			}
			((StaticRa) ra).getRegionsToAttest().add(currentRaRegion);
		}

		for (RemoteAttestation ra : neededRas.values()) {
			ra.checkConsistency();
		}

		if (isCompactRequired) {
			File output = new File(outputFile);

			PrintStream w;
			w = new PrintStream(new FileOutputStream(output));

			for (RemoteAttestation rA : neededRas.values()) {
				Utils.getLogger().info(rA.toString());
				w.println(rA.toCompactExport(divesificationDescriptor));
			}
			w.close();

		} else {
			Utils.getLogger().info("Extracted RAs");
			for (RemoteAttestation rA : neededRas.values()) {
				Utils.getLogger().info(rA.toString());
				rA.export(new File(outputFile));
			}
		}

	}

	@SuppressWarnings({ "unchecked", "rawtypes" })
	private static StaticRa processStaticRaAnnotation(String parameters) throws MalformedRaAnnotation {

		Class[] parEnumsClasses = { RandomWalkBlock.class, HashFunctionBlock.class, NonceInterpretationBlock.class, NonceGenearationBlock.class, DataStructuresBlock.class, MemoryAreaBlock.class };

		StaticRa staticRa = new StaticRa();

		// extract static_ra(...) parameters
		String[] staticRaParameters = parameters.substring(0, parameters.indexOf(")") + 1).replaceAll(" ", "").replaceAll(RaMainParameter.static_ra.name(), "").replaceAll("[)(]", "").split(",");

		// update received annotation string i.e. remove static_ra(...) part
		parameters = parameters.substring(parameters.indexOf(")") + 2, parameters.length());

		// elaborate static_ra(...) "command"
		if (staticRaParameters.length == 1 && staticRaParameters[0].matches("[0-9]+")) {

			int id = Integer.parseInt(staticRaParameters[0]);

			try {
				staticRa.ExpandUniqueId(id);
			} catch (MalformedRaAnnotation e) {
				throw e;
			}
		} else {

			for (String par : staticRaParameters) {
				int i;
				for (i = 0; i < parEnumsClasses.length; i++) {

					Class parType = parEnumsClasses[i];
					try {
						staticRa.setField(Enum.valueOf(parType, par));
						break;
					} catch (IllegalArgumentException e) {
					} catch (UnknownParameter e) {
						throw new MalformedRaAnnotation("Unknown parameter: " + par);
					}
				}
				if (i == parEnumsClasses.length)
					throw new MalformedRaAnnotation("Unknown parameter: " + par);
			}
		}

		boolean exit = false;
		while (!exit) {
			int placeBracket = parameters.indexOf("(");
			int placeComma = parameters.indexOf(",");
			int place;

			if (placeBracket != -1 || placeComma != -1) {

				if (placeBracket != -1 && placeComma != -1) {
					place = placeBracket < placeComma ? placeBracket : placeComma;
				} else {
					place = placeBracket;
					if (place == -1) {
						place = placeComma;
					}
				}

				StaticRaCommand staticRaCommand = StaticRaCommand.frequency;
				try {

					try {
						staticRaCommand = StaticRaCommand.valueOf(parameters.substring(0, place));
					} catch (IndexOutOfBoundsException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}

					int toRemove = 0;
					switch (staticRaCommand) {

					case frequency:
						toRemove = processFrequencyCommand(parameters, staticRa);
						break;
					case label:
						toRemove = processLabelCommand(parameters, staticRa);
						break;
					}
					parameters = parameters.substring(toRemove, parameters.length());

				} catch (IllegalArgumentException e) {
					throw new MalformedRaAnnotation("Unknown RA main parameter: " + parameters.substring(0, place));
				}
			} else {
				exit = true;
			}
		}

		if (!parameters.isEmpty()) {
			throw new MalformedRaAnnotation("Cannot process static ra parameters: \"" + parameters + "\"");
		}

		return staticRa;
	}

	private static StaticRaRegion processStaticRaRegionAnnotation(String parameters) throws MalformedRaAnnotation {


		parameters = parameters.replaceAll(" ", "").replaceFirst(RaMainParameter.static_ra_region.name() + ",", "");
		StaticRaRegion region = new StaticRaRegion();

		boolean exit = false;
		while (!exit) {
			int placeBracket = parameters.indexOf("(");
			int placeComma = parameters.indexOf(",");
			int place;

			if (placeBracket != -1 || placeComma != -1) {

				if (placeBracket != -1 && placeComma != -1) {
					place = placeBracket < placeComma ? placeBracket : placeComma;
				} else {
					place = placeBracket;
					if (place == -1) {
						place = placeComma;
					}
				}

				StaticRaRegionCommand staticRaRegionCommand = StaticRaRegionCommand.attestator;
				try {

					try {
						staticRaRegionCommand = StaticRaRegionCommand.valueOf(parameters.substring(0, place));
					} catch (IndexOutOfBoundsException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}

					int toRemove = 0;
					switch (staticRaRegionCommand) {

					case attest_at_startup:
						toRemove = processAttestAtStartCommand(parameters, region);
						break;
					case attestator:
						toRemove = processAttestatorCommand(parameters, region);
						break;
					}
					parameters = parameters.substring(toRemove, parameters.length());

				} catch (IllegalArgumentException e) {
					throw new MalformedRaAnnotation("Unknown RA main parameter: " + parameters.substring(0, place));
				}
			} else {
				exit = true;
			}
		}

		if (!parameters.isEmpty()) {
			throw new MalformedRaAnnotation("Cannot process static ra parameters: \"" + parameters + "\"");
		}
		return region;

	}

	private static int processAttestAtStartCommand(String containAaST, StaticRaRegion region) throws MalformedRaAnnotation {
		/*
		 * check multiple AAST definition
		 */
		if (countOccurrences(containAaST.replaceAll("(.*)", ""), StaticRaRegionCommand.attest_at_startup.name()) > 1) {
			throw new MalformedRaAnnotation("'" + StaticRaRegionCommand.attest_at_startup.name() + "'" + " parameter defined more than once");
		}

		String aastFirstCmd = containAaST.substring(0, containAaST.indexOf(")") + 2);

		String AASTParameter = aastFirstCmd.replaceAll(StaticRaRegionCommand.attest_at_startup.name(), "").replaceAll("[)(,]", "");

		if (AASTParameter.isEmpty()) {
			throw new MalformedRaAnnotation("'" + StaticRaRegionCommand.attest_at_startup.name() + "'" + " parameter is empty");
		}
		
		/* If string is not interpreted the region is assumed with attest_at_startup=false */
		region.setAttestAtStartUp(Boolean.parseBoolean(AASTParameter));

		return aastFirstCmd.length();

	}

	private static int processAttestatorCommand(String containAttestator, StaticRaRegion region) throws MalformedRaAnnotation {
		/*
		 * check multiple attestator definition
		 */
		if (countOccurrences(containAttestator.replaceAll("(.*)", ""), StaticRaRegionCommand.attestator.name()) > 1) {
			throw new MalformedRaAnnotation("'" + StaticRaRegionCommand.attestator.name() + "'" + " parameter defined more than once");
		}
		String attestatorFirstCmd = containAttestator.substring(0, containAttestator.indexOf(")") + 2);

		String attestatorParameter = attestatorFirstCmd.replaceFirst(StaticRaRegionCommand.attestator.name(), "").replaceAll("[)(,]", "");

		if (attestatorParameter.isEmpty()) {
			throw new MalformedRaAnnotation("'" + StaticRaRegionCommand.attestator.name() + "'" + " parameter is empty");
		}
		region.setAttestatorName(attestatorParameter);

		return attestatorFirstCmd.length();

	}

	private static int processLabelCommand(String containLabel, RemoteAttestation actualRa) throws MalformedRaAnnotation {
		/*
		 * check multiple label definition
		 */
		if (countOccurrences(containLabel.replaceAll("(.*)", ""), StaticRaCommand.label.name()) > 1) {
			throw new MalformedRaAnnotation("'" + StaticRaCommand.label.name() + "'" + " parameter defined more than once");
		}

		String labelFirstCmd = containLabel.substring(0, containLabel.indexOf(")") + 2);

		String labelParameter = labelFirstCmd.replaceFirst(StaticRaCommand.label.name(), "").replaceAll("[)(,]", "");

		if (labelParameter.isEmpty()) {
			throw new MalformedRaAnnotation("'" + StaticRaCommand.label.name() + "'" + " parameter is empty");
		}
		actualRa.setLabel(labelParameter);

		return labelFirstCmd.length();

	}

	private static int processFrequencyCommand(String containFrequency, RemoteAttestation actualRa) throws MalformedRaAnnotation {
		/*
		 * check multiple frequency definition
		 */
		if (countOccurrences(containFrequency.replaceAll("(.*)", ""), StaticRaCommand.frequency.name()) > 1) {
			throw new MalformedRaAnnotation("'" + StaticRaCommand.frequency.name() + "'" + " parameter defined more than once");
		}

		String frequencyFirstCmd = containFrequency.substring(0, containFrequency.indexOf(")") + 2);

		String frequencyParameter = frequencyFirstCmd.replaceAll(StaticRaCommand.frequency.name(), "").replaceAll("[)(,]", "");

		if (frequencyParameter.isEmpty()) {
			throw new MalformedRaAnnotation("'" + StaticRaCommand.frequency.name() + "'" + " parameter is empty");
		}
		
		try {
			actualRa.setFrequency(Integer.parseInt(frequencyParameter));
		} catch (NumberFormatException e) {
			throw new MalformedRaAnnotation(StaticRaCommand.frequency + " command has a non Integer parameter: " + frequencyParameter);
		}

		return frequencyFirstCmd.length();

	}

	public static int countOccurrences(String string, String occurrence) {
		return (string.length() - string.replace(occurrence, "").length()) / (occurrence).length();
	}

	private static void usage() {
		Utils.getLogger().info("Usage:");
		Utils.getLogger().info("java -jar <jar-name> <command> <input-entity> <output-entity> <parameter-1>");
		Utils.getLogger().info("  * jar-name    the name of this jar");
		Utils.getLogger().info("  * command     this specifies the operation to be performed");
		Utils.getLogger().info("                the next parametres meaning depend on command");
		Utils.getLogger().info("                supported commands are:");
		Utils.getLogger().info("                - parseAnnotations   : parses annotations and gets make file include style output");
		Utils.getLogger().info("                                       one file for each labeled attestator");
		Utils.getLogger().info("                  * input-enity      : path to the JSON file containing ASPIRE extracted annoatations");
		Utils.getLogger().info("                  * output-entity    : path to the folder where produced files must be put");
		Utils.getLogger().info("                  * parameter-1      : ignored");
		Utils.getLogger().info("");
		Utils.getLogger().info("                - staticRaDiverisify : parses Diversification descriptor JSON file and gets,");
		Utils.getLogger().info("                                       one file per each diversified version which indicates the");
		Utils.getLogger().info("                                       blocks src files needed to create the proper libblocks.a library");
		Utils.getLogger().info("                  * input-enity      : path to the Diversification descriptor JSON file");
		Utils.getLogger().info("                  * output-entity    : path to the folder where produced files must be put");
		Utils.getLogger().info("                  * parameter-1      : ignored");
		Utils.getLogger().info("");
		Utils.getLogger().info("                - parseAnnotationsCompact : same of 'parseAnnotations' but produces a single file");
		Utils.getLogger().info("                                            the diversified version name of the attestators");
		Utils.getLogger().info("                  * input-enity           : path to the JSON file containing ASPIRE extracted annoatations");
		Utils.getLogger().info("                  * output-entity         : path to the file to be filled with output names");
		Utils.getLogger().info("                  * parameter-1           : path to the diversification descriptor JSON file");
		Utils.getLogger().info("");
		Utils.getLogger().info("                - staticAttestator2BlocksFile : transforms the given name of an attestator diversified version ");
		Utils.getLogger().info("                                                into the relative static RA blocks src file");
		Utils.getLogger().info("                  * input-enity               : attestator version name (attestator_[1-40])");
		Utils.getLogger().info("                  * output-entity             : path where produced file must be put");
		Utils.getLogger().info("                  * parameter-1               : ignored");

	}

}
