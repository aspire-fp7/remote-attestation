package it.polito.security.aspire.ra.annotation;

import java.util.logging.Logger;

public class Utils {
	
	public static Logger getLogger(){
		return Logger.getLogger(Thread.currentThread().getStackTrace()[1].getClass().getSimpleName());
	}
}
