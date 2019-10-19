import locale 

def extractWords(sWords):

    locale.setlocale(locale.LC_ALL, 'de_DE.utf8')

    #Create list of single words (split words at " ") 
    vWords = splitWords(sWords)

    #extract words into map
    resultWords = {}
    for word in vWords:

        #Check if word is a word
        newWord = transform(word)
        if newWord == "":
            continue

        #if not first occurance, increase relevance
        if newWord in resultWords:
            resultWords[newWord] += 1
        #Set relavance to 1
        else:
            resultWords[newWord] = 1
    
    #return result
    return resultWords

def splitWords(words):
    
    # --- First " "   --- #
    vWords1 = words.split(" ")

    # --- Second "\n" --- #
    result1 = []
    #for each word, check "\n", if True, then split
    for word in vWords1:
        pos = word.find("\n")

        #Add words...
        if pos != -1:
            vWords2 = word.split("\n")
            for i in range(1, len(vWords2)-1):
                w = vWords2[i-1]
                if len(w) < 2:
                    vWords2[i-1] = ""
                elif w[len(w)-1] == '-':
                    vWords2[i] = w[:len(w)-1] + vWords2[i] 
                    vWords2[i-1] = ""
            for w in vWords2:
                if w != "":
                    result1.append(w)

        #Add word without changes to reslut1
        else:
            result1.append(word)

    # --- Second ";"
    result2 = []
    #for each word, check "\n", if True, then split
    for word in result1:

        #Add seperated words to result2
        if word.find(';') != -1:
            vWords3 = word.split(";")
            for w in vWords3:
                result2.append(w)

        #Add word without edditing to reslut2
        else:
            result2.append(word)

    return result2


#Transform word: don't accept: shorter than 2 chars, to many non-chars,
# delete non-characters at beginning and end, convert to lower case
def transform(sWord):

    #Check length of word: no words shorter than 2 chars not accepted
    if len(sWord) < 2:
        return ""

    #Count number of non-Characters: word only accepted if non-chars/len < 0.3
    counter=0
    for char in sWord:
        if char.isalpha() == False:
            counter+=1
    if counter/len(sWord) > 0.3:
        return ""

    #Delete non-characters at beginning and end of word
    while len(sWord) > 0:
        if sWord[0].isalpha() == False:
            sWord = sWord[1:]
        elif sWord[len(sWord)-1].isalpha() == False:
            sWord = sWord[:len(sWord)-1]
        else:
            break

    #Check length again
    if len(sWord) < 2:
        return ""

    #Rconvert to lower case and return 
    return sWord.lower()
        

#Check if givin line indicates new page
def isNewPage(sLine):   
    if len(sLine) < 6:
        return False
    if sLine.startswith("----- ") == False:
        return False
    if sLine[6].isdigit() == True:
        return True
    return False

