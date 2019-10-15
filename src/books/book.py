import os
import metadata

class CBook:

    #Constructor
    def __init__(self, j_metadata):
        self.metadata = metadata.CMetadata(j_metadata)
        self.key=j_metadata["key"]
        self.path = ""
        self.hasOcr = False
        self.mapWordsPages = {}     #key: word, #value: list<int>
        self.mapFuzzyMatches = {}   #key: word, #value: list<string>

    
    #Create map of words, called when single book is added
    def createMapOfWords(self, path):
        self.path = path

        if os.path.exists(self.path +"/ocr.txt") == False:
            return
        if os.path.exists(self.path+"/pages.txt")==False or os.stat(self.path+"/pages.txt").st_size==0 :
            self.createAndSafePages()
        else:
            self.loadPages()
        self.hasOcr = True

    #Create and safe pages on disk
    def createAndSafePages(self):
        print("creating...")


    #Load pages
    def loadPages(self):
        read = open(self.path+"/pages.txt")

        for line in read.readlines():
            if len(line) < 2:
                break

            vStrs = line.split(";")
            pages = []
            for page in vStrs[1].split(","):
                pages.append(page)  
            self.mapWordsPages[vStrs[0]] = pages


    #Get Pages 
    def getPages(self, sInput, fuzzyness):
        if self.hasOcr == False:
            return {}

        #Full search
        mapPages = {}
        if fuzzyness == 0:
            for page in self.mapWordsPages[sInput]:
                mapPages[page] = [] 
            return mapPages
       
        #Fuzzy search
        for fuzzyMatch in self.mapFuzzyMatches[sInput] :
            for page in self.mapWordsPages[fuzzyMatch] :
                if page not in mapPages :
                    mapPages[page] = [fuzzyMatch]
                else:
                    mapPages[page].append(fuzzyMatch) 

        return mapPages
            
    #Get Relevance
    def getRelevance(self, sInput, fuzzyness):
        if self.hasOcr == False:
            return 0

        #Full search
        if fuzzyness == 0:
            return len(self.mapWordsPages[sInput])

        #Fuzzy search
        if fuzzyness == 1:
            numPages = 0
            for fuzzyMatch in self.mapFuzzyMatches[sInput]:
                numPages += len(self.mapWordsPages[fuzzyMatch])
            return numPages
        else:
            return 100000

    def getPreview(self, sInput):
        return "No Preview" 
        #getPages
        #page = getPage()

    #def getPage(self, sInput)

       

        

   
