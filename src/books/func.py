import locale 

def extractWords(sWords):

    locale.setlocale(locale.LC_ALL, 'de_DE.utf8')
    #Create list of single words
    vWords = sWords.split(" ")

    #extract words into map
    resultWords = {}
    for word in vWords:
        #Check if word is a word
        if word.lower() in resultWords:
            resultWords[word.lower()] += 1
        resultWords[word.lower()] = 0
    
    #return result
    return resultWords
            
