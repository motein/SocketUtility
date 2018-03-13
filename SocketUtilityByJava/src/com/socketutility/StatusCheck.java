package com.socketutility;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;

public class StatusCheck {
	
	public static boolean isLaunched(String processName) {
		try {
			Process process = Runtime.getRuntime().exec("tasklist /NH /fi \"IMAGENAME eq " + processName + "\"");
			InputStream inputStream = process.getInputStream();
			BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, "UTF-8"));
			if (reader.lines().count() > 0)
				return true;
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		return false;
	}
	
	public static boolean isPortOccupied(int port) {
		try {
			Process process = Runtime.getRuntime().exec("CMD /C netstat -ano | findstr :" + port);
			InputStream inputStream = process.getInputStream();
			BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream, "UTF-8"));
			if (reader.lines().count() > 0)
				return true;
		} catch (IOException e) {
			e.printStackTrace();
		}
		return false;
	}
}
