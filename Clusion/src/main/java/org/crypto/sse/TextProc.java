/** * Copyright (C) 2016 Tarik Moataz
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//***********************************************************************************************//

/////////////////////    Text Parsing with a new partitioning technique 	/////////////////////////////

//***********************************************************************************************//
package org.crypto.sse;

import java.io.File;

import com.google.common.collect.Multimap;

public class TextProc {

	public TextProc(int i) {

	}

	public static void process(boolean flag, String pwd) throws Exception {
		int counter = 0;

		// ***********************************************************************************************//

		///////////////////// TEXT PARSING and Inverted Index CREATION
		///////////////////// /////////////////////////////

		// ***********************************************************************************************//

		System.out.println("\nBeginning of text extraction\n");

		File[] listOfFile = listf(pwd);
		try {
			TextExtractPar.extractTextPar(listOfFile);
		} catch (Exception e) {
			e.printStackTrace();
		}

		// ***********************************************************************************************//

		///////////////////// Partitioning /////////////////////////////

		// ***********************************************************************************************//
		if (flag) {
			Multimap<Integer, String> partitions = Partition.partitioning(TextExtractPar.lp1);
		}
	}

	/*
	 * This method gets all files from a directory. These files, will be
	 * processed later on to get all the keywords and create an inverted index
	 * structure
	 */
	public static File[] listf(String directoryName) {
		return new File(directoryName).listFiles();
	}

}
