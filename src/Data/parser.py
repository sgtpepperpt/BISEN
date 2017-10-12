import sys, os, email, re, random

def process_payload(payload):
    processed = ""
    isMetadata = False
    
    for line in payload.splitlines():
        if isMetadata:
            if(not line):
                isMetadata = False

            continue
        
        # match emails, urls and dates
        match1 = re.match(r"[0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2} [AM|PM]", line)
        match2 = re.match(r"[0-9]{2}/[0-9]{2}/[0-9]{4} [0-9]{2}:[0-9]{2}:[0-9]{2} [AM|PM]", line)
        match3 = re.match(r".*[\w\.]*@\w*\.\w*", line)
        match4 = re.match(r".*[\w\.\s]*@[\w\s]*.*", line)
        match5 = re.match(r"[\w\.]*@\w*\.\w* on \d{2}/\d{2}/\d{4} \d{2}:\d{2} [AM|PM]", line)
        match6 = re.match(r"[\w\.]*@\w*\.\w* on \d{2}/\d{2}/\d{4} \d{2}:\d{2}:\d{2} [AM|PM]", line)
        match7 = re.match(r".*https?:\/\/(www\.)?[-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,4}\b([-a-zA-Z0-9@:%_\+.~#?&//=]*).*", line)
        
        if (match1 or match2 or match3 or match4 or match5 or match6 or match7):
            isMetadata = True
        else:
            line = line.strip()
            if line.startswith(">"):
                line = line[1:]

            # match "metadata" lines
            isValid = line and not line.startswith("---") and not line.startswith("|") and not line.startswith("PM ----") and not line.startswith("AM ----") and not line.startswith("=")

            hasStrangeHeaders = line.startswith("From:") or line.startswith("To:") or line.startswith("Subject:") or line.startswith("cc:") or line.startswith("Sent:") or line.startswith("Date:")

            # only inserts non-empty lines
            if (not hasStrangeHeaders and isValid):
                processed += line + "\n"
    
    return processed

###############################################################################
def get_payload(filename):
    response = ""
    with open(filename) as file:
        mail = email.message_from_file(file)
        response += process_payload(mail.get_payload())

    return response

###############################################################################

def iterate_folders(dir, max):
    counter = 0
    files = [os.path.join(root, name) for root, dirs, files in os.walk(dir) for name in files]

    if(max < 0):
        max = len(files)

    while (counter < max):
        chosen = random.choice(files)
        mail = get_payload(chosen)
        
        # write parsed file
        nfile = open("./parsed/" + chosen[10:-1].replace("/", "_") + ".txt", "w") 
        nfile.write(mail)
        nfile.close()
        
        counter += 1

###############################################################################

if(len(sys.argv) != 2):
    print "Usage: python parser.py <number-of-docs>"
    print "To parse full dataset (~300k files): <number-of-docs> = -1"
    sys.exit(1)

# clear parsed folder
for f in os.listdir("./parsed/"):
    if f != ".gitkeep":
        os.remove("./parsed/" + f)

iterate_folders("./enron", int(sys.argv[1]))
