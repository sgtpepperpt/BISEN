//
//  EnglishAnalyzer.cpp
//  BooleanSSE
//
//  Created by Bernardo Ferreira on 13/03/15.
//  Copyright (c) 2015 NovaSYS. All rights reserved.
//

#include "EnglishAnalyzer.h"

using namespace std;

EnglishAnalyzer::~EnglishAnalyzer() {
    free(s);
}

void EnglishAnalyzer::increase_s() {
    i_max += INC;
    char * new_s = (char *) malloc(i_max+1);
    if (new_s == NULL) pee("malloc error in EnglishAnalyzer::increase_s()");
    for (int i = 0; i < i_max-INC; i++)
        new_s[i] = s[i]; /* copy across */
    free(s);
    s = new_s;
}

char* EnglishAnalyzer::stemWord(string word) {
    char* original = (char*)word.c_str();
    string response;

    for(int i = 0; original[i] != '\0'; i++)
        original[i] = tolower(original[i]); /* forces lower case */

    original[stem(original,0,strlen(original)-1)+1]  = 0; /* calls the stemmer and uses its result to zero-terminate the string in s */

    return original;
}

void EnglishAnalyzer::stemWord_wiki(char* word) {
    //for(int i = 0; word[i] != '\0'; i++)
     //   word[i] = tolower(word[i]); /* forces lower case */

    word[stem(word,0,strlen(word)-1)+1]  = 0; /* calls the stemmer and uses its result to zero-terminate the string in s */
}

set<string> EnglishAnalyzer::extractUniqueKeywords(string fname) {
    FILE* f = fopen(fname.c_str(),"r");
    //printf("%s\n", fname.c_str());
    set<string> words;
    while(true) {
        int ch = getc(f);
        if (ch == EOF) {
            fclose(f);
            return words;
        }
        if (isalnum(ch)) {
            int i = 0;
            while(true) {
                if (i == i_max) increase_s();
                ch = tolower(ch); /* forces lower case */
                s[i++] = ch;
                ch = getc(f);
                if (!isalnum(ch)) {
                    ungetc(ch,f);
                    break;
                }
            }

            if (!isStopWord(s))
                words.insert(stemWord(s));

            memset(s, 0x00, i);
        }
    }
}

vector<set<string>> EnglishAnalyzer::extractUniqueKeywords_wiki(string fname) {
    vector<set<string>> docs;
    docs.push_back(set<string>());

    ifstream file(fname.c_str());
    if (!file) {
        cout << "unable to open file";
        exit(1);
    }

    unsigned curr_article = 0;
    string line;
    while (getline(file, line)) {
        const char* l = line.c_str();

        if(!memcmp(l, "<doc ", 5))
            continue;

        if(!memcmp(l, "</doc>", 5)) {
            curr_article++;
            docs.push_back(set<string>());
            continue;
        }

        char temp[strlen(l)];
        memset(temp, 0x00, strlen(l));
        unsigned pos = 0;
        for (unsigned i = 0; i < strlen(l); ++i) {
            if (isalnum(l[i])) {
                temp[pos++] = (char)tolower(l[i]);
            } else {
                if (!isStopWord(string(temp))) {
                    stemWord_wiki(temp);
                    string t(temp);
                    docs[curr_article].insert(t);
                }
            }
        }
    }

    return docs;
}

bool EnglishAnalyzer::isStopWord(string word) {
    return stopWords.count(word);
}

EnglishAnalyzer::EnglishAnalyzer() {
    i_max = INC;
    s = (char*) malloc(i_max+1);
    memset(s, 0, i_max+1);

    stopWords.insert("a");
    stopWords.insert("able");
    stopWords.insert("about");
    stopWords.insert("above");
    stopWords.insert("according");
    stopWords.insert("accordingly");
    stopWords.insert("across");
    stopWords.insert("actually");
    stopWords.insert("after");
    stopWords.insert("afterwards");
    stopWords.insert("again");
    stopWords.insert("against");
    stopWords.insert("all");
    stopWords.insert("allow");
    stopWords.insert("allows");
    stopWords.insert("almost");
    stopWords.insert("alone");
    stopWords.insert("along");
    stopWords.insert("already");
    stopWords.insert("also");
    stopWords.insert("although");
    stopWords.insert("always");
    stopWords.insert("am");
    stopWords.insert("among");
    stopWords.insert("amongst");
    stopWords.insert("an");
    stopWords.insert("and");
    stopWords.insert("another");
    stopWords.insert("any");
    stopWords.insert("anybody");
    stopWords.insert("anyhow");
    stopWords.insert("anyone");
    stopWords.insert("anything");
    stopWords.insert("anyway");
    stopWords.insert("anyways");
    stopWords.insert("anywhere");
    stopWords.insert("apart");
    stopWords.insert("appear");
    stopWords.insert("appreciate");
    stopWords.insert("appropriate");
    stopWords.insert("are");
    stopWords.insert("around");
    stopWords.insert("as");
    stopWords.insert("aside");
    stopWords.insert("ask");
    stopWords.insert("asking");
    stopWords.insert("associated");
    stopWords.insert("at");
    stopWords.insert("available");
    stopWords.insert("away");
    stopWords.insert("awfully");
    stopWords.insert("b");
    stopWords.insert("be");
    stopWords.insert("became");
    stopWords.insert("because");
    stopWords.insert("become");
    stopWords.insert("becomes");
    stopWords.insert("becoming");
    stopWords.insert("been");
    stopWords.insert("before");
    stopWords.insert("beforehand");
    stopWords.insert("behind");
    stopWords.insert("being");
    stopWords.insert("believe");
    stopWords.insert("below");
    stopWords.insert("beside");
    stopWords.insert("besides");
    stopWords.insert("best");
    stopWords.insert("better");
    stopWords.insert("between");
    stopWords.insert("beyond");
    stopWords.insert("both");
    stopWords.insert("brief");
    stopWords.insert("but");
    stopWords.insert("by");
    stopWords.insert("c");
    stopWords.insert("came");
    stopWords.insert("can");
    stopWords.insert("cannot");
    stopWords.insert("cant");
    stopWords.insert("cause");
    stopWords.insert("causes");
    stopWords.insert("certain");
    stopWords.insert("certainly");
    stopWords.insert("changes");
    stopWords.insert("clearly");
    stopWords.insert("co");
    stopWords.insert("com");
    stopWords.insert("come");
    stopWords.insert("comes");
    stopWords.insert("concerning");
    stopWords.insert("consequently");
    stopWords.insert("consider");
    stopWords.insert("considering");
    stopWords.insert("contain");
    stopWords.insert("containing");
    stopWords.insert("contains");
    stopWords.insert("corresponding");
    stopWords.insert("could");
    stopWords.insert("course");
    stopWords.insert("currently");
    stopWords.insert("d");
    stopWords.insert("definitely");
    stopWords.insert("described");
    stopWords.insert("despite");
    stopWords.insert("did");
    stopWords.insert("different");
    stopWords.insert("do");
    stopWords.insert("does");
    stopWords.insert("doing");
    stopWords.insert("done");
    stopWords.insert("down");
    stopWords.insert("downwards");
    stopWords.insert("during");
    stopWords.insert("e");
    stopWords.insert("each");
    stopWords.insert("edu");
    stopWords.insert("eg");
    stopWords.insert("eight");
    stopWords.insert("either");
    stopWords.insert("else");
    stopWords.insert("elsewhere");
    stopWords.insert("enough");
    stopWords.insert("entirely");
    stopWords.insert("especially");
    stopWords.insert("et");
    stopWords.insert("etc");
    stopWords.insert("even");
    stopWords.insert("ever");
    stopWords.insert("every");
    stopWords.insert("everybody");
    stopWords.insert("everyone");
    stopWords.insert("everything");
    stopWords.insert("everywhere");
    stopWords.insert("ex");
    stopWords.insert("exactly");
    stopWords.insert("example");
    stopWords.insert("except");
    stopWords.insert("f");
    stopWords.insert("far");
    stopWords.insert("few");
    stopWords.insert("fifth");
    stopWords.insert("first");
    stopWords.insert("five");
    stopWords.insert("followed");
    stopWords.insert("following");
    stopWords.insert("follows");
    stopWords.insert("for");
    stopWords.insert("former");
    stopWords.insert("formerly");
    stopWords.insert("forth");
    stopWords.insert("four");
    stopWords.insert("from");
    stopWords.insert("further");
    stopWords.insert("furthermore");
    stopWords.insert("g");
    stopWords.insert("get");
    stopWords.insert("gets");
    stopWords.insert("getting");
    stopWords.insert("given");
    stopWords.insert("gives");
    stopWords.insert("go");
    stopWords.insert("goes");
    stopWords.insert("going");
    stopWords.insert("gone");
    stopWords.insert("got");
    stopWords.insert("gotten");
    stopWords.insert("greetings");
    stopWords.insert("h");
    stopWords.insert("had");
    stopWords.insert("happens");
    stopWords.insert("hardly");
    stopWords.insert("has");
    stopWords.insert("have");
    stopWords.insert("having");
    stopWords.insert("he");
    stopWords.insert("hello");
    stopWords.insert("help");
    stopWords.insert("hence");
    stopWords.insert("her");
    stopWords.insert("here");
    stopWords.insert("hereafter");
    stopWords.insert("hereby");
    stopWords.insert("herein");
    stopWords.insert("hereupon");
    stopWords.insert("hers");
    stopWords.insert("herself");
    stopWords.insert("hi");
    stopWords.insert("him");
    stopWords.insert("himself");
    stopWords.insert("his");
    stopWords.insert("hither");
    stopWords.insert("hopefully");
    stopWords.insert("how");
    stopWords.insert("howbeit");
    stopWords.insert("however");
    stopWords.insert("i");
    stopWords.insert("ie");
    stopWords.insert("if");
    stopWords.insert("ignored");
    stopWords.insert("immediate");
    stopWords.insert("in");
    stopWords.insert("inasmuch");
    stopWords.insert("inc");
    stopWords.insert("indeed");
    stopWords.insert("indicate");
    stopWords.insert("indicated");
    stopWords.insert("indicates");
    stopWords.insert("inner");
    stopWords.insert("insofar");
    stopWords.insert("instead");
    stopWords.insert("into");
    stopWords.insert("inward");
    stopWords.insert("is");
    stopWords.insert("it");
    stopWords.insert("its");
    stopWords.insert("itself");
    stopWords.insert("j");
    stopWords.insert("just");
    stopWords.insert("k");
    stopWords.insert("keep");
    stopWords.insert("keeps");
    stopWords.insert("kept");
    stopWords.insert("know");
    stopWords.insert("knows");
    stopWords.insert("known");
    stopWords.insert("l");
    stopWords.insert("last");
    stopWords.insert("lately");
    stopWords.insert("later");
    stopWords.insert("latter");
    stopWords.insert("latterly");
    stopWords.insert("least");
    stopWords.insert("less");
    stopWords.insert("lest");
    stopWords.insert("let");
    stopWords.insert("like");
    stopWords.insert("liked");
    stopWords.insert("likely");
    stopWords.insert("little");
    stopWords.insert("ll"); // added to avoid words like you'll,I'll etc.
    stopWords.insert("look");
    stopWords.insert("looking");
    stopWords.insert("looks");
    stopWords.insert("ltd");
    stopWords.insert("m");
    stopWords.insert("mainly");
    stopWords.insert("many");
    stopWords.insert("may");
    stopWords.insert("maybe");
    stopWords.insert("me");
    stopWords.insert("mean");
    stopWords.insert("meanwhile");
    stopWords.insert("merely");
    stopWords.insert("might");
    stopWords.insert("more");
    stopWords.insert("moreover");
    stopWords.insert("most");
    stopWords.insert("mostly");
    stopWords.insert("much");
    stopWords.insert("must");
    stopWords.insert("my");
    stopWords.insert("myself");
    stopWords.insert("n");
    stopWords.insert("name");
    stopWords.insert("namely");
    stopWords.insert("nd");
    stopWords.insert("near");
    stopWords.insert("nearly");
    stopWords.insert("necessary");
    stopWords.insert("need");
    stopWords.insert("needs");
    stopWords.insert("neither");
    stopWords.insert("never");
    stopWords.insert("nevertheless");
    stopWords.insert("new");
    stopWords.insert("next");
    stopWords.insert("nine");
    stopWords.insert("no");
    stopWords.insert("nobody");
    stopWords.insert("non");
    stopWords.insert("none");
    stopWords.insert("noone");
    stopWords.insert("nor");
    stopWords.insert("normally");
    stopWords.insert("not");
    stopWords.insert("nothing");
    stopWords.insert("novel");
    stopWords.insert("now");
    stopWords.insert("nowhere");
    stopWords.insert("o");
    stopWords.insert("obviously");
    stopWords.insert("of");
    stopWords.insert("off");
    stopWords.insert("often");
    stopWords.insert("oh");
    stopWords.insert("ok");
    stopWords.insert("okay");
    stopWords.insert("old");
    stopWords.insert("on");
    stopWords.insert("once");
    stopWords.insert("one");
    stopWords.insert("ones");
    stopWords.insert("only");
    stopWords.insert("onto");
    stopWords.insert("or");
    stopWords.insert("other");
    stopWords.insert("others");
    stopWords.insert("otherwise");
    stopWords.insert("ought");
    stopWords.insert("our");
    stopWords.insert("ours");
    stopWords.insert("ourselves");
    stopWords.insert("out");
    stopWords.insert("outside");
    stopWords.insert("over");
    stopWords.insert("overall");
    stopWords.insert("own");
    stopWords.insert("p");
    stopWords.insert("particular");
    stopWords.insert("particularly");
    stopWords.insert("per");
    stopWords.insert("perhaps");
    stopWords.insert("placed");
    stopWords.insert("please");
    stopWords.insert("plus");
    stopWords.insert("possible");
    stopWords.insert("presumably");
    stopWords.insert("probably");
    stopWords.insert("provides");
    stopWords.insert("q");
    stopWords.insert("que");
    stopWords.insert("quite");
    stopWords.insert("qv");
    stopWords.insert("r");
    stopWords.insert("rather");
    stopWords.insert("rd");
    stopWords.insert("re");
    stopWords.insert("really");
    stopWords.insert("reasonably");
    stopWords.insert("regarding");
    stopWords.insert("regardless");
    stopWords.insert("regards");
    stopWords.insert("relatively");
    stopWords.insert("respectively");
    stopWords.insert("right");
    stopWords.insert("s");
    stopWords.insert("said");
    stopWords.insert("same");
    stopWords.insert("saw");
    stopWords.insert("say");
    stopWords.insert("saying");
    stopWords.insert("says");
    stopWords.insert("second");
    stopWords.insert("secondly");
    stopWords.insert("see");
    stopWords.insert("seeing");
    stopWords.insert("seem");
    stopWords.insert("seemed");
    stopWords.insert("seeming");
    stopWords.insert("seems");
    stopWords.insert("seen");
    stopWords.insert("self");
    stopWords.insert("selves");
    stopWords.insert("sensible");
    stopWords.insert("sent");
    stopWords.insert("serious");
    stopWords.insert("seriously");
    stopWords.insert("seven");
    stopWords.insert("several");
    stopWords.insert("shall");
    stopWords.insert("she");
    stopWords.insert("should");
    stopWords.insert("since");
    stopWords.insert("six");
    stopWords.insert("so");
    stopWords.insert("some");
    stopWords.insert("somebody");
    stopWords.insert("somehow");
    stopWords.insert("someone");
    stopWords.insert("something");
    stopWords.insert("sometime");
    stopWords.insert("sometimes");
    stopWords.insert("somewhat");
    stopWords.insert("somewhere");
    stopWords.insert("soon");
    stopWords.insert("sorry");
    stopWords.insert("specified");
    stopWords.insert("specify");
    stopWords.insert("specifying");
    stopWords.insert("still");
    stopWords.insert("sub");
    stopWords.insert("such");
    stopWords.insert("sup");
    stopWords.insert("sure");
    stopWords.insert("t");
    stopWords.insert("take");
    stopWords.insert("taken");
    stopWords.insert("tell");
    stopWords.insert("tends");
    stopWords.insert("th");
    stopWords.insert("than");
    stopWords.insert("thank");
    stopWords.insert("thanks");
    stopWords.insert("thanx");
    stopWords.insert("that");
    stopWords.insert("thats");
    stopWords.insert("the");
    stopWords.insert("their");
    stopWords.insert("theirs");
    stopWords.insert("them");
    stopWords.insert("themselves");
    stopWords.insert("then");
    stopWords.insert("thence");
    stopWords.insert("there");
    stopWords.insert("thereafter");
    stopWords.insert("thereby");
    stopWords.insert("therefore");
    stopWords.insert("therein");
    stopWords.insert("theres");
    stopWords.insert("thereupon");
    stopWords.insert("these");
    stopWords.insert("they");
    stopWords.insert("think");
    stopWords.insert("third");
    stopWords.insert("this");
    stopWords.insert("thorough");
    stopWords.insert("thoroughly");
    stopWords.insert("those");
    stopWords.insert("though");
    stopWords.insert("three");
    stopWords.insert("through");
    stopWords.insert("throughout");
    stopWords.insert("thru");
    stopWords.insert("thus");
    stopWords.insert("to");
    stopWords.insert("together");
    stopWords.insert("too");
    stopWords.insert("took");
    stopWords.insert("toward");
    stopWords.insert("towards");
    stopWords.insert("tried");
    stopWords.insert("tries");
    stopWords.insert("truly");
    stopWords.insert("try");
    stopWords.insert("trying");
    stopWords.insert("twice");
    stopWords.insert("two");
    stopWords.insert("u");
    stopWords.insert("un");
    stopWords.insert("under");
    stopWords.insert("unfortunately");
    stopWords.insert("unless");
    stopWords.insert("unlikely");
    stopWords.insert("until");
    stopWords.insert("unto");
    stopWords.insert("up");
    stopWords.insert("upon");
    stopWords.insert("us");
    stopWords.insert("use");
    stopWords.insert("used");
    stopWords.insert("useful");
    stopWords.insert("uses");
    stopWords.insert("using");
    stopWords.insert("usually");
    stopWords.insert("uucp");
    stopWords.insert("v");
    stopWords.insert("value");
    stopWords.insert("various");
    stopWords.insert("ve"); // added to avoid words like I've,you've etc.
    stopWords.insert("very");
    stopWords.insert("via");
    stopWords.insert("viz");
    stopWords.insert("vs");
    stopWords.insert("w");
    stopWords.insert("want");
    stopWords.insert("wants");
    stopWords.insert("was");
    stopWords.insert("way");
    stopWords.insert("we");
    stopWords.insert("welcome");
    stopWords.insert("well");
    stopWords.insert("went");
    stopWords.insert("were");
    stopWords.insert("what");
    stopWords.insert("whatever");
    stopWords.insert("when");
    stopWords.insert("whence");
    stopWords.insert("whenever");
    stopWords.insert("where");
    stopWords.insert("whereafter");
    stopWords.insert("whereas");
    stopWords.insert("whereby");
    stopWords.insert("wherein");
    stopWords.insert("whereupon");
    stopWords.insert("wherever");
    stopWords.insert("whether");
    stopWords.insert("which");
    stopWords.insert("while");
    stopWords.insert("whither");
    stopWords.insert("who");
    stopWords.insert("whoever");
    stopWords.insert("whole");
    stopWords.insert("whom");
    stopWords.insert("whose");
    stopWords.insert("why");
    stopWords.insert("will");
    stopWords.insert("willing");
    stopWords.insert("wish");
    stopWords.insert("with");
    stopWords.insert("within");
    stopWords.insert("without");
    stopWords.insert("wonder");
    stopWords.insert("would");
    stopWords.insert("would");
    stopWords.insert("x");
    stopWords.insert("y");
    stopWords.insert("yes");
    stopWords.insert("yet");
    stopWords.insert("you");
    stopWords.insert("your");
    stopWords.insert("yours");
    stopWords.insert("yourself");
    stopWords.insert("yourselves");
    stopWords.insert("z");
    stopWords.insert("zero");
}
