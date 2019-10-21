import os
import metadata
import func

class CBook:

    #Constructor
    def __init__(self, j_metadata):
        self.metadata = metadata.CMetadata(j_metadata)  #Complete metadata of book (Class)
        self.key = j_metadata["key"]                    #Key of book (also name of directory)
        self.path = ""              #Path to directory of book
        self.hasOcr = False         #Indicating whether book has an ocr
        self.mapWordsPages = {}     #Key: word, #value: list<int>    (list of pages)
        self.mapFuzzyMatches = {}   #Key: word, #value: list<string> (list of fuzzy-matches 
                                    #                                   for this word)

        self.mapRelevance = {}      #Key: word, #value: float (relavance)
        self.numPages = 0           #Number of pages

    
    #Create map of words, called when single book is added
    def createMapOfWords(self, path):
        self.path = path

        #Check whether path exists (whether book has an ocr)
        if os.path.exists(self.path +"/ocr.txt") == False:
            return

        #Check whether map of pages already exists (True: load existing map, False: create map)
        if os.path.exists(self.path+"/pages_new.txt")==False or os.stat(self.path+"/pages_new.txt").st_size==0 :
            self.createAndSafePages()
        else:
            self.loadPages()

        #Set has Ocr to true
        self.hasOcr = True


    #Create and safe pages on disk
    def createAndSafePages(self):

        #Open file (ocr of book)
        read = open(self.path+"/ocr.txt") 

        #Initialize variables
        sBuffer = ""
        pageCounter = 0

        # ---- CREATE MAP ---- #
        for line in read.readlines():

            #If new page, c
            if func.isNewPage(line):

                #Extract words from string and add to dict
                for word, rel in func.extractWords(sBuffer).items():
                    if word not in self.mapWordsPages:
                        self.mapWordsPages[word] = []
                        self.mapRelevance[word]  = float(rel)
                    self.mapWordsPages[word].append(pageCounter) 
                    self.mapRelevance[word] += float(rel)
                pageCounter +=1

                #Create new page as txt
                pagePath = self.path + "/page" + str(pageCounter) +".txt"
                if os.path.exists(pagePath) == True:
                    f = open("page"+str(pageCounter)+".txt", "w")
                    f.write(sBuffer)
                    f.close()
                else:
                    f = open(pagePath, "x")
                    f.write(sBuffer)
                    f.close()

                #Reset buffer
                sBuffer = ""

            #Othewise append line to buffer
            else:
                sBuffer += line
        read.close()

        self.numPages = pageCounter

        # ---- SAFE MAP ---- #
        f = open(self.path+"/pages_new.txt", "w")
        line = str(self.numPages)+"\n"
        f.write(line)
        for word, pages in self.mapWordsPages.items():
            line = word +";" 
            for page in pages:
                line += str(page) + ","
            relevance = self.mapRelevance[word]
            line += ";" + str(relevance) + "\n"
            f.write(line)


    #Load pages
    def loadPages(self):

        #Open file (saved map of words/pages)
        read = open(self.path+"/pages_new.txt")

        #Initialize variables
        firstline = read.readline()
        self.numPages = int(firstline)

        #Read line by line (values separated by ";": 0=word, 1=pages, 2=relevance)
        for line in read.readlines():

            #Detect false line
            if len(line) < 2:
                break

            #Split values
            vStrs = line.split(";")
            pages = []
            
            #Read pages
            for page in vStrs[1].split(","):
                pages.append(page)  

            #Set map of words and map of relevance
            self.mapWordsPages[vStrs[0]] = pages
            self.mapRelevance[vStrs[0]] = float(vStrs[2])

        read.close()


    #Get Pages where match was found
    def getPages(self, sInput, fuzzyness):

        #Check if book has ocr
        if self.hasOcr == False:
            return {}

        #Initialize results
        mapPages = {}

        # --- FULL SEARCH --- #

        if fuzzyness == 0:

            #Iterate over pages and add to map of results (without adding found word)
            for page in self.mapWordsPages[sInput]:
                mapPages[page] = [] 
            return mapPages
       

        # --- FUZZY SEARCH ---# 

        #Iterate over (fuzzy-) matches and add pages to results for each match
        for fuzzyMatch in self.mapFuzzyMatches[sInput] :
            #Iterate over pages and att to map of reslust (adding found match as value)
            for page in self.mapWordsPages[fuzzyMatch] :
                if page not in mapPages :
                    mapPages[page] = [fuzzyMatch]
                else:
                    mapPages[page].append(fuzzyMatch) 
        return mapPages
            

    # ---- get Praview (3 Functions) ---- #

    #Get Praview: function called
    def getPreview(self, sInput):

        #Check whetehr book has ocr and return message, telling user, why no preview found
        if self.hasOcr == False:
            return "No scans yet. We are trying to change that."

        #Getting (best) match found and page on which match occures
        sInput  = self.getMatch(sInput)
        page    = self.mapWordsPages[sInput][0]

        #Return result of get Prev
        return self.getPrev(sInput, int(page))

    #Get Match and page on which match has been found (not best match jetzt)
    def getMatch(self, sInput):
        if len(self.mapFuzzyMatches) == 0:
            return sInput
        else:
            return self.mapFuzzyMatches[sInput][0] 

    #Get preview from specific page
    def getPrev(self, sInput, page):

        #Get page-content from file
        f = open(self.path + "/page" + str(page+1) +".txt")
        sBuffer = f.read();
        f.close()

        #Find match
        pos = sBuffer.lower().find(sInput)

        #If no match found, return error message
        if pos == -1 :
            return "No Preview found, we're sorry for that."

        #Determin characters bevor and after match (idealy 75 befor and after)
        front = 0
        if pos - front > 75:
            front = pos-75
        back = len(sBuffer)-1
        if back-pos > 75:
            back = pos+75

        #Create preview 
        sPrev = sBuffer[front:back].replace('\n', '')
        p = sPrev.lower().find(sInput)
        l = len(sInput)

        #Return preview and add html mark for highlighting
        return "[...] " + sPrev[:p] + "<mark>" + sPrev[p:p+l] + "</mark>" + sPrev[p+l:] + " [...]"
        
        

       

