import locale 

def extractWords(sWords):

    locale.setlocale(locale.LC_ALL, 'de_DE.utf8')
    #Create list of single words
    vWords = sWords.split(" ")

    #extract words into map
    resultWords = {}
    for word in vWords:
        for w in word.split("\n"):
            #Check if word is a word
            newWord = transform(w)
            if newWord == "":
                continue
            if newWord in resultWords:
                resultWords[newWord] += 1
            else:
                resultWords[newWord] = 1
    
    #return result
    return resultWords

def transform(sWord):
    if len(sWord) <= 2:
        return ""

    counter=0
    for char in sWord:
        if char.isalpha() == False:
            counter+=1

    if counter/len(sWord) > 0.3:
        return ""

    while len(sWord) > 0:
        if sWord[0].isalpha() == False:
            sWord = sWord[1:]
        elif sWord[len(sWord)-1].isalpha() == False:
            sWord = sWord[:len(sWord)-1]
        else:
            break

    sWord = sWord.replace(';','')

    if len(sWord) <= 2:
        return ""

    return sWord.lower()
        
def isNewPage(sLine):   
    if len(sLine) < 6:
        return False
    if sLine.startswith("----- ") == False:
        return False
    if sLine[6].isdigit() == True:
        return True
    return False

