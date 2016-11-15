package it.polito.security.aspire.ra.annotation.staticra;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.PrintStream;
import java.io.Reader;
import java.lang.reflect.Field;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import com.google.gson.Gson;
import com.google.gson.JsonIOException;
import com.google.gson.JsonSyntaxException;

import it.polito.security.aspire.ra.annotation.RemoteAttestation;
import it.polito.security.aspire.ra.annotation.Utils;
import it.polito.security.aspire.ra.annotation.exceptions.InconsistentRemoteAttestationExceptiopn;
import it.polito.security.aspire.ra.annotation.exceptions.MalformedDiversifiedVersions;
import it.polito.security.aspire.ra.annotation.exceptions.MalformedRaAnnotation;
import it.polito.security.aspire.ra.annotation.exceptions.RemoteAttestationExportException;
import it.polito.security.aspire.ra.annotation.exceptions.UnknownParameter;
import it.polito.security.aspire.ra.annotation.jsonmappings.StaticRaDiverisification;

public class StaticRa extends RemoteAttestation {

	// private static final Logger LOGGER =
	// Logger.getLogger(RaAnnotationInterpreter.class.getName());

	private RandomWalkBlock randomWalkBlock;
	private HashFunctionBlock hashFunctionBlock;
	private NonceInterpretationBlock nonceInterpretationBlock;
	private NonceGenearationBlock nonceGenearationBlock;
	private DataStructuresBlock dataStructuresBlock;
	private MemoryAreaBlock memoryAreaBlock;
	private List<StaticRaRegion> regionsToAttest;
	private boolean niAldoEnabled;

	public StaticRa() {

		super();
		this.randomWalkBlock = RandomWalkBlock.values()[0];
		this.hashFunctionBlock = HashFunctionBlock.values()[0];
		this.nonceInterpretationBlock = NonceInterpretationBlock.values()[0];
		this.nonceGenearationBlock = NonceGenearationBlock.values()[0];
		this.dataStructuresBlock = DataStructuresBlock.values()[0];
		this.memoryAreaBlock = MemoryAreaBlock.values()[0];
		this.regionsToAttest = new ArrayList<StaticRaRegion>();
		this.niAldoEnabled = false;
	}

	public StaticRa(RandomWalkBlock randomWalk, HashFunctionBlock hashFunctionBlock, NonceInterpretationBlock nonceInterpretationBlock, NonceGenearationBlock nonceGenearationBlock,
			DataStructuresBlock dataStructuresBlock, MemoryAreaBlock memoryAreaBlock) {
		super();
		this.randomWalkBlock = randomWalk;
		this.hashFunctionBlock = hashFunctionBlock;
		this.nonceInterpretationBlock = nonceInterpretationBlock;
		this.nonceGenearationBlock = nonceGenearationBlock;
		this.dataStructuresBlock = dataStructuresBlock;
		this.memoryAreaBlock = memoryAreaBlock;
		this.regionsToAttest = new ArrayList<StaticRaRegion>();
		this.niAldoEnabled = false;
	}

	protected StaticRa(int randomWalk, int hashFunctionBlock, int nonceInterpretationBlock, int nonceGenearationBlock, int dataStructuresBlock, int memoryAreaBlock) {

		super();
		this.randomWalkBlock = RandomWalkBlock.values()[randomWalk];
		this.hashFunctionBlock = HashFunctionBlock.values()[hashFunctionBlock];
		this.nonceInterpretationBlock = NonceInterpretationBlock.values()[nonceInterpretationBlock];
		this.nonceGenearationBlock = NonceGenearationBlock.values()[nonceGenearationBlock];
		this.dataStructuresBlock = DataStructuresBlock.values()[dataStructuresBlock];
		this.memoryAreaBlock = MemoryAreaBlock.values()[memoryAreaBlock];
		this.regionsToAttest = new ArrayList<StaticRaRegion>();
		this.niAldoEnabled = false;
	}

	public StaticRa(RandomWalkBlock randomWalk, HashFunctionBlock hashFunctionBlock, NonceInterpretationBlock nonceInterpretationBlock, NonceGenearationBlock nonceGenearationBlock,
			DataStructuresBlock dataStructuresBlock, MemoryAreaBlock memoryAreaBlock, String label) {

		super();
		this.setLabel(label);
		this.randomWalkBlock = randomWalk;
		this.hashFunctionBlock = hashFunctionBlock;
		this.nonceInterpretationBlock = nonceInterpretationBlock;
		this.nonceGenearationBlock = nonceGenearationBlock;
		this.dataStructuresBlock = dataStructuresBlock;
		this.memoryAreaBlock = memoryAreaBlock;
		this.regionsToAttest = new ArrayList<StaticRaRegion>();
		this.niAldoEnabled = false;
	}

	public StaticRa(StaticRaDiverisification version) {

		super();
		this.setLabel(version.getVersion().toString());

		this.randomWalkBlock = RandomWalkBlock.values()[version.getBlocks().getRW()];
		this.hashFunctionBlock = HashFunctionBlock.values()[version.getBlocks().getHF()];
		this.nonceInterpretationBlock = NonceInterpretationBlock.values()[version.getBlocks().getNI()];
		this.nonceGenearationBlock = NonceGenearationBlock.values()[version.getBlocks().getNG()];
		this.dataStructuresBlock = DataStructuresBlock.values()[version.getBlocks().getDS()];
		this.memoryAreaBlock = MemoryAreaBlock.values()[version.getBlocks().getMA()];
		this.regionsToAttest = new ArrayList<StaticRaRegion>();
		this.niAldoEnabled = false;
	}

	protected StaticRa(int randomWalk, int hashFunctionBlock, int nonceInterpretationBlock, int nonceGenearationBlock, int dataStructuresBlock, int memoryAreaBlock, String label) {

		super();
		this.setLabel(label);
		this.randomWalkBlock = RandomWalkBlock.values()[randomWalk];
		this.hashFunctionBlock = HashFunctionBlock.values()[hashFunctionBlock];
		this.nonceInterpretationBlock = NonceInterpretationBlock.values()[nonceInterpretationBlock];
		this.nonceGenearationBlock = NonceGenearationBlock.values()[nonceGenearationBlock];
		this.dataStructuresBlock = DataStructuresBlock.values()[dataStructuresBlock];
		this.memoryAreaBlock = MemoryAreaBlock.values()[memoryAreaBlock];
		this.regionsToAttest = new ArrayList<StaticRaRegion>();
		this.niAldoEnabled = false;
	}

	/* Getters ad setters */

	public RandomWalkBlock getRandomWalk() {
		return randomWalkBlock;
	}

	public void setRandomWalk(String randomWalk) {
		this.randomWalkBlock = RandomWalkBlock.valueOf(randomWalk);
	}

	public HashFunctionBlock getHashFunctionBlock() {
		return hashFunctionBlock;
	}

	public void setHashFunctionBlock(String hashFunctionBlock) {
		this.hashFunctionBlock = HashFunctionBlock.valueOf(hashFunctionBlock);
	}

	public NonceInterpretationBlock getNonceInterpretationBlock() {
		return nonceInterpretationBlock;
	}

	public void setNonceInterpretationBlock(String nonceInterpretationBlock) {
		this.nonceInterpretationBlock = NonceInterpretationBlock.valueOf(nonceInterpretationBlock);
	}

	public NonceGenearationBlock getNonceGenearationBlock() {
		return nonceGenearationBlock;
	}

	public void setNonceGenearationBlock(String nonceGenearationBlock) {
		this.nonceGenearationBlock = NonceGenearationBlock.valueOf(nonceGenearationBlock);
	}

	public DataStructuresBlock getDataTableBlock() {
		return dataStructuresBlock;
	}

	public void setDataTableBlock(String dataTableBlock) {
		this.dataStructuresBlock = DataStructuresBlock.valueOf(dataTableBlock);
	}

	public MemoryAreaBlock getMemoryAreaBlock() {
		return memoryAreaBlock;
	}

	public void setMemoryAreaBlock(String memoryAreaBlock) {
		this.memoryAreaBlock = MemoryAreaBlock.valueOf(memoryAreaBlock);
	}

	public List<StaticRaRegion> getRegionsToAttest() {
		return regionsToAttest;
	}

	public void setRegionsToAttest(List<StaticRaRegion> regionsToAttest) {
		this.regionsToAttest = regionsToAttest;
	}

	public boolean isNiAldoEnabled() {
		return niAldoEnabled;
	}

	public void setNiAldoEnabled(boolean niAldoEnabled) {
		this.niAldoEnabled = niAldoEnabled;
	}

	public void setField(Enum<?> value) throws UnknownParameter {

		if (value instanceof RandomWalkBlock)
			this.randomWalkBlock = (RandomWalkBlock) value;
		else if (value instanceof HashFunctionBlock)
			this.hashFunctionBlock = (HashFunctionBlock) value;
		else if (value instanceof NonceInterpretationBlock)
			this.nonceInterpretationBlock = (NonceInterpretationBlock) value;
		else if (value instanceof NonceGenearationBlock)
			this.nonceGenearationBlock = (NonceGenearationBlock) value;
		else if (value instanceof DataStructuresBlock)
			this.dataStructuresBlock = (DataStructuresBlock) value;
		else if (value instanceof MemoryAreaBlock)
			this.memoryAreaBlock = (MemoryAreaBlock) value;
		else
			throw new UnknownParameter();
	}

	public void setAll(int randomWalk, int hashFunctionBlock, int nonceInterpretationBlock, int nonceGenearationBlock, int dataStructuresBlock, int memoryAreaBlock) {
		this.randomWalkBlock = RandomWalkBlock.values()[randomWalk];
		this.hashFunctionBlock = HashFunctionBlock.values()[hashFunctionBlock];
		this.nonceInterpretationBlock = NonceInterpretationBlock.values()[nonceInterpretationBlock];
		this.nonceGenearationBlock = NonceGenearationBlock.values()[nonceGenearationBlock];
		this.dataStructuresBlock = DataStructuresBlock.values()[dataStructuresBlock];
		this.memoryAreaBlock = MemoryAreaBlock.values()[memoryAreaBlock];

	}

	/* END Getters and setters */

	public void ExpandUniqueId(int id) throws MalformedRaAnnotation {

		switch (id) {

		/* 2, 5, 4, 1, 1, 1 */
		case 1:
			setAll(0, 0, 0, 0, 0, 0);
			return;
		case 2:
			setAll(0, 0, 1, 0, 0, 0);
			return;
		case 3:
			setAll(0, 0, 2, 0, 0, 0);
			return;
		case 4:
			setAll(0, 0, 3, 0, 0, 0);
			return;

		case 5:
			setAll(0, 1, 0, 0, 0, 0);
			return;
		case 6:
			setAll(0, 1, 1, 0, 0, 0);
			return;
		case 7:
			setAll(0, 1, 2, 0, 0, 0);
			return;
		case 8:
			setAll(0, 1, 3, 0, 0, 0);
			return;

		case 9:
			setAll(0, 2, 0, 0, 0, 0);
			return;
		case 10:
			setAll(0, 2, 1, 0, 0, 0);
			return;
		case 11:
			setAll(0, 2, 2, 0, 0, 0);
			return;
		case 12:
			setAll(0, 2, 3, 0, 0, 0);
			return;

		case 13:
			setAll(0, 3, 0, 0, 0, 0);
			return;
		case 14:
			setAll(0, 3, 1, 0, 0, 0);
			return;
		case 15:
			setAll(0, 3, 2, 0, 0, 0);
			return;
		case 16:
			setAll(0, 3, 3, 0, 0, 0);
			return;

		case 17:
			setAll(0, 4, 0, 0, 0, 0);
			return;
		case 18:
			setAll(0, 4, 1, 0, 0, 0);
			return;
		case 19:
			setAll(0, 4, 2, 0, 0, 0);
			return;
		case 20:
			setAll(0, 4, 3, 0, 0, 0);
			return;

		case 21:
			setAll(1, 0, 0, 0, 0, 0);
			return;
		case 22:
			setAll(1, 0, 1, 0, 0, 0);
			return;
		case 23:
			setAll(1, 0, 2, 0, 0, 0);
			return;
		case 24:
			setAll(1, 0, 3, 0, 0, 0);
			return;

		case 25:
			setAll(1, 1, 0, 0, 0, 0);
			return;
		case 26:
			setAll(1, 1, 1, 0, 0, 0);
			return;
		case 27:
			setAll(1, 1, 2, 0, 0, 0);
			return;
		case 28:
			setAll(1, 1, 3, 0, 0, 0);
			return;

		case 29:
			setAll(1, 2, 0, 0, 0, 0);
			return;
		case 30:
			setAll(1, 2, 1, 0, 0, 0);
			return;
		case 31:
			setAll(1, 2, 2, 0, 0, 0);
			return;
		case 32:
			setAll(1, 2, 3, 0, 0, 0);
			return;

		case 33:
			setAll(1, 3, 0, 0, 0, 0);
			return;
		case 34:
			setAll(1, 3, 1, 0, 0, 0);
			return;
		case 35:
			setAll(1, 3, 2, 0, 0, 0);
			return;
		case 36:
			setAll(1, 3, 3, 0, 0, 0);
			return;

		case 37:
			setAll(1, 4, 0, 0, 0, 0);
			return;
		case 38:
			setAll(1, 4, 1, 0, 0, 0);
			return;
		case 39:
			setAll(1, 4, 2, 0, 0, 0);
			return;
		case 40:
			setAll(1, 4, 3, 0, 0, 0);
			return;

		default:
			throw new MalformedRaAnnotation("Unknown unique static RA id: " + id);
		}

	}

	@Override
	public String toString() {
		// TODO Auto-generated method stub
		return super.toString() + "Static Ra parameters:\n" + randomWalkBlock.name() + " > " + randomWalkBlock.toMakefileVariable + "\n" + hashFunctionBlock.name() + " > "
				+ hashFunctionBlock.toMakefileVariable + "\n" + nonceInterpretationBlock.name() + " > " + nonceInterpretationBlock.toMakefileVariable + "\n" + nonceGenearationBlock.name() + " > "
				+ nonceGenearationBlock.toMakefileVariable + "\n" + dataStructuresBlock.name() + " > " + dataStructuresBlock.toMakefileVariable + "\n" + memoryAreaBlock.name() + " > "
				+ memoryAreaBlock.toMakefileVariable + "\n Attestator attests " + regionsToAttest.size() + " areas";
	}

	/*
	 * equal if same label or same blocks
	 * 
	 * @see
	 * it.polito.security.aspire.ra.annotation.RemoteAttestation#compareTo(it.
	 * polito.security.aspire.ra.annotation.RemoteAttestation)
	 */
	@Override
	public int compareTo(RemoteAttestation o) {

		if (o.getClass() == StaticRa.class) {
			if (this.getLabel().compareTo(o.getLabel()) == 0) {
				StaticRa other = (StaticRa) o;
				return Math.abs(dataStructuresBlock.compareTo(other.dataStructuresBlock)) + Math.abs(hashFunctionBlock.compareTo(other.hashFunctionBlock))
						+ Math.abs(memoryAreaBlock.compareTo(other.memoryAreaBlock)) + Math.abs(nonceGenearationBlock.compareTo(other.nonceGenearationBlock))
						+ Math.abs(nonceInterpretationBlock.compareTo(other.nonceInterpretationBlock)) + Math.abs(randomWalkBlock.compareTo(other.randomWalkBlock));
			}
		}
		return -1;
	}

	public boolean isOfDiversiefiedVersion(StaticRaDiverisification version) {
		return this.randomWalkBlock.ordinal() == version.getBlocks().getRW() && this.hashFunctionBlock.ordinal() == version.getBlocks().getHF()
				&& this.nonceInterpretationBlock.ordinal() == version.getBlocks().getNI() && this.nonceGenearationBlock.ordinal() == version.getBlocks().getNG()
				&& this.dataStructuresBlock.ordinal() == version.getBlocks().getDS() && this.memoryAreaBlock.ordinal() == version.getBlocks().getMA();
	}

	@Override
	public void export(File outputFolder) throws RemoteAttestationExportException {

		if (!outputFolder.isDirectory())
			throw new RemoteAttestationExportException(outputFolder.getName() + " is not a folder.");

		File f = new File(outputFolder.getPath() + File.separator + getLabel());
		File fFreq = new File(outputFolder.getPath() + File.separator + getLabel() + ".freq");
		try {

			PrintStream w = new PrintStream(new FileOutputStream(f));
			PrintStream wFreq = new PrintStream(new FileOutputStream(fFreq));

			for (Field field : this.getClass().getDeclaredFields()) {
				if (field.get(this) instanceof StaticRaBlock) {
					StaticRaBlock raField = (StaticRaBlock) field.get(this);
					w.println(raField.toString4Makefile());
				}
			}

			wFreq.println("" + getFrequency());

			w.close();
			wFreq.close();

		} catch (FileNotFoundException e) {
			throw new RemoteAttestationExportException("Unable to open file: " + f.getPath());
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	@Override
	public void export(String outputFilepath) {

		File f = new File(outputFilepath);
		try {

			PrintStream w = new PrintStream(new FileOutputStream(f));

			for (Field field : this.getClass().getDeclaredFields()) {
				StaticRaBlock raField = (StaticRaBlock) field.get(this);
				w.println(raField.toString4Makefile());
			}

			w.close();

		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			e.printStackTrace();
		} catch (IllegalAccessException e) {
			e.printStackTrace();
		}
	}

	public String toCompactExport(File diverisificationDescriptor) throws RemoteAttestationExportException, MalformedDiversifiedVersions, FileNotFoundException {

		try {
			// Class<StaticRa> cl = StaticRa.class;
			// URL url = cl.getResource("/resources/ldv.json");
			// String f = url.getFile();
			// FileReader fr = new FileReader(f);

			StaticRaDiverisification[] versions = (new Gson()).fromJson(new FileReader(diverisificationDescriptor), StaticRaDiverisification[].class);
			for (StaticRaDiverisification version : versions) {
				Utils.getLogger().info("Comparing to version: " + version.getVersion());
				if (this.isOfDiversiefiedVersion(version)) {
					// return this.getLabel() + " " +
					// version.getVersion().toString();
					// TODO improve
					return "attestator_" + version.getVersion().toString();
				}
			}

		} catch (JsonSyntaxException e) {
			// TODO Auto-generated catch block
			Utils.getLogger().severe("Error malformed Diversification descriptor JSON file");
			throw new MalformedDiversifiedVersions();
		} catch (JsonIOException e) {
			// TODO Auto-generated catch block
			Utils.getLogger().severe("Error I/O Diversification descriptor JSON file");
			throw new MalformedDiversifiedVersions();
		}
		throw new RemoteAttestationExportException("Unknown diversification version for the specified StaticRa: " + this.toString());
	}

	public void exportDiversified(File outputFolder) throws RemoteAttestationExportException {

		if (!outputFolder.isDirectory())
			throw new RemoteAttestationExportException(outputFolder.getName() + " is not a folder.");

		File f = new File(outputFolder.getPath() + File.separator + getLabel());
		try {

			PrintStream w = new PrintStream(new FileOutputStream(f));

			for (Field field : this.getClass().getDeclaredFields()) {
				StaticRaBlock raField = (StaticRaBlock) field.get(this);
				w.println(raField.toStringFilenameC());
			}

			w.close();

		} catch (FileNotFoundException e) {
			throw new RemoteAttestationExportException("Unable to open file: " + f.getPath());
		} catch (IllegalAccessException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}

	@Override
	public void checkConsistency() throws InconsistentRemoteAttestationExceptiopn {
		if (regionsToAttest.size() == 0) {
			throw new InconsistentRemoteAttestationExceptiopn("Static RA attestator has no associated regions");
		}
		if (nonceInterpretationBlock == NonceInterpretationBlock.NI_1 || nonceInterpretationBlock == NonceInterpretationBlock.NI_2) {
			if ((regionsToAttest.size() & (regionsToAttest.size() - 1)) != 0) {
				if (niAldoEnabled) {
					throw new InconsistentRemoteAttestationExceptiopn("Static RA attestator with '" + nonceInterpretationBlock.toString() + "' attests number of regions different from 2^n");
				} else {
					Utils.getLogger().warning("Static RA attestator with '" + nonceInterpretationBlock.toString() + "' is currently not supported");
				}
			}
		}
		return;
	}

}
