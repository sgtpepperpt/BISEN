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

// This file contains the step-by-step local benchmarking of the IEX-2Lev. The encrypted data structure remains in the RAM.
// This file also gathers stats useful to give some insights about the scheme implementation
// One needs to wait until the complete creation of the encrypted data structures of IEX-2Lev in order to issue queries.
// Queries need to be in the form of CNF. Follow on-line instructions
// ***********************************************************************************************//
package org.crypto.sse;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.Set;

public class TestLocalIEX2Lev {
	private static final String DATASET_DIR = "/home/guilherme/BooleanSSE/src/Data/parsed/"; // EDIT!
	private static final int TESTS = 6;

	private static Map<Integer, Integer> docFrequencies = new HashMap<Integer, Integer>();

	public static void main(String[] args) throws Exception {
		if (args.length != 1) {
			System.err.println("Usage: TestLocalIEX2Lev <dataset-size>");
			System.exit(1);
		}

		int datasetSize = Integer.parseInt(args[0]);
		
		//runTest(50);
		//System.out.println("#################################");
		runTest(100);
	}
	
	public static void runTest(int datasetSize) throws Exception {
		String pass = "1234";
		List<byte[]> listSK = IEX2Lev.keyGen(256, pass, "salt/salt", 100);

		TextProc.process(false, DATASET_DIR + datasetSize + "/");

		long startTime2 = System.nanoTime();
		IEX2Lev disj = IEX2Lev.setup(listSK, TextExtractPar.lp1, TextExtractPar.lp2, 1000, 100, 0);
		long endTime2 = System.nanoTime();
		long totalTime2 = endTime2 - startTime2;

		System.out.println("\n*****************************************************************");
		System.out.println("\n\t\tSTATS");
		System.out.println("\n*****************************************************************");

		System.out.println("\nNumber of keywords " + TextExtractPar.allWords.size());
		System.out.println("\nNumber of (w, id) pairs " + TextExtractPar.lp2.size());
		System.out.println("\nTotal number of stored (w, Id) including in local MM : " + IEX2Lev.numberPairs);
		System.out.println("\nTime elapsed per (w, Id) in ms: " + ((totalTime2 / IEX2Lev.numberPairs) / 1000));
		System.out.println("\nTotal Time elapsed for the entire construction in seconds: " + totalTime2 / 1000000000);
		System.out.printf("\nRelative Time elapsed per (w, Id) in s: %.3f\n",
				((double) totalTime2 / TextExtractPar.lp1.size()) / 1000000.0);

		// The two commented commands are used to compute the size of the
		// encrypted Local multi-maps and global multi-maps

		// System.out.println("\nSize of the Structure LMM: "+
		// SizeOf.humanReadable(SizeOf.deepSizeOf(disj.getLocalMultiMap())));
		// System.out.println("\nSize of the Structure MMg: "+
		// SizeOf.humanReadable(SizeOf.deepSizeOf(disj.getGlobalMM())));

		conjTest(disj, listSK);
		disjTest(disj, listSK);
		boolTest(disj, listSK);

		System.out.println("Number of docs returned in query: " + docFrequencies);
	}

	public static void conjTest(IEX2Lev disj, List<byte[]> listSK) throws Exception {
		// Conjunctive test for Selectivity: w AND x
		System.out.println("\n-- Conjunctive Test --");

		long start = 0;
		long total = 0;

		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[2][1];
			bool[0][0] = "enron";
			bool[1][0] = "time";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("AND 2 words: %dms\n", total / TESTS);

		start = 0;
		total = 0;
		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[5][1];
			bool[0][0] = "enron";
			bool[1][0] = "time";
			bool[2][0] = "inform";
			bool[3][0] = "work";
			bool[4][0] = "call";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("AND 5 words: %dms\n", total / TESTS);

		start = 0;
		total = 0;
		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[10][1];
			bool[0][0] = "enron";
			bool[1][0] = "time";
			bool[2][0] = "inform";
			bool[3][0] = "work";
			bool[4][0] = "call";
			bool[5][0] = "discuss";
			bool[6][0] = "meet";
			bool[7][0] = "week";
			bool[8][0] = "receive";
			bool[9][0] = "day";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("AND 10 words: %dms\n", total / TESTS);

		// System.out.printf("Average search time: %.2f ms\n", (((double) sumTime) /
		// counterTime) / 1000.0);
		// System.out.printf("Average word size of queries: %.0f\n\n", (((double)
		// sumWords) / counterWords));
	}

	public static void disjTest(IEX2Lev disj, List<byte[]> listSK) throws Exception {
		// Disjunctive test for Selectivity: w OR x
		System.out.println("-- Disjunctive Test --");

		long start = 0;
		long total = 0;

		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[1][2];
			bool[0][0] = "enron";
			bool[0][1] = "time";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("OR 2 words: %dms\n", total / TESTS);

		start = 0;
		total = 0;
		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[1][5];
			bool[0][0] = "enron";
			bool[0][1] = "time";
			bool[0][2] = "inform";
			bool[0][3] = "work";
			bool[0][4] = "call";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("OR 5 words: %dms\n", total / TESTS);

		start = 0;
		total = 0;
		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[1][10];
			bool[0][0] = "enron";
			bool[0][1] = "time";
			bool[0][2] = "inform";
			bool[0][3] = "work";
			bool[0][4] = "call";
			bool[0][5] = "discuss";
			bool[0][6] = "meet";
			bool[0][7] = "week";
			bool[0][8] = "receive";
			bool[0][9] = "day";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("OR 10 words: %dms\n", total / TESTS);

		// System.out.printf("Average search time: %.2f ms\n", (((double) sumTime) /
		// counterTime) / 1000.0);
		// System.out.printf("Average word size of queries: %.0f\n\n", (((double)
		// sumWords) / counterWords));
	}

	public static void boolTest(IEX2Lev disj, List<byte[]> listSK) throws Exception {
		// Boolean test for selectivity: (w OR x) AND (y or z)
		System.out.println("-- Boolean Test --");

		long start = 0;
		long total = 0;

		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[2][2];
			bool[0][0] = "call";
			bool[0][1] = "enron";
			bool[1][0] = "time";
			bool[1][1] = "attach";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("BOOL 2 words: %dms\n", total / TESTS);

		start = 0;
		total = 0;
		for (int i = 0; i < TESTS; i++) {
			start = System.currentTimeMillis();
			String[][] bool = new String[4][2];
			bool[0][0] = "call";
			bool[0][1] = "enron";
			bool[1][0] = "time";
			bool[1][1] = "attach";
			bool[2][0] = "inform";
			bool[2][1] = "work";
			bool[3][0] = "meet";
			bool[3][1] = "week";
			test(disj, listSK, bool);
			total += System.currentTimeMillis() - start;
		}

		System.out.printf("BOOL 8 words: %dms\n", total / TESTS);

		// System.out.printf("Average search time: %.2f ms\n", (((double) sumTime) /
		// counterTime) / 1000.0);
		// System.out.printf("Average word size of queries: %.0f\n\n", (((double)
		// sumWords) / counterWords));
	}

	public static void test(IEX2Lev disj, List<byte[]> listSK, String[][] bool) throws Exception {
		// long minimum = 1000000000;
		// long maximum = 0;
		// long average = 0;

		// Generate the IEX token
		List<String> searchBol = new ArrayList<String>();
		for (int i = 0; i < bool[0].length; i++) {
			searchBol.add(bool[0][i]);
		}

		// Generation of stream file to measure size of the token
		// long startTime3 = System.nanoTime();

		// System.out.println(searchBol);

		Set<String> tmpBol = IEX2Lev.query(IEX2Lev.token(listSK, searchBol), disj);

		// System.out.println(tmpBol);

		for (int i = 1; i < bool.length; i++) {
			Set<String> finalResult = new HashSet<String>();
			for (int k = 0; k < bool[0].length; k++) {
				List<String> searchTMP = new ArrayList<String>();
				searchTMP.add(bool[0][k]);
				for (int r = 0; r < bool[i].length; r++) {
					searchTMP.add(bool[i][r]);
				}

				List<TokenDIS> tokenTMP = IEX2Lev.token(listSK, searchTMP);

				Set<String> result = new HashSet<String>(RR2Lev.query(tokenTMP.get(0).getTokenMMGlobal(),
						disj.getGlobalMM().getDictionary(), disj.getGlobalMM().getArray()));

				if (!(tmpBol.size() == 0)) {
					List<Integer> temp = new ArrayList<Integer>(
							disj.getDictionaryForMM().get(new String(tokenTMP.get(0).getTokenDIC())));

					if (!(temp.size() == 0)) {
						int pos = temp.get(0);

						for (int j = 0; j < tokenTMP.get(0).getTokenMMLocal().size(); j++) {

							Set<String> temporary = new HashSet<String>();
							List<String> tempoList = RR2Lev.query(tokenTMP.get(0).getTokenMMLocal().get(j),
									disj.getLocalMultiMap()[pos].getDictionary(),
									disj.getLocalMultiMap()[pos].getArray());

							if (!(tempoList == null)) {
								temporary = new HashSet<String>(RR2Lev.query(tokenTMP.get(0).getTokenMMLocal().get(j),
										disj.getLocalMultiMap()[pos].getDictionary(),
										disj.getLocalMultiMap()[pos].getArray()));
							}

							finalResult.addAll(temporary);

							if (tmpBol.isEmpty()) {
								break;
							}

						}
					}

				}
			}
			tmpBol.retainAll(finalResult);

		}

		if (docFrequencies.containsKey(tmpBol.size()))
			docFrequencies.put(tmpBol.size(), docFrequencies.get(tmpBol.size()) + 1);
		else
			docFrequencies.put(tmpBol.size(), 1);

		// long totalTime3 = System.nanoTime() - startTime3;

		/*
		 * if (totalTime3 < minimum) { minimum = totalTime3; }
		 * 
		 * if (totalTime3 > maximum) { maximum = totalTime3; }
		 */

		// average += totalTime3;

		// System.out.printf("\nWord %s minimum: %.2f s\n", word, (double) (minimum /
		// 1000000.0));
		// System.out.printf("Word %s minimum: %.2f s\n", word, (double) (maximum /
		// 1000000.0));
		// System.out.printf("Word %s minimum: %.2f s\n", word, (double) (average /
		// numberIterations / 1000000.0));
		// sumTime += average;
		// counterTime += 1;
	}

	public static String getRandomWord() {
		int pos = new Random().nextInt(TextExtractPar.allWords.size());
		return TextExtractPar.allWords.get(pos);
	}
}
