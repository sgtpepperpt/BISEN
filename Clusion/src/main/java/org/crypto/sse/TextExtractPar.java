/**
 * * Copyright (C) 2016 Tarik Moataz
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

// ***********************************************************************************************//

///////////////////// This file contains the generation of the database DB, i.e., building a plaintext look-up table that associates every keyword to
///////////////////// the set fo documents identifiers /////////////////////////////

/*
 * TEXT extractor parses the content of documents into raw text the output of this parser is given to Lucene for tokenization. The tokenization used
 * is a standard one where stop words are eliminated. A more sophisticated tokenization is possible such as Porter stemming algorithm. This part can
 * be modified to handle a more specific user grammar. The actual parser handles the following extensions: - .txt, html etc - Microsoft documents .doc
 * and .docx, EXCEEL sheet .xls and Powerpoint presentation .ppt - Pdf files .pdf - All media files such as pictures and videos are not parsed and
 * only the title of the media file is taken as input gif, jpeg, .wmv, .mpeg, .mp4
 */
// ***********************************************************************************************//

package org.crypto.sse;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.analysis.en.EnglishAnalyzer;
import org.apache.lucene.analysis.standard.StandardAnalyzer;
import org.apache.lucene.analysis.util.CharArraySet;

import com.google.common.base.Charsets;
import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.Multimap;
import com.google.common.io.Files;

public class TextExtractPar implements Serializable {
	private static final long serialVersionUID = 1L;

	public static int lengthStrings = 0;
	public static int maxTupleSize = 0;
	public static int threshold = 100;

	// lookup stores a plaintext inverted index of the dataset, i.e., the
	// association between the keyword and documents that contain the keyword

	Multimap<String, String> lookup1 = ArrayListMultimap.create();
	static Multimap<String, String> lp1 = ArrayListMultimap.create();

	// lookup2 stores the document identifier (title) and the keywords contained
	// in this document

	Multimap<String, String> lookup2 = ArrayListMultimap.create();
	static Multimap<String, String> lp2 = ArrayListMultimap.create();
	
	public static List<String> allWords = new ArrayList<String>(100000);

	static int counter = 0;

	public TextExtractPar(Multimap<String, String> lookup, Multimap<String, String> lookup2) {
		this.lookup1 = lookup;
		this.lookup2 = lookup2;
	}

	public Multimap<String, String> getL1() {
		return this.lookup1;
	}

	public Multimap<String, String> getL2() {
		return this.lookup2;
	}

	public static void extractTextPar(File[] listOfFile) throws Exception {
		int threads = 0;
		if (Runtime.getRuntime().availableProcessors() > listOfFile.length) {
			threads = listOfFile.length;
		} else {
			threads = Runtime.getRuntime().availableProcessors();
		}

		ExecutorService service = Executors.newFixedThreadPool(threads);
		List<File[]> inputs = new ArrayList<File[]>(threads);

		System.out.println("Number of Threads " + threads);

		for (int i = 0; i < threads; i++) {
			File[] tmp;
			if (i == threads - 1) {
				tmp = new File[listOfFile.length / threads + listOfFile.length % threads];
				for (int j = 0; j < listOfFile.length / threads + listOfFile.length % threads; j++) {
					tmp[j] = listOfFile[(listOfFile.length / threads) * i + j];
				}
			} else {
				tmp = new File[listOfFile.length / threads];
				for (int j = 0; j < listOfFile.length / threads; j++) {

					tmp[j] = listOfFile[(listOfFile.length / threads) * i + j];
				}
			}
			inputs.add(i, tmp);
		}

		List<Future<TextExtractPar>> futures = new ArrayList<Future<TextExtractPar>>();
		for (final File[] input : inputs) {
			Callable<TextExtractPar> callable = new Callable<TextExtractPar>() {
				public TextExtractPar call() throws Exception {
					TextExtractPar output = extractOneDoc(input);

					return output;
				}
			};
			futures.add(service.submit(callable));
		}

		service.shutdown();

		for (Future<TextExtractPar> future : futures) {
			Set<String> keywordSet1 = future.get().getL1().keySet();
			Set<String> keywordSet2 = future.get().getL2().keySet();

			for (String key : keywordSet1) {
				lp1.putAll(key, future.get().getL1().get(key));
			}
			for (String key : keywordSet2) {
				lp2.putAll(key, future.get().getL2().get(key));
			}
		}

	}

	private static TextExtractPar extractOneDoc(File[] listOfFile) throws FileNotFoundException {

		Multimap<String, String> lookup1 = ArrayListMultimap.create();
		Multimap<String, String> lookup2 = ArrayListMultimap.create();

		for (File file : listOfFile) {
			List<String> lines = new ArrayList<String>();
			counter++;
			FileInputStream fis = new FileInputStream(file);

			try {
				lines = Files.readLines(file, Charsets.UTF_8);
			} catch (IOException e) {
				System.out.println("File not read: " + file.getName());
			} finally {
				try {
					fis.close();
				} catch (IOException ioex) {
					// omitted.
				}
			}

			// ***********************************************************************************************//

			///////////////////// Begin word extraction
			///////////////////// /////////////////////////////

			// ***********************************************************************************************//

			int temporaryCounter = 0;

			// Filter threshold
			int counterDoc = 0;
			for (int i = 0; i < lines.size(); i++) {

				CharArraySet noise = EnglishAnalyzer.getDefaultStopSet();

				// We are using a standard tokenizer that eliminates the stop
				// words. We can use Stemming tokenizer such Porter
				// A set of English noise keywords is used that will eliminates
				// words such as "the, a, etc"

				Analyzer analyzer = new StandardAnalyzer(noise);
				List<String> token = Tokenizer.tokenizeString(analyzer, lines.get(i));
				allWords.addAll(token);//BENCHMARKS add words to search later
				
				temporaryCounter = temporaryCounter + token.size();
				for (int j = 0; j < token.size(); j++) {
					// Avoid counting occurrences of words in the same file
					if (!lookup2.get(file.getName()).contains(token.get(j))) {
						lookup2.put(file.getName(), token.get(j));
					}

					// Avoid counting occurrences of words in the same file
					if (!lookup1.get(token.get(j)).contains(file.getName())) {
						lookup1.put(token.get(j), file.getName());
					}
				}
			}
		}

		// System.out.println(lookup.toString());
		return new TextExtractPar(lookup1, lookup2);
	}
}