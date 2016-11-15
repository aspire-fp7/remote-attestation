package it.polito.security.aspire.ra.annotation.jsonmappings;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.TreeSet;

import com.google.gson.TypeAdapter;
import com.google.gson.stream.JsonReader;
import com.google.gson.stream.JsonWriter;

public class LineNumberJsonAdapter extends TypeAdapter<List<Integer>> {
	@Override
	public void write(JsonWriter out, List<Integer> numbers) throws IOException {
		// implement write: combine firstName and lastName into name
		out.beginObject();
		out.name("name");
		String outString = "[" + numbers.get(0);
		int i = 0;
		for (Integer integer : numbers) {
			if (i == 0)
				continue;
			outString = outString + "," + integer;
		}
		outString = outString + "]";
		out.value(outString);
		out.endObject();
		// implement the write method
	}

	@Override
	public List<Integer> read(JsonReader in) throws IOException {
		// implement read: split name into firstName and lastName
		in.beginObject();
		in.nextName();
		String[] items = in.nextString().split("[,\\[\\]]");
		in.endObject();
		List ret =  new ArrayList<Integer>();
		for (String e : items) {
			if(!e.isEmpty()){
				ret.add(e);
			}
		}
		return ret;
	}

}
