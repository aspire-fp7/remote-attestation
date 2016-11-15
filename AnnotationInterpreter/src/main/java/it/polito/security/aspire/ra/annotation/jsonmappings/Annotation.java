package it.polito.security.aspire.ra.annotation.jsonmappings;

import java.util.ArrayList;
import java.util.List;
import com.google.gson.annotations.Expose;
import com.google.gson.annotations.JsonAdapter;
import com.google.gson.annotations.SerializedName;

public class Annotation {

	@SerializedName("file name")
	@Expose
	private String fileName;
	
	@SerializedName("line number")
	//@JsonAdapter(LineNumberJsonAdapter.class)
	@Expose(deserialize=false, serialize = false)
	private Object lineNumber ;
	
	
	@SerializedName("annotation type")
	@Expose
	private String annotationType;
	@SerializedName("annotation content")
	@Expose
	private String annotationContent;

	/**
	 *
	 * @return The fileName
	 */
	public String getFileName() {
		return fileName;
	}

	/**
	 *
	 * @param fileName
	 *            The file name
	 */
	public void setFileName(String fileName) {
		this.fileName = fileName;
	}


	/**
	 *
	 * @return The annotationType
	 */
	public String getAnnotationType() {
		return annotationType;
	}

	/**
	 *
	 * @param annotationType
	 *            The annotation type
	 */
	public void setAnnotationType(String annotationType) {
		this.annotationType = annotationType;
	}

	/**
	 *
	 * @return The annotationContent
	 */
	public String getAnnotationContent() {
		return annotationContent;
	}

	/**
	 *
	 * @param annotationContent
	 *            The annotation content
	 */
	public void setAnnotationContent(String annotationContent) {
		this.annotationContent = annotationContent;
	}

}